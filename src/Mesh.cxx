#include "lib/glad/include/glad/glad.h"

#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
    : vertices(vertices), indices(indices), textures(textures)
{
    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(ebo, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, textureCoordinates));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw(Shader shader)
{
    int diffuseNr = 0;
    int specularNr = 0;
    int emissionNr = 0;

    for (int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate texture based on index

        std::string uniformName;
        switch (textures[i].type)
        {
        case TextureType::diffuse:
            uniformName = "textureDiffuse" + std::to_string(diffuseNr);
            diffuseNr++;
            break;
        case TextureType::specular:
            uniformName = "textureSpecular" + std::to_string(specularNr);
            specularNr = 0;
            break;
        case TextureType::emission:
            uniformName = "textureEmission" + std::to_string(emissionNr);
            emissionNr++;
            break;
        default:
            std::cerr << "Unknown texture type: "
                      << static_cast<std::underlying_type_t<TextureType>>(textures[i].type);
            break;
        }

        shader.setFloat("material." + uniformName, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}