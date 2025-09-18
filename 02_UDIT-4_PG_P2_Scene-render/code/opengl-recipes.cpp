
// Este código es de dominio público
// Miguel Rodríguez Gallego

#include "opengl-recipes.hpp"

#include <SDL.h>

using namespace std;

namespace udit
{

    GLuint compile_shaders(const std::string& vertex_shader_code, const std::string& fragment_shader_code)
    {
        GLint succeeded = GL_FALSE;

        // Se crean objetos para los shaders:
        GLuint   vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        // Se carga el código de los shaders:
        const char*    vertex_shaders_code[] = { vertex_shader_code.c_str() };
        const char*  fragment_shaders_code[] = { fragment_shader_code.c_str() };
        const GLint    vertex_shaders_size[] = { GLint(vertex_shader_code.size()) };
        const GLint  fragment_shaders_size[] = { GLint(fragment_shader_code.size()) };

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

        return (program_id);
    }

    GLuint compile_shaders()
    {
        GLint succeeded = GL_FALSE;

        // Se crean objetos para los shaders:
        GLuint   vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

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

        return (program_id);
    }

    template<typename COLOR_FORMAT>
    GLuint create_texture_2d(const std::string& texture_path)
    {
        auto image = load_image<COLOR_FORMAT>(texture_path);

        if (image)
        {
            GLuint texture_id;

            texture_id = SOIL_load_OGL_texture
            (
                texture_path.c_str(),   // Ruta de la textura
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
            );

            glEnable(GL_TEXTURE_2D);
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                image->get_width(),
                image->get_height(),
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                image->colors()
            );

            glGenerateMipmap(GL_TEXTURE_2D);

            return texture_id;
        }

        return -1;
    }

    template<>
    GLuint create_texture_2d<GLuint>(const std::string& texture_path)
    {
        auto image = load_image<GLuint>(texture_path);

        if (image)
        {
            GLuint texture_id;

            texture_id = SOIL_load_OGL_texture
            (
                texture_path.c_str(),   // Ruta de la textura
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
            );

            glEnable(GL_TEXTURE_2D);
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                image->get_width(),
                image->get_height(),
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                image->colors()
            );

            glGenerateMipmap(GL_TEXTURE_2D);

            return texture_id;
        }

        return -1;
    }


    template<>
    GLuint create_texture_2d<Monochrome8>(const std::string& texture_path)
    {
        auto image = load_image< Monochrome8 >(texture_path);

        if (image)
        {
            GLuint texture_id;

            glEnable(GL_TEXTURE_2D);
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_R8,
                image->get_width(),
                image->get_height(),
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                image->colors()
            );

            glGenerateMipmap(GL_TEXTURE_2D);

            return texture_id;
        }

        return -1;
    }

    /// ------------------ ERRORES (Utilidades) -----------------

    void show_compilation_error (GLuint shader_id)
    {
        static auto message = "Error compiling a shader.";

        string info_log;
        GLint  info_log_length;

        glGetShaderiv (shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize (info_log_length);

        glGetShaderInfoLog (shader_id, info_log_length, NULL, &info_log.front ());

        SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, message, info_log.c_str (), nullptr);

        throw message;
    }

    void show_linkage_error (GLuint program_id)
    {
        static auto message = "Error linking the shaders.";

        string info_log;
        GLint  info_log_length;

        glGetProgramiv (program_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize (info_log_length);

        glGetProgramInfoLog (program_id, info_log_length, NULL, &info_log.front ());

        SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, message, info_log.c_str (), nullptr);

        throw message;
    }

    /// ------------------------------------------------------

}
