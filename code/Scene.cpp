
// Este código es de dominio público
// Miguel Rodríguez Gallego

#pragma once

#include "Scene.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <glm.hpp>                          // vec3, vec4, ivec4, mat4
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr

// Importar objetos (.obj) a escena
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Cargar texturas
#include <SOIL2.h>

#include "opengl-recipes.hpp"

#include "AssimpMesh.hpp"

using namespace std;
using namespace glm;

namespace udit
{
    const std::string Scene::vertex_shader_code = 
        R"(
            #version 330

            // Definición del struct que describe una luz puntual
            struct Light
            {
                vec4 position;        // Posición de la luz en espacio ojo (eye-space)
                vec3 color;           // Color/intensidad de la luz (RGB)
            };

            // Matrices uniformes enviadas desde la CPU/C++
            uniform mat4 model_view_matrix;   // Modelo + vista: lleva coordenadas de modelo a eye-space
            uniform mat4 projection_matrix;   // Proyección de cámara (perspectiva u ortográfica)
            uniform mat4 normal_matrix;       // Matriz para transformar normales correctamente

            // Parámetros de iluminación especular
            uniform float specular_intensity; // Intensidad global del componente especular
            uniform float shininess;          // Exponente de “dureza” del brillo
            uniform vec3  specular_color;     // Color del brillo especular

            // Parámetros de la luz y los componentes
            uniform Light light;              // Datos de la luz (posición + color)
            uniform float ambient_intensity;  // Intensidad de luz ambiental
            uniform float diffuse_intensity;  // Intensidad de luz difusa

            // Propiedades del material
            uniform vec3 material_color;      // Color base del material (difuso)

            // Niebla: rango y color
            uniform float fog_near;           // distancia mínima donde empieza a entrar la niebla
            uniform float fog_far;            // distancia a la que la niebla ya es completa
            uniform vec3  fog_color;          // color de la niebla

            // Atributos de vértice (entradas del VAO)
            layout(location = 0) in vec3 vertex_coordinates;
            layout(location = 1) in vec3 vertex_normal;
            layout(location = 2) in vec2 vertex_uv;

            // Salidas al fragment shader
            out vec3  front_color;            // color iluminado (sin texturizar)
            out vec2  texture_uv;             // pasamos las coordenadas UV
            out float fog_factor;             // intensidad de la niebla [0..1]

