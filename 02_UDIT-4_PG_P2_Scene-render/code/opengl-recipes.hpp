
// Este código es de dominio público
// Miguel Rodríguez Gallego

#pragma once

#include "Color.hpp"
#include "Color_Buffer.hpp"
#include <glad/glad.h>
#include <memory>
#include <SOIL2.h>
#include <string>

namespace udit
{

    GLuint compile_shaders        (const std::string & vertex_shader_code, const std::string & fragment_shader_code);
    void   show_compilation_error (GLuint  shader_id);
    void   show_linkage_error     (GLuint program_id);

    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

    template< typename COLOR_FORMAT >
    std::unique_ptr< Color_Buffer< COLOR_FORMAT > > load_image (const std::string & image_path)
    {
        // Se carga la imagen del archivo usando SOIL2:
        int image_width = 0;
        int image_height = 0;
        int image_channels = 0;

        uint8_t* loaded_pixels = SOIL_load_image
        (
            image_path.c_str(),
            &image_width,
            &image_height,
            &image_channels,
            SOIL_LOAD_RGBA              // Indica que nos devuelva los pixels en formato RGB32
        );                              // al margen del formato usado en el archivo

        // Si loaded_pixels no es nullptr, la imagen se ha podido cargar correctamente:
        if (loaded_pixels)
        {
            auto image = std::make_unique< Color_Buffer<COLOR_FORMAT> >(image_width, image_height);

            // Se copian los bytes de un buffer a otro directamente:
            std::copy_n
            (
                loaded_pixels,
                size_t(image_width) * size_t(image_height) * sizeof(typename Color_Buffer<COLOR_FORMAT>::Color),
                reinterpret_cast<uint8_t*>(image->colors())
            );

            // Se libera la memoria que reservó SOIL2 para cargar la imagen:
            SOIL_free_image_data(loaded_pixels);

            return image;
        }

        return nullptr;
    }

    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

    template<typename COLOR_FORMAT>
    GLuint create_texture_2d(const std::string& texture_path);

}
