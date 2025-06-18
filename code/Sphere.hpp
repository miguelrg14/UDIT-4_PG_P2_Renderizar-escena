
// Este código es de dominio público
// Miguel Rodríguez Gallego

#ifndef SPHERE_HEADER
#define SPHERE_HEADER

    #pragma once

    #include <glad/glad.h>

    namespace udit
    {

        class Sphere
        {
        private:

            // Índices para acceder a los buffers en el array vbo_ids:
            enum
            {
                COORDINATES_VBO,
                COLORS_VBO,
                INDICES_EBO,
                VBO_COUNT
            };

            int indexCount = 0;

            GLuint vbo_ids[VBO_COUNT];   // Ids de los VBOs (coordenadas, colores, índices)
            GLuint vao_id;               // Id del VAO del cono

        public:

             Sphere();
            ~Sphere();

            void Subdivision(int subdivisions);

            void render();

        };

    }

#endif