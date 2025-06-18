
// Este código es de dominio público
// Miguel Rodríguez Gallego

#ifndef ASSIMPMESH_HEADER
#define ASSIMPMESH_HEADER

    #pragma once

    #include "Mesh.hpp"
    #include <assimp/scene.h>
    #include <vector>

    namespace udit 
    {
        class AssimpMesh : public Mesh 
        {
        public:
            // Aquí guardas tus IDs
            enum
            { 
                COORD_VBO = 0, 
                NORMAL_VBO, 
                UV_VBO, 
                INDICES_EBO, 
                VBO_COUNT 
            };

            GLuint vbo_ids[VBO_COUNT];
            GLuint vao_id;
            GLsizei number_of_indices = 0;

            void load(const std::string& mesh_file_path) override;
            void draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const override;
        };

    }

#endif