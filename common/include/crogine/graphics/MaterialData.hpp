/*-----------------------------------------------------------------------

Matt Marchant 2017
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

#ifndef CRO_MATERIAL_DATA_HPP_
#define CRO_MATERIAL_DATA_HPP_

#include <crogine/Config.hpp>
#include <crogine/detail/Types.hpp>
#include <crogine/graphics/MeshData.hpp>

namespace cro
{
    namespace Material
    {
        /*!
        \brief Material data held by a model component and used for rendering.
        This should be created exclusively through a MaterialResource instance,
        manually trying to configure this will lead to undefined behaviour
        */
        struct CRO_EXPORT_API Data final
        {
            uint32 shader = 0;
            std::array<std::array<int32, 2u>, Mesh::Attribute::Total> attribs{}; //< maps attrib location to attrib size between shader and mesh
            std::size_t attribCount = 0; //< count of attributes successfully mapped
        };
    }
}

#endif //CRO_MATERIAL_DATA_HPP_