            void main() 
            {
                // 1) Transformar posición a eye-space
                vec4 pos_view = model_view_matrix * vec4(vertex_coordinates, 1.0);

                // 2) Iluminación Phong (ambient + diffuse + specular)
                vec3 N = normalize((normal_matrix * vec4(vertex_normal, 0.0)).xyz);
                vec3 L = normalize((light.position - pos_view).xyz);
                vec3 V = normalize(-pos_view.xyz);
                float diff = diffuse_intensity * max(dot(N, L), 0.0);
                vec3 H = normalize(L + V);
                float spec = specular_intensity * pow(max(dot(N, H), 0.0), shininess);

                // 3) Componente iluminado sin texturizar
                vec3 lit_color = ambient_intensity * material_color
                               + diff * light.color * material_color
                               + spec * specular_color;

                // 4) Cálculo del factor de niebla según la distancia en eye-space
                float d = -pos_view.z;  // distancia desde la cámara (eye-space)
                fog_factor = clamp((d - fog_near) / (fog_far - fog_near), 0.0, 1.0);

                // 5) Pasar al fragment shader sin mezclar aún con la niebla
                front_color = lit_color;
                texture_uv  = vertex_uv;

                // 6) Posición final en clip-space
                gl_Position = projection_matrix * pos_view;
            }
        )";

    const std::string Scene::fragment_shader_code = 
        R"(
            #version 330

            // Textura y niebla
            uniform sampler2D sampler;        // unidad 0: tu textura 2D
            uniform vec3      fog_color;      // color de la niebla

            // Entradas desde el vertex shader
            in  vec2  texture_uv;
            in  vec3  front_color;            // color iluminado (RGB)
            in  float fog_factor;             // [0 = limpio, 1 = niebla completa]

            // Salida
            out vec4 fragment_color;

            void main() 
            {
                // Si ya estamos en niebla completa, descartamos el fragmento
                if (fog_factor >= 1.0)
                {
                    discard;    // desaparece en la niebla
                }

                vec4 texcol = texture(sampler, texture_uv);             // 1) Muestreamos la textura (rgba)

                vec3 littex = front_color * texcol.rgb;                 // 2) Iluminación * textura.rgb

                vec3 final_rgb   = mix(littex, fog_color, fog_factor);  // 3) Mezcla RGB con el color de la niebla

                float final_alpha = texcol.a * (0.5 - fog_factor);      // 4) Atenuar el αlpha original según la niebla

                fragment_color = vec4(final_rgb, final_alpha);          // Resultado final: color + transparencia progresiva

                // fragment_color = vec4(front_color, 1.0); // Prueba con solo color
            }
        )";

    /// Vertex Shader para renderizar el quad de post-procesado
    const std::string Scene::effect_vertex_shader_code = 
        R"(
            #version 330

            /// Atributos de entrada (VAO):
            layout (location = 0) in vec3 vertex_coordinates; // Posición del vértice en clip-space (-1 a +1)
            layout (location = 1) in vec2 vertex_texture_uv;  // Coordenadas UV para muestrear la textura

            /// Salida al fragment shader:
            out vec2 texture_uv;  // Se pasa la UV para usar en el muestreo

            void main()
            {
                // 1) Asigna la posición directamente (ya está en clip-space)
                gl_Position = vec4(vertex_coordinates, 1.0);
                // 2) Propaga el UV al fragment shader
                texture_uv  = vertex_texture_uv;
            }
        )";

    /// Fragment Shader de ejemplo para un efecto simple
    const std::string Scene::effect_fragment_shader_code = 
        R"(
            #version 330

            /// Uniform para la textura renderizada en el framebuffer
            uniform sampler2D sampler2d;

            /// Entrada desde el vertex shader:
            in  vec2 texture_uv;      // Coordenadas UV interpoladas

            /// Salida del fragment shader:
            out vec4 fragment_color;  // Color final del fragmento

            void main()
            {
                /// Ejemplo de efecto: tono sepia amortiguado
                // 1) Muestrea el color original de la textura
                   vec3 color = texture (sampler2d, texture_uv.st).rgb;
                // 2) Convierte a intensidad luminosa promedio
                   float i = (color.r + color.g + color.b) * 0.3333333333;
                // 3) Aplica un tinte amarronado (sepia suave)
                   vec3 sepia = vec3(1.0, 0.75, 0.5);
                   fragment_color = vec4(vec3(i, i, i) * sepia, 1.0);

                /// Alternativa: (Aplicar textura original sin modificaciones)
                //fragment_color = texture(sampler2d, texture_uv);
            }
        )";

    // definición de los dos static members
    const std::string udit::Scene::terrain_vertex_shader_code = 
        R"(
            #version 330

            uniform mat4 model_view_matrix;
            uniform mat4 projection_matrix;

            layout(location = 0) in vec2 vertex_xz;
            layout(location = 1) in vec2 vertex_uv;

            uniform sampler2D sampler;
            uniform float     max_height;

            out float intensity;

            void main()
            {
                float sample = texture(sampler, vertex_uv).r;
                intensity    = sample * 0.75 + 0.25;
                float height = sample * max_height;
                vec4  xyzw   = vec4(vertex_xz.x, height, vertex_xz.y, 1.0);

                gl_Position  = projection_matrix * model_view_matrix * xyzw;
            }
        )";
    const std::string udit::Scene::terrain_fragment_shader_code = 
        R"(
            #version 330

            in  float intensity;
            out vec4  fragment_color;

            void main()
            {
                fragment_color = vec4(intensity, intensity, intensity, 1.0);
            }
        )";

    const string Scene::texture_path_terrain = "../assets/height-map.png";

    Scene::Scene(unsigned width, unsigned height)
        : 
        camera(float(width) / float(height)),
        skybox("../assets/sky-cube-map-"),
        angle(0),
        terrain(10.f, 10.f, 50, 50)
    {
        /// Grafo
        rootNode = std::make_unique<SceneNode>();

        /// Terreno
        // 1) Compilar shaders del terreno
        terrain_program_id = compile_shaders(terrain_vertex_shader_code,terrain_fragment_shader_code);

        // 2) Cargar la textura de alturas (canal R de un PNG, por ejemplo)
        height_texture_id = create_texture_2d<GLuint>("../assets/height-map.png");

        // 3) Configurar uniforms estáticos
        glUseProgram(terrain_program_id);
        // sampler en la unidad 1
        GLint loc = glGetUniformLocation(terrain_program_id, "sampler");
        glUniform1i(loc, 1);
        // altura máxima = 5 unidades (o el valor que quieras)
        loc = glGetUniformLocation(terrain_program_id, "max_height");
        glUniform1f(loc, 5.0f);
        ///

        /// Postprocesado
        // Se crea la textura y se dibuja algo en ella:
        build_framebuffer();

        // Se compilan y se activan los shaders:
               program_id = compile_shaders(vertex_shader_code, fragment_shader_code);
        effect_program_id = compile_shaders(effect_vertex_shader_code, effect_fragment_shader_code);

        glUseProgram(program_id);

        /// Ids del shader
        // Modelos
        model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");
            normal_matrix_id = glGetUniformLocation(program_id, "normal_matrix"    );
           material_color_id = glGetUniformLocation(program_id, "material_color"   );
        // Terreno
        terrain_projection_matrix_id = glGetUniformLocation(terrain_program_id, "projection_matrix");
        terrain_model_view_matrix_id = glGetUniformLocation(terrain_program_id, "model_view_matrix");

        /// Niebla
        // Se configura la niebla:
        GLint  fog_near = glGetUniformLocation(program_id, "fog_near" );
        GLint   fog_far = glGetUniformLocation(program_id, "fog_far"  );
        GLint fog_color = glGetUniformLocation(program_id, "fog_color");

        glUniform1f( fog_near, 30.0f);   // Donde aparece niebla
        glUniform1f(  fog_far, 50.0f);   // Donde es completamente niebla
        glUniform3f(fog_color, 0.8f, 0.8f, 0.9f);

        configure_material(program_id);
           configure_light(program_id);

        // Se establece la configuración básica:
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.f, 0.f, 0.f, 1.f);

        resize(width, height);

        load_mesh("../assets/Terreno.obj", "../assets/Stone_Base_Color.png", glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, 0.0f)));
        load_mesh("../assets/Painting.obj", "../assets/Frame1.jpg", glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, -3.0f)));
        load_mesh("../assets/Plant.obj", "../assets/plant1_Material.001_BaseColor.png", glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -1.0f, 0.0f)));
        load_mesh("../assets/Umbrella.obj", "../assets/Paraguas_DefaultMaterial_BaseColor.png", glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, 0.0f, 0.0f)));
    }

    Scene::~Scene()
    {
        glDeleteVertexArrays(1, &vao_id);
        glDeleteBuffers(VBO_COUNT, vbo_ids);

        glDeleteVertexArrays(1, &framebuffer_quad_vao);
        glDeleteBuffers(2, framebuffer_quad_vbos);

        glDeleteProgram(program_id);

        if (there_is_texture)
        {
            glDeleteTextures(1, &texture_id);
        }
    }

    void Scene::update ()
    {
        angle += 0.01f; // Rotación de la escena en tiempo real
    }


    void Scene::render()
    {
        /// ——— 1ª PASADA: render a FRAMEBUFFER ———
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// 1.1) Skybox (dibujar siempre al fondo)
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skybox.render(camera);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        /// 1.2) Precálculo de las matrices de cámara
        const glm::mat4              view = camera.get_transform_matrix_inverse();
        const glm::mat4 camera_projection = camera.get_projection_matrix();

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        /// 1.3 Mallas de elevación de terreno (que hacen uso de shaders)
        // 1.3.1) Terreno
        glUseProgram(terrain_program_id);

        // textura de alturas en unidad 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, height_texture_id);

        // Subir matrices para terreno
        glUniformMatrix4fv(terrain_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(camera_projection));
        glUniformMatrix4fv(terrain_model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(view));

        // Dibujado de la malla de terreno
        terrain.render();
        ///
        // 1.3.2) Espiral
        glm::mat4 model_spiral = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, -10.0f));
        spiral.render(camera, model_spiral);
        ///

        // 1.4) Objetos opacos (cube u otros meshes)
        glUseProgram(program_id);

        /// Bind de textura difusa si existe
        if (there_is_texture) 
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glUniform1i(glGetUniformLocation(program_id, "sampler"), 0);
        }

        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(camera_projection));

        rootNode->draw(view, program_id);
        ///
        // Dibujar VAO principal
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, number_of_indices, GL_UNSIGNED_SHORT, nullptr);

        /// Cubo de Rubik
        //glm::mat4         model_rubik = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
        //rubik.render(view, camera_projection, model_rubik, model_view_matrix_id, normal_matrix_id, material_color_id, program_id);
        //
        //// Rotaciones
        //rubik.rotate_row(+1.0f,  1.0f); // Capa superior (Y = +1, rota un grado por frame)
        //rubik.rotate_row( 0.0f,  0.0f); // Capa central
        //rubik.rotate_row(-1.0f, -1.0f); // Capa inferior
        ///

        /// Esfera
        glm::mat4      model_sphere = glm::translate (glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, -10.0f)) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model_view_sphere = view * model_sphere;
        glm::mat4     normal_sphere = glm::transpose (glm::inverse(model_view_sphere));

        glUniformMatrix4fv (model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_sphere));
        glUniformMatrix4fv (    normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_sphere    ));

        sphere.render();

        // Esfera 2
        glm::mat4   model_view_sphere2(1);

                    model_view_sphere2 = glm::translate(model_view_sphere2, glm::vec3(5.f, 0.f, 0.f));
                    model_view_sphere2 =     glm::scale(model_view_sphere2, glm::vec3(0.5f));
                    model_view_sphere2 =    glm::rotate(model_view_sphere2, angle * 5.f, glm::vec3(0.f, 1.f, 0.f));

                    model_view_sphere2 = view * model_sphere * model_view_sphere2;

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_sphere2));

        sphere.render();
        /// Esfera

        /// 1.5) Objetos transparentes (tal como ya lo tenías)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        
        glm::mat4   model_view_transparent_cube(1);

        model_view_transparent_cube = glm::translate(model_view_transparent_cube, glm::vec3(10.f, 0.f, 0.f));
        model_view_transparent_cube = glm::scale(model_view_transparent_cube, glm::vec3(0.5f));
        model_view_transparent_cube = glm::rotate(model_view_transparent_cube, angle * 5.f, glm::vec3(0.f, 1.f, 0.f));

        model_view_transparent_cube = view * model_view_transparent_cube;

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_transparent_cube));

        cube.render();  // o tu lógica específica de transparencias

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        ///

        /// ——— 2ª PASADA: post‐procesado a pantalla ———
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);

        glDisable(GL_DEPTH_TEST);
        glUseProgram(effect_program_id);

        glBindVertexArray(framebuffer_quad_vao);
        glBindTexture(GL_TEXTURE_2D, out_texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    /// <summary>
    ///  OpenGL adapta el campo visual horizontal/vertical según la nueva forma de la ventana si se cambia su tamaño
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void Scene::resize (unsigned width, unsigned height)
    {
        window_width  = width;
        window_height = height;

        glm::mat4 projection_matrix = glm::perspective (20.f, GLfloat(width) / height, 1.f, 5000.f);

        glUniformMatrix4fv (projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glViewport (0, 0, width, height);
    }

    /// ------------------ POSTPROCESADO ------------------

    void Scene::build_framebuffer()
    {
        // Se crea un framebuffer en el que poder renderizar:
        {
            glGenFramebuffers(1, &framebuffer_id);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        }

        // Se crea una textura que será el búffer de color vinculado al framebuffer:
        {
            glGenTextures(1, &out_texture_id);
            glBindTexture(GL_TEXTURE_2D, out_texture_id);

            // El búfer de color tendrá formato RGB:

            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGB,
                framebuffer_width,
                framebuffer_height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                0
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        // Se crea un Z-Buffer para usarlo en combinación con el framebuffer:
        {
            glGenRenderbuffers(1, &depthbuffer_id);
            glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer_id);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer_id);
        }

        // Se configura el framebuffer:
        {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, out_texture_id, 0);

            const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;

            glDrawBuffers(1, &draw_buffer);
        }

        // Se comprueba que el framebuffer está listo:

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        // Se desvincula el framebuffer:

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Se crea la malla del quad necesario para hacer el render del framebuffer:

        static const GLfloat quad_positions[] =
        {
            +1.0f, -1.0f, 0.0f,
            +1.0f, +1.0f, 0.0f,
            -1.0f, +1.0f, 0.0f,
            -1.0f, +1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            +1.0f, -1.0f, 0.0f,
        };

        static const GLfloat quad_texture_uvs[] =
        {
            +1.0f,  0.0f,
            +1.0f, +1.0f,
             0.0f, +1.0f,
             0.0f, +1.0f,
             0.0f,  0.0f,
             1.0f,  0.0f,
        };

        glGenVertexArrays(1, &framebuffer_quad_vao);
        glGenBuffers(2, framebuffer_quad_vbos);

        glBindVertexArray(framebuffer_quad_vao);

        glBindBuffer(GL_ARRAY_BUFFER, framebuffer_quad_vbos[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_positions), quad_positions, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, framebuffer_quad_vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texture_uvs), quad_texture_uvs, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    void Scene::render_framebuffer()
    {
        glViewport(0, 0, window_width, window_height);

        // Se activa el framebuffer de la ventana:

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(effect_program_id);

        // Se activa la textura del framebuffer y se renderiza en la ventana:

        glBindTexture(GL_TEXTURE_2D, out_texture_id);

        glBindVertexArray(framebuffer_quad_vao);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    ///----------------------------------------------------

    /// <summary>
    ///     Importa un modelo 3D a la escena
    /// </summary>
    /// <param name="path"></param>
    void Scene::load_mesh
    (
        const std::string& mesh_file_path,
        const std::string& texture_file_path,
        const glm::mat4&   localTransform
    )
    {
        auto mesh = std::make_shared<AssimpMesh>();

        // --- Carga aquí la textura y pásasela al mesh:
        GLuint tex = create_texture_2d<GLuint>(texture_file_path);
        if (tex == GLuint(-1)) 
        {
            std::cerr << "[Scene] error cargando textura: " << texture_file_path << "\n";
        }
        mesh->setTextureID(tex);

        // --- Ahora carga solo la geometría:
        mesh->load(mesh_file_path);

        auto node = std::make_unique<SceneNode>();
        node->addMesh(mesh);
        node->localTransform = localTransform;
        rootNode->addChild(std::move(node));
    }

    void Scene::configure_material(GLuint program_id)
    {
        GLint material_color = glGetUniformLocation(program_id, "material_color");

        glUniform3f(material_color, 1.f, 1.f, 1.f);
    }

    void Scene::configure_light(GLuint program_id)
    {
        GLint    light_position = glGetUniformLocation(program_id, "light.position"    );
        GLint       light_color = glGetUniformLocation(program_id, "light.color"       );
        GLint ambient_intensity = glGetUniformLocation(program_id, "ambient_intensity" );
        GLint diffuse_intensity = glGetUniformLocation(program_id, "diffuse_intensity" );
        GLint      spec_int_loc = glGetUniformLocation(program_id, "specular_intensity");
        GLint     shininess_loc = glGetUniformLocation(program_id, "shininess"         );
        GLint    spec_color_loc = glGetUniformLocation(program_id, "specular_color"    );

        glUniform4f(light_position   , 10.0f, 10.f, 10.f, 1.f);
        glUniform3f(light_color      , 1.0f, 1.f, 1.f        );
        glUniform1f(ambient_intensity, 0.2f                  );
        glUniform1f(diffuse_intensity, 0.8f                  );
        glUniform1f(spec_int_loc     , 1.0f                  ); // fuerza del brillo
        glUniform1f(shininess_loc    , 32.0f                 ); // “dureza” del material
        glUniform3f(spec_color_loc   , 1.0f, 1.0f, 1.0f      ); // color del reflejo (blanco)
    }
    
    glm::vec3 Scene::random_color()
    {
        return glm::vec3
        (
            float(rand()) / float(RAND_MAX),
            float(rand()) / float(RAND_MAX),
            float(rand()) / float(RAND_MAX)
        );
    }

}

