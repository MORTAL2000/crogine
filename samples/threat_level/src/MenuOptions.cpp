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

#include "MainState.hpp"
#include "Slider.hpp"
#include "MyApp.hpp"

#include <crogine/ecs/systems/SpriteRenderer.hpp>
#include <crogine/ecs/systems/TextRenderer.hpp>
#include <crogine/ecs/systems/UISystem.hpp>
#include <crogine/ecs/systems/SceneGraph.hpp>
#include <crogine/ecs/systems/CommandSystem.hpp>
#include <crogine/ecs/systems/DebugInfo.hpp>

#include <crogine/ecs/components/Sprite.hpp>
#include <crogine/ecs/components/Text.hpp>
#include <crogine/ecs/components/UIInput.hpp>
#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/CommandID.hpp>

#include <crogine/graphics/Image.hpp>
#include <crogine/graphics/Font.hpp>
#include <crogine/graphics/SpriteSheet.hpp>

#include <crogine/core/App.hpp>
#include <crogine/core/Clock.hpp>
#include <crogine/detail/GlobalConsts.hpp>
#include <crogine/util/Constants.hpp>

namespace
{
#include "MenuConsts.inl"
}

void MainState::createOptionsMenu(cro::uint32 mouseEnterCallback, cro::uint32 mouseExitCallback,
    const cro::SpriteSheet& spriteSheetButtons, const cro::SpriteSheet& spriteSheetIcons)
{
    auto& menuFont = m_sharedResources.fonts.get(FontID::MenuFont);
    const auto buttonNormalArea = spriteSheetButtons.getSprite("button_inactive").getTextureRect();

    //create an entity to move the menu
    auto controlEntity = m_menuScene.createEntity();
    auto& controlTx = controlEntity.addComponent<cro::Transform>();
    controlTx.setPosition({ -960.f, 624.f, 0.f });
    controlEntity.addComponent<cro::CommandTarget>().ID = CommandID::MenuController;
    controlEntity.addComponent<Slider>();

    auto backgroundEnt = m_menuScene.createEntity();
    backgroundEnt.addComponent<cro::Sprite>() = spriteSheetButtons.getSprite("menu");
    auto size = backgroundEnt.getComponent<cro::Sprite>().getSize();
    backgroundEnt.addComponent<cro::Transform>().setOrigin({ size.x / 2.f, size.y, 0.f });
    backgroundEnt.getComponent<cro::Transform>().setPosition({ 0.f, 140.f, -10.f });
    backgroundEnt.getComponent<cro::Transform>().setParent(controlEntity);

    auto textEnt = m_menuScene.createEntity();
    auto& titleText = textEnt.addComponent<cro::Text>(menuFont);
    titleText.setString("Options");
    titleText.setColour(textColourSelected);
    titleText.setCharSize(TextMedium);
    auto& titleTextTx = textEnt.addComponent<cro::Transform>();
    titleTextTx.setPosition({ -86.f, 110.f, 0.f });
    titleTextTx.setParent(controlEntity);
    
    auto entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetButtons.getSprite("button_inactive");
    auto& backTx = entity.addComponent<cro::Transform>();
    backTx.setPosition({ 0.f, -480.f, 0.f });
    backTx.setParent(controlEntity);
    backTx.setOrigin({ buttonNormalArea.width / 2.f, buttonNormalArea.height / 2.f, 0.f });

    textEnt = m_menuScene.createEntity();
    auto& backText = textEnt.addComponent<cro::Text>(menuFont);
    backText.setString("Back");
    backText.setColour(textColourNormal);
    backText.setCharSize(TextLarge);
    auto& backTexTx = textEnt.addComponent<cro::Transform>();
    backTexTx.setParent(entity);
    backTexTx.move({ 40.f, 100.f, 0.f });

    auto iconEnt = m_menuScene.createEntity();
    iconEnt.addComponent<cro::Transform>().setParent(entity);
    iconEnt.getComponent<cro::Transform>().setPosition({ buttonNormalArea.width - buttonIconOffset, 0.f, 0.f });
    iconEnt.addComponent<cro::Sprite>() = spriteSheetIcons.getSprite("back");

    auto backCallback = m_uiSystem->addCallback([this](cro::Entity, cro::uint64 flags)
    {
        if ((flags & cro::UISystem::LeftMouse)
            || flags & cro::UISystem::Finger)
        {
            cro::Command cmd;
            cmd.targetFlags = CommandID::MenuController;
            cmd.action = [](cro::Entity e, cro::Time)
            {
                auto& slider = e.getComponent<Slider>();
                slider.active = true;
                slider.destination = e.getComponent<cro::Transform>().getPosition() + glm::vec3(-static_cast<float>(cro::DefaultSceneSize.x), 0.f, 0.f);
            };
            m_commandSystem->sendCommand(cmd);
        }
    });
    auto& backControl = entity.addComponent<cro::UIInput>();
    backControl.callbacks[cro::UIInput::MouseUp] = backCallback;
    backControl.callbacks[cro::UIInput::MouseEnter] = mouseEnterCallback;
    backControl.callbacks[cro::UIInput::MouseExit] = mouseExitCallback;
    backControl.area.width = buttonNormalArea.width;
    backControl.area.height = buttonNormalArea.height;

    //build the actual controls

    //music
    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetIcons.getSprite("music");
    entity.getComponent<cro::Sprite>().setColour(textColourSelected);
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().setPosition({ 120.f, 280.f, 0.2f });


    //sfx
    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetIcons.getSprite("effects");
    entity.getComponent<cro::Sprite>().setColour(textColourSelected);
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().setPosition({ 120.f, 140.f, 0.2f });

#ifdef PLATFORM_DESKTOP
    //resolution
    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetButtons.getSprite("arrow_inactive");
    size = entity.getComponent<cro::Sprite>().getSize();
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().rotate({ 0.f, 0.f, 1.f }, cro::Util::Const::PI / 2.f);
    entity.getComponent<cro::Transform>().setOrigin({ size.x / 2.f, size.y / 2.f, 0.f });
    entity.getComponent<cro::Transform>().setPosition({ 166.f, 86.f, 0.2f });

    const auto& resList = cro::App::getWindow().getAvailableResolutions();
    std::string resString;
    for (const auto& r : resList)
    {
        resString += std::to_string(r.x) + "x" + std::to_string(r.y) + "\n";
    }
    resString.pop_back();
    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Text>(m_sharedResources.fonts.get(FontID::ScoreboardFont)).setString(resString);
    entity.getComponent<cro::Text>().setAlignment(cro::Text::Centre);
    entity.getComponent<cro::Text>().setCharSize(TextLarge);
    entity.getComponent<cro::Text>().setColour(textColourSelected);
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().setPosition({ 260.f, 118.f, 0.f });
    entity.getComponent<cro::Text>().setCroppingArea({ 0.f, 0.f, 600.f, -64.f });

    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetButtons.getSprite("arrow_inactive");
    size = entity.getComponent<cro::Sprite>().getSize();
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().rotate({ 0.f, 0.f, 1.f }, -cro::Util::Const::PI / 2.f);
    entity.getComponent<cro::Transform>().setOrigin({ size.x / 2.f, size.y / 2.f, 0.f });
    entity.getComponent<cro::Transform>().setPosition({ 686.f, 86.f, 0.2f });

    //full screen
    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetButtons.getSprite("unchecked_inactive");
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().setOrigin({ size.x / 2.f, size.y / 2.f, 0.f });
    entity.getComponent<cro::Transform>().setPosition({ 866.f, 86.f, 0.2f });


    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Text>(menuFont).setString("Full Screen");
    entity.getComponent<cro::Text>().setCharSize(TextMedium);
    entity.getComponent<cro::Text>().setColour(textColourSelected);
    entity.addComponent<cro::Transform>().setParent(backgroundEnt);
    entity.getComponent<cro::Transform>().setPosition({ 920.f, 106.f, 0.f });

#endif
}