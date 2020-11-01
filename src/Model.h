#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <unordered_map>
#include <vector>

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>

#include "lib/glad/include/glad/glad.h"

#include "Mesh.h"
#include "Shader.h"

class Model
{
public:
    Model(const std::string &path);

    void draw(Shader &shader);

private:
    std::vector<Mesh> meshes;
    std::string baseDir;
    std::unordered_map<std::string, Texture> loadedTextureByPath;

    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *material, const aiScene *scene,
                                              aiTextureType aiType, TextureType type);

    static GLuint loadEmbeddedTexture(const aiTexture *texture, GLint wrappingMode = GL_REPEAT);

    static GLuint loadTextureFromFile(const std::string &texturePath, const std::string &baseDir,
                                      GLint wrappingMode = GL_REPEAT);

    static GLuint createGlTexture(unsigned char *buffer, int width, int height, int nrChannels,
                                  GLint wrappingMode = GL_REPEAT);
};

#endif