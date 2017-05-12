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

#ifndef TL_ROCKFALL_HPP_
#define TL_ROCKFALL_HPP_

#include <crogine/ecs/System.hpp>
#include <crogine/core/Clock.hpp>
#include <crogine/core/Wavetable.hpp>

//component
struct RockFall final
{
    enum
    {
        Idle,
        Spawning,
        Shaking,
        Falling
    }state = Idle;
    float velocity = 0.f;
    float stateTime = 0.f;
};

//system
class RockFallSystem final : public cro::System
{
public:
    RockFallSystem(cro::MessageBus&);

    void process(cro::Time) override;

    void handleMessage(const cro::Message&) override;

    void setRunning(bool r) { m_running = r; }

private:

    bool m_running;
    cro::Clock m_clock;
    cro::Wavetable m_wavetable;
};

#endif //TL_ROCKFALL_HPP_