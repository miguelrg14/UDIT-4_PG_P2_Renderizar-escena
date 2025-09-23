
// Este código es de dominio público
// Miguel Rodríguez Gallego

#ifndef MESH_HEADER
#define MESH_HEADER

    #pragma once

    #include <string>
    #include <glm.hpp>
    #include <glad/glad.h>

    namespace udit 
    {
        class Mesh 
        {
        public:
            virtual ~Mesh() = default;

            /// Carga el modelo desde fichero
            virtual void load(const std::string& mesh_file_path) = 0;

            /// Dibuja la malla con la matriz de modelo y un shader
            virtual void draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const = 0;
        };

    }

#endif