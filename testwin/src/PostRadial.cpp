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

#include "PostRadial.hpp"

#include <crogine/graphics/shaders/PostVertex.hpp>
#include <crogine/graphics/RenderTexture.hpp>

namespace
{
    const std::string fragment = R"(
        uniform sampler2D u_texture;

        varying vec2 v_texCoord;
        
        void main()
        {
            gl_FragColor = texture2D(u_texture, v_texCoord);
        }       
    )";
}

PostRadial::PostRadial()
{
    m_shader.loadFromString(cro::PostVertex, fragment);
}

//public
void PostRadial::apply(const cro::RenderTexture& source)
{
    setUniform("u_texture", source.getTexture(), m_shader);
    drawQuad(m_shader, { 0.f, 0.f, 800.f, 600.f });
}