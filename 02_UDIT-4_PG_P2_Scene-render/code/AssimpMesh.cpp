
// Este código es de dominio público
// Miguel Rodríguez Gallego

#include "AssimpMesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <SOIL2.h>          // para SOIL_load_OGL_texture
#include <gtc/type_ptr.hpp>
#include <stdexcept>
#include <iostream>

namespace udit
{
    AssimpMesh::AssimpMesh() : textureID(0)
    {

    }
    AssimpMesh::~AssimpMesh()
    {
        if (textureID) glDeleteTextures(1, &textureID);
    }

    void AssimpMesh::load(const std::string& mesh_file_path)
    {
        Assimp::Importer importer;

        // 1) Importar modelo
        const aiScene* scene = importer.ReadFile
        (
            mesh_file_path,
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType
        );
        if (!scene || scene->mNumMeshes == 0)
        {
            throw std::runtime_error
            (
                std::string("No se pudo cargar mesh: ") + importer.GetErrorString()
            );
        }

        // Si scene es un puntero nulo significa que el archivo no se pudo cargar con éxito:
        if (scene && scene->mNumMeshes > 0)
        {
            // 2) Subida de geometría
            aiMesh* mesh = scene->mMeshes[0];
            size_t  nverts = mesh->mNumVertices;

            // Se generan índices para los VBOs del cubo:
            glGenVertexArrays(1, &vao_id);
            glGenBuffers(VBO_COUNT, vbo_ids);
            // Se activa el VAO del cubo para configurarlo:
            glBindVertexArray(vao_id);

            // — Coordenadas —
            glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORD_VBO]);
            glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(aiVector3D), mesh->mVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            // — Normales — (Normales para iluminación)
            if (mesh->HasNormals())
            {
                // Se suben a un VBO los datos de color y se vinculan al VAO:
                glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMAL_VBO]);
                glBufferData(GL_ARRAY_BUFFER, nverts * sizeof(aiVector3D), mesh->mNormals, GL_STATIC_DRAW);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            }

            // — UVs — (Coordenadas de textura (UVs))
            if (mesh->HasTextureCoords(0))
            {
                std::vector<glm::vec2> uvs(nverts);
                for (size_t i = 0; i < nverts; ++i)
                {
                    uvs[i] = glm::vec2
                    (
                        mesh->mTextureCoords[0][i].x,
                        mesh->mTextureCoords[0][i].y
                    );
                }
                glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[UV_VBO]);
                glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            }

            // Índices
            // Los índices en ASSIMP están repartidos en "faces", pero OpenGL necesita un array de enteros
            // por lo que vamos a mover los índices de las "faces" a un array de enteros:
            // Se asume que todas las "faces" son triángulos (revisar el flag aiProcess_Triangulate arriba).
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

            // Se suben a un EBO los datos de índices:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
    }


    void AssimpMesh::draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const
    {
        // 1) Uniform de modelo
        GLint loc = glGetUniformLocation(shaderProgram, "model_view_matrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // 2) Bind de la textura si existe
        if (textureID) 
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            GLint s = glGetUniformLocation(shaderProgram, "sampler");
            glUniform1i(s, 0);
        }

        // 3) Dibujo de la malla
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, number_of_indices,GL_UNSIGNED_SHORT,nullptr);
        glBindVertexArray(0);
    }
}