#include "Spiral.hpp"
#include <vector>
#include <iostream>
#include <gtc/type_ptr.hpp>

namespace udit
{
    using namespace std;
    using namespace glm;

    const std::string Spiral::vertex_shader_code =
        R"(
            #version 330

            uniform mat4 model_view_matrix;
            uniform mat4 projection_matrix;
            uniform vec3 camera_position;

            layout (location = 0) in float index;

            out float indice;

            void main()
            {
                float PI = 3.1415926535897932384626433832795;
                float radius = 0.2 + 0.005 * index;

                vec4 local_pos = vec4
                (
                    sin(2.0 * PI * 0.005 * index) * radius,
                    index * 0.005 - 0.5,
                    cos(2.0 * PI * 0.005 * index) * radius,
                    1.0
                );

                gl_Position = projection_matrix * model_view_matrix * local_pos;
                indice = index;
            }
        )";

    const std::string Spiral::fragment_shader_code = 
        R"(
            #version 330

            in  float indice;
            out vec4 fragment_color;

            uniform samplerCube sampler;

            void main()
            {
                fragment_color = vec4(indice * 0.001, indice * 0.001, indice * 0.001, 1.0);
            }
        )";

    Spiral::Spiral()
    {
        // Generar índices de espiral
        std::vector<GLfloat> indices(1000);
        for (size_t i = 0; i < indices.size(); ++i)
        {
            indices[i] = static_cast<GLfloat>(i);
        }

        // Se compilan y enlazan los shaders:
        indexCount = indices.size();

        shader_program_id = compile_shaders();
        model_view_matrix_id = glGetUniformLocation(shader_program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(shader_program_id, "projection_matrix");
        camera_position_id = glGetUniformLocation(shader_program_id, "camera_position");

        // Se generan ú‹dices para los VBOs del cubo:
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);

        // Se activa el VAO del cubo para configurarlo:
        glBindVertexArray(vao_id);

        // Se suben a un VBO los datos de coordenadas y se vinculan al VAO:
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(GLfloat), indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    Spiral::~Spiral()
    {
        // Se liberan los VBOs y el VAO usados:
        glDeleteBuffers(VBO_COUNT, vbo_ids);
        glDeleteVertexArrays(1, &vao_id);
    }

    void Spiral::render(const Camera& camera, const glm::mat4& model_matrix)
    {
        // Se usa el shader
        glUseProgram(shader_program_id);

        const glm::mat4       view_matrix = camera.get_transform_matrix_inverse();
        const glm::mat4 model_view_matrix = view_matrix * model_matrix;
        const glm::mat4&       projection = camera.get_projection_matrix();

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));
        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection));
              glUniform3fv(  camera_position_id, 1, glm::value_ptr(camera.get_location()));

        glDepthMask(GL_FALSE);
        // Se activa el VAO del modelo para configurarlo:
        glBindVertexArray(vao_id);
        glDrawArrays(GL_LINE_STRIP, 1, indexCount - 1);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        glUseProgram(0);
    }

    GLuint Spiral::compile_shaders()
    {
        GLint succeeded = GL_FALSE;

        // Se crean objetos para los shaders:
        GLuint   vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        // Se carga el código de los shaders:
        const char* vertex_shaders_code[] = { vertex_shader_code.c_str() };
        const char* fragment_shaders_code[] = { fragment_shader_code.c_str() };
        const GLint    vertex_shaders_size[] = { (GLint)vertex_shader_code.size() };
        const GLint  fragment_shaders_size[] = { (GLint)fragment_shader_code.size() };

        glShaderSource(vertex_shader_id, 1, vertex_shaders_code, vertex_shaders_size);
        glShaderSource(fragment_shader_id, 1, fragment_shaders_code, fragment_shaders_size);

        // Se compilan los shaders:
        glCompileShader(vertex_shader_id);
        glCompileShader(fragment_shader_id);

        // Se comprueba que si la compilación ha tenido éxito:
        glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error(vertex_shader_id);

        glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error(fragment_shader_id);

        // Se crea un objeto para un programa:
        GLuint program_id = glCreateProgram();

        // Se cargan los shaders compilados en el programa:
        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);

        // Se linkan los shaders:
        glLinkProgram(program_id);

        // Se comprueba si el linkage ha tenido éxito:
        glGetProgramiv(program_id, GL_LINK_STATUS, &succeeded);
        if (!succeeded) show_linkage_error(program_id);

        // Se liberan los shaders compilados una vez se han linkado:
        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);

        return program_id;
    }

    void Spiral::show_compilation_error(GLuint shader_id)
    {
        string info_log;
        GLint  info_log_length;

        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize(info_log_length);

        glGetShaderInfoLog(shader_id, info_log_length, NULL, &info_log.front());

        cerr << info_log.c_str() << endl;

#ifdef _MSC_VER
        //OutputDebugStringA (info_log.c_str ());
#endif

        assert(false);
    }

    void Spiral::show_linkage_error(GLuint program_id)
    {
        string info_log;
        GLint  info_log_length;

        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize(info_log_length);

        glGetProgramInfoLog(program_id, info_log_length, NULL, &info_log.front());

        cerr << info_log.c_str() << endl;

#ifdef _MSC_VER
        //OutputDebugStringA (info_log.c_str ());
#endif

        assert(false);
    }
}