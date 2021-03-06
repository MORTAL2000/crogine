/*-----------------------------------------------------------------------

Matt Marchant 2017 - 2020
http://trederia.blogspot.com

crogine - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

-----------------------------------------------------------------------*/

#pragma once

#include <crogine/Config.hpp>
#include <crogine/detail/Types.hpp>
#include <crogine/detail/SDLResource.hpp>
#include <crogine/graphics/Rectangle.hpp>

#include <crogine/detail/glm/vec2.hpp>

#include <string>

namespace cro
{
    /*!
    \brief Generic texture wrapper for OpenGL RGB or RGBA textures.
    This class is intended for use with mesh texturing, rather than any
    advanced texture usage.
    */
    class CRO_EXPORT_API Texture final : public Detail::SDLResource
    {
    public:
        /*!
        \brief Constructor.
        By default Textures are invalid until create() has been called on
        them, or they have had a successful call to loadFromFile() or
        loadFromMemory()
        */
        Texture();
        ~Texture();

        Texture(const Texture&) = delete;
        Texture(Texture&&);
        Texture& operator = (const Texture&) = delete;
        Texture& operator = (Texture&&);

        /*!
        \brief Creates an empty texture.
        \param width Width of the texture to create. On mobile platfors this should be pow2
        \param height Height of the texture to create. On mobile platforms this should be pow2
        \param format Graphics::Format type - valid values are RGB or RGBA
        */
        void create(uint32 width, uint32 height, ImageFormat::Type format = ImageFormat::RGBA);

        /*!
        \brief Attempts to the the file in the given file path.
        \param path Path to file to load. The image file should have pow2 dimensions on mobile platforms
        \returns true on success, else false
        */
        bool loadFromFile(const std::string& path, bool createMipMaps = false);

        /*!
        \brief Updates the pixel data for the texture.
        Ensure the texture is valid by calling create() or successfully calling loadFromFile()
        before attempting to update it.
        \param pixels Pointer to array containing pixel data. The data must match the same format
        returned by Texture::getFormat() else results are undefined.
        \param area InRect representing the area of the texture to update. If the size is zero
        the entire texture will be updated.
        */
        bool update(const uint8* pixels, bool createMipMaps = false, URect area = {});

        /*!
        \brief Returns the dimensions of the image which makes up this texture
        */
        glm::uvec2 getSize() const;

        /*!
        \brief Returns the current format of the texture
        */
        ImageFormat::Type getFormat() const;

        /*!
        brief Returns the OpenGL handle used by this texture.
        */
        uint32 getGLHandle() const;

        /*!
        \brief Enables texture smoothing
        */
        void setSmooth(bool);

        /*!
        \brief Returns true is texture smoothing is enabled
        */
        bool isSmooth() const;

        /*!
        \brief Enables or disables texture repeating
        */
        void setRepeated(bool);

        /*!
        \brief Returns true if texture repeating is enabled
        */
        bool isRepeated() const;

        /*!
        \brief Returns the max texture size for the current platform
        */
        static uint32 getMaxTextureSize();

    private:
        glm::uvec2 m_size;
        ImageFormat::Type m_format;
        uint32 m_handle;
        bool m_smooth;
        bool m_repeated;
        bool m_hasMipMaps;
    };
}