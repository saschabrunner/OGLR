#define STB_IMAGE_IMPLEMENTATION // stb_image.h one time initialization

#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glibmm-2.4/glibmm/miscutils.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "lib/stb_image.h"

Model::Model(const std::string &path)
{
    loadModel(path);
}

void Model::draw(Shader &shader)
{
    for (Mesh &mesh : meshes)
    {
        mesh.draw(shader);
    }
}

void Model::loadModel(const std::string &path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Assimp: " << importer.GetErrorString() << std::endl;
        return;
    }

    baseDir = Glib::path_get_dirname(path);

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // iterate through all the meshes in the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // get the actual mesh, since the node only stores the index
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // process child nodes recursively
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    // for every vertex of the mesh
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        if (mesh->HasNormals())
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (mesh->HasTextureCoords(0))
        {
            // a vertex can heave up to 8 texture coordinates in assimp
            // we only use the first one for now
            vertex.textureCoordinates.x = mesh->mTextureCoords[0][i].x;
            vertex.textureCoordinates.y = mesh->mTextureCoords[0][i].y;
        }

        vertices.push_back(vertex);
    }

    // for every face (vertices that form a primitive)
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuseMaps =
        loadMaterialTextures(material, scene,
                             aiTextureType_DIFFUSE, TextureType::diffuse);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture> specularMaps =
        loadMaterialTextures(material, scene,
                             aiTextureType_SPECULAR, TextureType::specular);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<Texture> emissiveMaps =
        loadMaterialTextures(material, scene,
                             aiTextureType_EMISSIVE, TextureType::specular);
    textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *material, const aiScene *scene,
                                                 aiTextureType aiType, TextureType type)
{
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < material->GetTextureCount(aiType); i++)
    {
        aiString path;
        material->GetTexture(aiType, i, &path);
        std::string stdPath = path.C_Str();

        if (stdPath.length() == 0)
        {
            std::cerr << "Got texture from Assimp with no path" << std::endl;
            return textures;
        }

        if (loadedTextureByPath.count(stdPath) > 0)
        {
            // texture exists already
            textures.push_back(loadedTextureByPath.at(stdPath));
        }
        else
        {
            // new texture, needs to be loaded
            Texture texture;

            if (stdPath[0] == '*')
            {
                // texture is embedded in same file, needs to be extracted through assimp
                int assimpTextureIndex = std::stoi(stdPath.substr(1, std::string::npos));
                aiTexture *aiTexture = scene->mTextures[assimpTextureIndex];
                texture.id = loadEmbeddedTexture(aiTexture);
            }
            else
            {
                texture.id = loadTextureFromFile(stdPath, baseDir);
            }

            texture.path = stdPath;
            texture.type = type;
            textures.push_back(texture);
            loadedTextureByPath.insert({stdPath, texture});
        }
    }

    return textures;
}

GLuint Model::loadEmbeddedTexture(const aiTexture *texture, GLint wrappingMode)
{
    int width, height, nrChannels;

    if (texture->mHeight == 0)
    {
        // texture is compressed
        unsigned char *textureData =
            stbi_load_from_memory((const stbi_uc *)texture->pcData, texture->mWidth,
                                  &width, &height, &nrChannels, 0);

        if (!textureData)
        {
            // TODO: Add some error handling or fallback behavior
            std::cerr << "Could not read embedded texture" << std::endl;
            return -1;
        }

        GLuint glTextureId = createGlTexture(textureData, width, height, nrChannels, wrappingMode);

        // free the texture data again
        stbi_image_free(textureData);

        return glTextureId;
    }
    else
    {
        return createGlTexture((unsigned char *)texture->pcData,
                               texture->mWidth, texture->mHeight, 4);
    }
}

GLuint Model::loadTextureFromFile(const std::string &texturePath, const std::string &baseDir,
                                  GLint wrappingMode)
{
    // texture paths are provided as relative paths to the model
    std::string path = baseDir + '/' + texturePath;

    // make sure the image is loaded in a way that represents OpenGL texture coordinates
    stbi_set_flip_vertically_on_load(true);

    // read image into byte array
    int width, height, nrChannels;
    unsigned char *textureData = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    // disable vertical flipping again, for future loads that might not need it
    stbi_set_flip_vertically_on_load(false);

    if (!textureData)
    {
        // TODO: Add some error handling or fallback behavior
        std::cerr << "Could not read texture from '" << path << "'" << std::endl;
        return -1;
    }

    GLuint glTextureId = createGlTexture(textureData, width, height, nrChannels, wrappingMode);

    // free the texture data again
    stbi_image_free(textureData);

    return glTextureId;
}

GLuint Model::createGlTexture(unsigned char *buffer, int width, int height, int nrChannels,
                              GLint wrappingMode)
{
    GLenum format;

    if (nrChannels == 1)
    {
        format = GL_RED;
    }
    else if (nrChannels == 3)
    {
        format = GL_RGB;
    }
    else if (nrChannels == 4)
    {
        format = GL_RGBA;
    }
    else
    {
        std::cerr << "Unexpected number of channels: " << nrChannels << std::endl;
        return -1;
    }

    // create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture attributes (repeat and use linear filtering)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // copy texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);

    // let OpenGL generate mipmaps for us
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}