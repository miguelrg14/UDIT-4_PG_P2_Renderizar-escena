
// Este código es de dominio público
// Miguel Rodríguez Gallego

#ifndef ASSIMPMESH_HEADER
#define ASSIMPMESH_HEADER

    #pragma once
    #include "Mesh.hpp"
    #include <string>
    #include <glm.hpp>
    #include <glad/glad.h>

    namespace udit 
    {
        class AssimpMesh : public Mesh 
        {
        public:
            enum 
            { 
                COORD_VBO, 
                NORMAL_VBO,
                UV_VBO,
                INDICES_EBO,
                VBO_COUNT 
            };

            GLuint vao_id;
            GLuint vbo_ids[VBO_COUNT];
            GLsizei number_of_indices = 0;

            // Nueva parte:
            /// Ruta a la textura que queramos usar para este mesh
            void setTexturePath(const std::string& path) { texturePath = path; }
            void   setTextureID(           GLuint    id) { textureID   = id;   }

            AssimpMesh();
            ~AssimpMesh();

            void load(const std::string& mesh_file_path) override;
            void draw(const glm::mat4&   modelMatrix, GLuint shaderProgram) const override;

        private:
            std::string texturePath;
            GLuint      textureID = 0;
        };

    }

#endif