// Este código es de dominio público
// Miguel Rodríguez Gallego

#pragma once

#include <vector>
#include <memory>
#include <glm.hpp>                          // vec3, mat4
#include <gtc/type_ptr.hpp>                // value_ptr
#include <glad/glad.h>                     // GLuint
#include "Mesh.hpp"                         // Definición completa de Mesh

namespace udit
{
    class SceneNode
    {
    public:
        glm::mat4 localTransform = glm::mat4(1.0f);
        std::vector<std::shared_ptr<Mesh>> meshes;         // Mallas en este nodo
        std::vector<std::unique_ptr<SceneNode>> children;  // Hijos en la jerarquía

        // Añade una malla a este nodo
        void addMesh(const std::shared_ptr<Mesh>& m)
        {
            meshes.push_back(m);
        }

        // Añade un nodo hijo a este nodo
        void addChild(std::unique_ptr<SceneNode> child)
        {
            children.push_back(std::move(child));
        }

        // Recorrido y dibujo recursivo
        void draw(const glm::mat4& parentTransform, GLuint shaderProgram)
        {
            // 1) Calculamos transformación global
            glm::mat4 globalTransform = parentTransform * localTransform;

            // 2) Subimos uniforms del shader
            GLint mvLoc = glGetUniformLocation(shaderProgram, "model_view_matrix");
            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(globalTransform));

            glm::mat4 normalMat = glm::transpose(glm::inverse(globalTransform));
            GLint nmLoc = glGetUniformLocation(shaderProgram, "normal_matrix");
            glUniformMatrix4fv(nmLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

            // 3) Dibujamos cada malla
            for (auto& m : meshes)
            {
                m->draw(globalTransform, shaderProgram);
            }

            // 4) Recorremos y dibujamos hijos
            for (auto& child : children)
            {
                child->draw(globalTransform, shaderProgram);
            }
        }
    };

}
