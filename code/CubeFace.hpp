
// Este código es de dominio público
// Miguel Rodríguez Gallego

#ifndef CUBEFACE_HEADER
#define CUBEFACE_HEADER

    #pragma once

    #include <glad/glad.h>
    #include <glm.hpp>

    namespace udit
    {
        class CubeFace
        {
        private:

            // Índices para indexar el array vbo_ids:
            enum
            {
                COORDINATES_VBO,
                NORMALS_VBO,
                INDICES_IBO,
                VBO_COUNT
            };

        private:
            // Arrays de datos del cubo base:
            static const GLfloat coordinates[];
            static const GLfloat normals[];
            static const GLubyte indices[];

            GLuint vbo_ids[VBO_COUNT];      // Ids de los VBOs que se usan
            GLuint vao_id;                  // Id del VAO del cubo

        public:
            CubeFace();  // genera un plano cuadrado 2x2 en XY (normal hacia Z+)
            ~CubeFace();

            void render();
        };
    }

#endif