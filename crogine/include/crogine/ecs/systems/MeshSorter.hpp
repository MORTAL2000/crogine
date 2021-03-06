/*-----------------------------------------------------------------------

Matt Marchant 2017
http://trederia.blogspot.com

crogine test application - Zlib license.

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

#ifndef CRO_MESH_SORTER_HPP_
#define CRO_MESH_SORTER_HPP_

#include <crogine/ecs/System.hpp>
#include <crogine/graphics/MaterialData.hpp>
#include <crogine/ecs/systems/SceneRenderer.hpp>

#include <glm/mat4x4.hpp>

namespace cro
{
    class Entity;
    class SceneRenderer;
    class MessageBus;
    /*!
    \brief Culls and sorts entities into draw lists for renderer.
    \deprecated - this is done by default in the ModelRenderer system.
    */
    class CRO_EXPORT_API MeshSorter final : public System
    {
    public:
        /*!
        \brief Constructor
        \param mb Reference to the system message bus
        \param renderer Reference to renderer used to draw
        the meshes sorted by this system
        */
        MeshSorter(MessageBus& mb,SceneRenderer& renderer);

        /*!
        \brief Automatically executed once per fram by the ECS.
        Performs culling and sorting of entities
        */
        void process(Time) override;

    private:

        SceneRenderer& m_renderer;
        MaterialList m_visibleEntities;
    };
}

#endif//CRO_MESH_SORTER_HPP_