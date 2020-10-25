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
    std::string directory;
    std::unordered_map<std::string, Texture> loadedTextureByPath;

    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType aiType,
                                              TextureType type);

    static GLuint loadTextureFromFile(const std::string &path, const std::string &directory,
                                      bool gamma = false, GLint wrappingMode = GL_REPEAT);
};

#endif