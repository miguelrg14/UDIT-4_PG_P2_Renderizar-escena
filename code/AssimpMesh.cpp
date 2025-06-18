
// Este código es de dominio público
// Miguel Rodríguez Gallego

#include "AssimpMesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <gtc/type_ptr.hpp>
#include <stdexcept>

//using namespace udit;

namespace udit
{
    void AssimpMesh::load(const std::string& mesh_file_path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile
        (
            mesh_file_path,
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType
        );

        if (!scene || scene->mNumMeshes == 0) 
        {
            throw std::runtime_error(std::string("No se pudo cargar mesh: ") + importer.GetErrorString());
        }

        aiMesh* mesh = scene->mMeshes[0];
        size_t nverts = mesh->mNumVertices;

        // Genera VAO y VBOs
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);
        glBindVertexArray(vao_id);

        // Coordenadas
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORD_VBO]);
        glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(aiVector3D), mesh->mVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Normales
        if (mesh->HasNormals()) 
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMAL_VBO]);
            glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(aiVector3D), mesh->mNormals, GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        // UVs
        if (mesh->HasTextureCoords(0)) 
        {
            std::vector<glm::vec2> uvs(nverts);
            for (size_t i = 0; i < nverts; ++i)
            {
                uvs[i] = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            }
            glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[UV_VBO]);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        // Índices
        number_of_indices = mesh->mNumFaces * 3;
        std::vector<GLushort> indices;
        indices.reserve(number_of_indices);
        for (unsigned i = 0; i < mesh->mNumFaces; ++i) 
        {
            auto& f = mesh->mFaces[i];
            indices.push_back(f.mIndices[0]);
            indices.push_back(f.mIndices[1]);
            indices.push_back(f.mIndices[2]);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void AssimpMesh::draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const
    {
        // Asume uniform mat4 uModel
        GLint loc = glGetUniformLocation(shaderProgram, "uModel");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, number_of_indices, GL_UNSIGNED_SHORT, nullptr);
        glBindVertexArray(0);
    }
}