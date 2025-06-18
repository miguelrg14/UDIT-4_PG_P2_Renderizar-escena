
// Este código es de dominio público
// Miguel Rodríguez Gallego

#include "CubeFace.hpp"

namespace udit
{
    const GLfloat CubeFace::coordinates[] =
    {
        -1.f, -1.f, 0.f,
        +1.f, -1.f, 0.f,
        +1.f, +1.f, 0.f,
        -1.f, +1.f, 0.f
    };

    const GLfloat CubeFace::normals[] =
    {
         0.f,  0.f, +1.f,
         0.f,  0.f, +1.f,
         0.f,  0.f, +1.f,
         0.f,  0.f, +1.f,
    };

    const GLubyte CubeFace::indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    CubeFace::CubeFace()
    {
        // Se generan índices para los VBOs del cubo:
        glGenBuffers(VBO_COUNT, vbo_ids);
        glGenVertexArrays(1, &vao_id);

        // Se activa el VAO del cubo para configurarlo:
        glBindVertexArray(vao_id);

        // Se suben a un VBO los datos de coordenadas y se vinculan al VAO:
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Se suben a un VBO los datos de normales y se vinculan al VAO:
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMALS_VBO]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Se suben a un IBO los datos de índices:
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_IBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    CubeFace::~CubeFace()
    {
        // Se liberan los VBOs y el VAO usados:
        glDeleteVertexArrays(1, &vao_id);
        glDeleteBuffers(VBO_COUNT, vbo_ids);
    }

    void CubeFace::render()
    {
        // Se selecciona el VAO que contiene los datos del objeto y se dibujan sus elementos:
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);
    }
}