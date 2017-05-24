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

#include <crogine/core/Clock.hpp>
#include <crogine/ecs/systems/SkeletalAnimator.hpp>
#include <crogine/ecs/components/Model.hpp>
//#include <crogine/graphics/MaterialData.hpp>

using namespace cro;

SkeletalAnimator::SkeletalAnimator(MessageBus& mb)
    : System(mb, typeid(SkeletalAnimator))
{
    requireComponent<Model>();
    requireComponent<Skeleton>();
}

//public
void SkeletalAnimator::process(Time dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        //get skeleton

        //update current frame if running
        //only interpolate if visible (frustum cull?)

        //get model then for each material set uniform for current frame
        //if we can cache the uniform ID somewhere, then even better.

        //TODO applying matrix arrays in renderer
        //TODO applying skinning in shader
    }
}

//private
void SkeletalAnimator::interpolate(const Skeleton::Frame& a, const Skeleton::Frame& b, float time, Skeleton::Frame& output)
{

}