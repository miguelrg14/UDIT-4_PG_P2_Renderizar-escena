
// Este código es de dominio público
// Miguel Rodríguez Gallego

#pragma once

#include <glad/glad.h>
#include "Camera.hpp"
#include <string>

namespace udit
{
    class Spiral
    {
        static const std::string vertex_shader_code;
        static const std::string fragment_shader_code;

    private:

        enum
        {
            COORDINATES_VBO,
            INDICES_EBO,
            VBO_COUNT
        };

        int indexCount = 0;
        GLuint vbo_ids[VBO_COUNT];   // VBOs: índice y opcionalmente colores
        GLuint vao_id;               // VAO de la espiral
        GLuint shader_program_id;

        GLint model_view_matrix_id;
        GLint projection_matrix_id;
        GLint camera_position_id;

    public:

        Spiral();
        ~Spiral();

        void render(const Camera& camera, const glm::mat4& model_matrix);

    private:

        GLuint compile_shaders();
        void show_compilation_error(GLuint shader_id);
        void show_linkage_error(GLuint program_id);
    };
}