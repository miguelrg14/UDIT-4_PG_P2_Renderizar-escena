
// Este código es de dominio público
// Miguel Rodríguez Gallego

#ifndef RUBIK_HEADER
#define RUBIK_HEADER

    #pragma once

    #include <vector>
    #include <glm.hpp>
    #include "Cube.hpp"

    namespace udit
    {
        class Rubik
        {

        public:
            struct SubCube
            {
                glm::vec3 base_position;   // Posición base (fija)
                glm::mat4 transform;       // Transformación acumulada
            };

        private:
            std::vector<SubCube> cubes;
            const GLfloat spacing;

        public:
             Rubik (float spacing = 2.1f);
            ~Rubik ();

            void render
            (
                const glm::mat4& view,
                const glm::mat4& projection,
                const glm::mat4& model_global,
                GLint model_view_matrix_id,
                GLint normal_matrix_id,
                GLint material_color_loc,
                GLuint program_id
            );

            void rotate_row (float row, float speed);  // Y == 1, 0, -1
        };
    }

#endif