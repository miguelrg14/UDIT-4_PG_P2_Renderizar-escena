
// Este código es de dominio público
// Miguel Rodríguez Gallego

#pragma once

#include <memory>
#include <string>
#include <glad/glad.h>
#include <glm.hpp>
#include "Color.hpp"
#include "Color_Buffer.hpp"
#include "Camera.hpp"
#include "Skybox.hpp"
#include "Cube.hpp"
#include "Terrain.hpp"

#include "SceneNode.hpp"

namespace udit
{

    class Scene
    {
    private:

        typedef Color_Buffer< Rgba8888 > Color_Buffer;
        //typedef Color_Buffer< Monochrome8 > Color_Buffer;

        std::unique_ptr<SceneNode> rootNode;

        enum
        {
            COORDINATES_VBO,
            COLORS_VBO,
            INDICES_EBO,
            UVS_VBO,
            VBO_COUNT
        };

        // Postprocesado: Reescalado de la pantalla con framebuffer
        GLsizei  framebuffer_width = 256; // 256; // 1024;
        GLsizei framebuffer_height = 256; // 256; // 1024;

        static const std::string          vertex_shader_code;
        static const std::string        fragment_shader_code;
        static const std::string                texture_path;
        static const std::string        texture_path_terrain;
        static const std::string   effect_vertex_shader_code;
        static const std::string effect_fragment_shader_code;

    public:
        static const std::string terrain_vertex_shader_code;
        static const std::string terrain_fragment_shader_code;

    private:

        GLuint terrain_program_id = 0;
        GLuint height_texture_id = 0;

        GLint           model_view_matrix_id;
        GLint           projection_matrix_id;
        GLint               normal_matrix_id;
        GLint              material_color_id;
        GLuint                framebuffer_id;
        GLuint                depthbuffer_id;
        GLuint                out_texture_id;
        GLuint  terrain_projection_matrix_id;
        GLuint  terrain_model_view_matrix_id;

        /// Terreno (Mallas de elevación)
        Terrain terrain;

        Cube  cube;

        float angle;

        /// Cargar modelos 3D
        GLuint  vao_model, vbo_model, ebo_model;
        GLsizei index_count = 0;

        /// Color aleatorio
        GLuint  vbo_ids[VBO_COUNT];
        GLuint              vao_id;
        GLsizei  number_of_indices;

        /// Cargar texturas
        GLuint          program_id;
        GLuint      texture_id = 0;
        GLuint use_vertex_color_id;
        //GLuint     cube_program_id;
        GLuint   effect_program_id;

        bool      there_is_texture;

        /// Postprocesado
        GLuint     framebuffer_quad_vao;
        GLuint framebuffer_quad_vbos[2];

        int window_width;
        int window_height;

    public:

        /// Cámara
        Camera camera;
        Skybox skybox;

        Scene(unsigned width, unsigned height);
        ~Scene();

        void   update();
        void   render();
        void   resize(unsigned width, unsigned height);
        //void   load_model   (const std::string& path);

    private:

        /// Postprocesado
        void   build_framebuffer();
        void   render_framebuffer();

        void load_mesh
        (
            const std::string& mesh_file_path, 
            const std::string& texture_file_path, 
            const glm::mat4& localTransform = glm::mat4(1.0f)
        );
        glm::vec3   random_color();

        void   configure_material(GLuint program_id);
        void   configure_light(GLuint program_id);
    };
}