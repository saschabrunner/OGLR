#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

#include "Shader.h"

enum class TextureType
{
    diffuse,
    specular,
    emissive
};

// note: this could be optimized for meshes that don't have textures (or even normals)
// separate vertex types could be introduced for that as an optimization
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinates;
};

struct Texture
{
    GLuint id;
    TextureType type;
    std::string path;
};

class Mesh
{
public:
    const std::vector<Vertex> vertices;
    const std::vector<GLuint> indices;
    const std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
    void draw(Shader &shader);

private:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    void setupMesh();
};

#endif