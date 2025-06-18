
// Este código es de dominio público
// Miguel Rodríguez Gallego

#include "Rubik.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "CubeFace.hpp"

namespace udit
{
    Rubik::Rubik(float spacing) : spacing(spacing)
    {
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                for (int z = -1; z <= 1; ++z)
                {
                    SubCube sc;
                    sc.base_position = glm::vec3(x, y, z) * spacing;
                    sc.transform = glm::mat4(1.0f);
                    cubes.push_back(sc);
                }
            }
        }
    }
    Rubik::~Rubik()
    {
        //// Se liberan los VBOs y el VAO usados:
        //glDeleteVertexArrays(1, &vao_id);
        //glDeleteBuffers(VBO_COUNT, vbo_ids);
    }

    void Rubik::render
    (
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::mat4& model_global,
        GLint model_view_matrix_id,
        GLint normal_matrix_id,
        GLint material_color_loc,
        GLuint program_id
    )
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST);

        static CubeFace face;   // Caras individuales visibles (planos) para representar las caras del Rubik.
        static const float d = spacing / 2.1f;

        for (const auto& sub : cubes)
        {
            glm::vec3 pos = sub.base_position / spacing;

            // Lista de caras visibles o negras según posición del subcubo
            struct FaceDef 
            {
                glm::vec3 dir;
                glm::vec3 color;
            };

            std::vector<FaceDef> faces = 
            {
                { glm::vec3(+1,  0,  0), (pos.x >  0.9f) ? glm::vec3(1,   0, 0) : glm::vec3(0, 0, 0) },  // Rojo
                { glm::vec3(-1,  0,  0), (pos.x < -0.9f) ? glm::vec3(1, 0.5, 0) : glm::vec3(0, 0, 0) },  // Naranja
                { glm::vec3( 0, +1,  0), (pos.y >  0.9f) ? glm::vec3(1,   1, 1) : glm::vec3(0, 0, 0) },  // Blanco
                { glm::vec3( 0, -1,  0), (pos.y < -0.9f) ? glm::vec3(1,   1, 0) : glm::vec3(0, 0, 0) },  // Amarillo
                { glm::vec3( 0,  0, +1), (pos.z >  0.9f) ? glm::vec3(0,   0, 1) : glm::vec3(0, 0, 0) },  // Azul
                { glm::vec3( 0,  0, -1), (pos.z < -0.9f) ? glm::vec3(0,   1, 0) : glm::vec3(0, 0, 0) },  // Verde
            };

            glUseProgram(program_id);

            for (const auto& f : faces)
            {
                // Rotación para alinear el plano con la cara
                glm::mat4 rot;
                     if (f.dir == glm::vec3( 1,  0,  0)) rot = glm::rotate(glm::mat4(1.0f), glm::radians( 90.f), glm::vec3(0, 1, 0));
                else if (f.dir == glm::vec3(-1,  0,  0)) rot = glm::rotate(glm::mat4(1.0f), glm::radians(-90.f), glm::vec3(0, 1, 0));
                else if (f.dir == glm::vec3( 0,  1,  0)) rot = glm::rotate(glm::mat4(1.0f), glm::radians(-90.f), glm::vec3(1, 0, 0));
                else if (f.dir == glm::vec3( 0, -1,  0)) rot = glm::rotate(glm::mat4(1.0f), glm::radians( 90.f), glm::vec3(1, 0, 0));
                else if (f.dir == glm::vec3( 0,  0, -1)) rot = glm::rotate(glm::mat4(1.0f), glm::radians(180.f), glm::vec3(0, 1, 0));
                else rot = glm::mat4(1.0f); // +Z no necesita rotación

                // Modelo final = transformaciones de Rubik + traslación + orientación cara
                glm::mat4 model =
                    model_global 
                    * sub.transform 
                    * glm::translate(glm::mat4(1.0f), sub.base_position + f.dir * d) 
                    * rot;

                glm::mat4 model_view = view * model;
                glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view));

                glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view));
                glUniformMatrix4fv(    normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));
                      glUniform3fv(  material_color_loc, 1, glm::value_ptr(f.color));

                face.render();
            }
        }
    }

    void Rubik::rotate_row(float row, float speed)
    {
        float radians = glm::radians(speed);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 1, 0));

        // Calcular centro de la capa
        glm::vec3 center(0.0f);
        int count = 0;
        for (const auto& sub : cubes)
        {
            float y = sub.base_position.y / spacing;
            if (glm::abs(y - row) < 0.1f)
            {
                center += sub.base_position;
                count++;
            }
        }

        if (count == 0) return;
        center /= float(count);  // centro promedio de la capa

        // Aplicar rotación relativa al centro
        for (auto& sub : cubes)
        {
            float y = sub.base_position.y / spacing;
            if (glm::abs(y - row) < 0.1f)
            {
                glm::mat4 T1 = glm::translate(glm::mat4(1.0f), -center);
                glm::mat4 T2 = glm::translate(glm::mat4(1.0f), center);
                sub.transform = T2 * rotation * T1 * sub.transform;
            }
        }
    }
}