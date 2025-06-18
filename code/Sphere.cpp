
// Este código es de dominio público
// Miguel Rodríguez Gallego

#include "Sphere.hpp"
#include <vector>
#include <cmath>

namespace udit
{
    Sphere::Sphere()
    {
        indexCount = 0;
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);

        Subdivision(6);

        glBindVertexArray(0);
    }

    Sphere::~Sphere()
    {
        glDeleteBuffers(VBO_COUNT, vbo_ids);
        glDeleteVertexArrays(1, &vao_id);
    }

    void Sphere::Subdivision(int subdivisions)
    {
        std::vector<GLfloat> coords;
        std::vector<GLfloat> cols;
        std::vector<GLuint > inds;

        const int stacks = subdivisions;
        const int slices = subdivisions * 2; // más segmentos en horizontal
        const float radius = 1.0f;

        const float PI = 3.141592653589f;

        for (int i = 0; i <= stacks; ++i)
        {
            float v = float(i) / stacks;
            float phi = v * PI; // de 0 a PI

            for (int j = 0; j <= slices; ++j)
            {
                float u = float(j) / slices;
                float theta = u * 2.0f * PI; // de 0 a 2PI

                float x = radius * sin(phi) * cos(theta);
                float y = radius * cos(phi);
                float z = radius * sin(phi) * sin(theta);

                coords.push_back(x);
                coords.push_back(y);
                coords.push_back(z);

                // Color según altura
                cols.push_back((y + 1.0f) * 0.5f); // rojo
                cols.push_back(0.5f);             // verde
                cols.push_back(1.0f - (y + 1.0f) * 0.5f); // azul
            }
        }

        // Generar índices
        for (int i = 0; i < stacks; ++i)
        {
            for (int j = 0; j < slices; ++j)
            {
                int first = i * (slices + 1) + j;
                int second = first + slices + 1;

                inds.push_back(first);
                inds.push_back(first + 1);
                inds.push_back(second);

                inds.push_back(second);
                inds.push_back(first + 1);
                inds.push_back(second + 1);
            }
        }

        indexCount = inds.size();

        glBindVertexArray(vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(GLfloat), coords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COLORS_VBO]);
        glBufferData(GL_ARRAY_BUFFER, cols.size() * sizeof(GLfloat), cols.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }


    void Sphere::render()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}
