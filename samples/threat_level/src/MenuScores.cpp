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
#include <crogine/ecs/components/UIDraggable.hpp>
#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/CommandID.hpp>
#include <crogine/ecs/components/Callback.hpp>

#include <crogine/graphics/Image.hpp>
#include <crogine/graphics/Font.hpp>
#include <crogine/graphics/SpriteSheet.hpp>

#include <crogine/core/App.hpp>
#include <crogine/core/Clock.hpp>
#include <crogine/core/ConfigFile.hpp>
#include <crogine/detail/GlobalConsts.hpp>
#include <crogine/util/Random.hpp>
#include <crogine/util/Maths.hpp>
#include <crogine/util/Constants.hpp>
    
#include <crogine/android/Android.hpp>

#include <crogine/detail/glm/gtx/norm.hpp>

#include <string>

namespace
{
#include "MenuConsts.inl"

    const float scrollSpeed = 360.f;
}

using Score = std::pair<std::string, std::string>;

void MainState::createScoreMenu(cro::uint32 mouseEnterCallback, cro::uint32 mouseExitCallback,
    const cro::SpriteSheet& spriteSheetButtons, const cro::SpriteSheet& spriteSheetIcons)
{
    auto& menuFont = m_sharedResources.fonts.get(FontID::MenuFont);
    auto& scoreboardFont = m_sharedResources.fonts.get(FontID::ScoreboardFont);

    const auto buttonNormalArea = spriteSheetButtons.getSprite("button_inactive").getTextureRect();

    //create an entity to move the menu
    auto controlEntity = m_menuScene.createEntity();
    auto& controlTx = controlEntity.addComponent<cro::Transform>();
    controlTx.setPosition({ 2880.f, 624.f, 0.f });
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
    titleText.setString("Scores");
    titleText.setColour(textColourSelected);
    titleText.setCharSize(TextMedium);
    auto& titleTextTx = textEnt.addComponent<cro::Transform>();
    titleTextTx.setPosition({ -84.f, 110.f, 0.f });
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
                slider.destination = e.getComponent<cro::Transform>().getPosition() + glm::vec3(static_cast<float>(cro::DefaultSceneSize.x), 0.f, 0.f);
            };
            m_commandSystem->sendCommand(cmd);
        }
    });
    auto& backControl = entity.addComponent<cro::UIInput>();
    backControl.callbacks[cro::UIInput::MouseDown] = backCallback;
    backControl.callbacks[cro::UIInput::MouseEnter] = mouseEnterCallback;
    backControl.callbacks[cro::UIInput::MouseExit] = mouseExitCallback;
    backControl.area.width = buttonNormalArea.width;
    backControl.area.height = buttonNormalArea.height;


    //load scores if we can, fill with fake if not yet created
    //yes these are plain text but you're daft to rig your own high scores
    cro::ConfigFile scores;
    if (!scores.loadFromFile(cro::App::getPreferencePath() + highscoreFile))
    {
        std::array<std::string, 10u> names =
        {
            "Jo", "Rick", "Sam", "Dmitri", "Lela",
            "Jeff", "Karen", "Marcus", "Chloe", "Cole"
        };
        for (auto i = 0u; i < 10u; ++i)
        {
            scores.addProperty(std::to_string(cro::Util::Random::value(10000, 500000)), names[i]);
        }
        if (!scores.save(cro::App::getPreferencePath() + highscoreFile))
        {
            cro::Logger::log("Failed saving default score file", cro::Logger::Type::Warning);
        }
    }

    std::vector<Score> scoreList;
    const auto& scoreValues = scores.getProperties();
    for (const auto& s : scoreValues)
    {
        scoreList.push_back(std::make_pair(s.getValue<std::string>(), s.getName()));
    }
    std::sort(std::begin(scoreList), std::end(scoreList), [](const Score& scoreA, const Score& scoreB)
    {
        try
        {
            //int conversion may fail :(
            return(std::stoi(scoreA.second) > std::stoi(scoreB.second));
        }
        catch (...)
        {
            return false;
        }
    });

    std::string scoreString;
    for (auto i = 0u; i < scoreList.size(); ++i)
    {
        scoreString += std::to_string(i + 1) + " " + scoreList[i].first + " " + scoreList[i].second + "\n";
    }

    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Transform>().setParent(controlEntity);
    entity.addComponent<cro::Text>(scoreboardFont).setString(scoreString);
    entity.getComponent<cro::Text>().setCharSize(TextLarge);
    entity.getComponent<cro::Text>().setColour(textColourSelected);
    entity.getComponent<cro::Text>().setAlignment(cro::Text::Alignment::Centre);

    auto bounds = entity.getComponent<cro::Text>().getLocalBounds();
    entity.getComponent<cro::Transform>().setOrigin({ bounds.width / 2.f, 0.f, 0.f });

    size = backgroundEnt.getComponent<cro::Sprite>().getSize();
    cro::FloatRect croppingArea(0.f, 0.f, size.x * 0.8f, -(size.y - backgroundEnt.getComponent<cro::Transform>().getPosition().y - 36.f)); //remember text origin is at top
    entity.getComponent<cro::Text>().setCroppingArea(croppingArea);

    //add click /drag
    const auto& scroll = [](cro::Entity entity, float delta)->float
    {
        auto& text = entity.getComponent<cro::Text>();
        auto crop = text.getCroppingArea();

        //clamp movement
        float movement = 0.f;
        if (delta > 0)
        {
            movement = cro::Util::Maths::clamp((text.getLocalBounds().height + crop.height) - entity.getComponent<cro::Transform>().getPosition().y, 0.f, delta);
        }
        else
        {
            movement = std::max(-entity.getComponent<cro::Transform>().getPosition().y, delta);
        }
        entity.getComponent<cro::Transform>().move({ 0.f, movement, 0.f });

        //update the cropping area
        crop.bottom -= movement;
        text.setCroppingArea(crop);

        //update the input area
        entity.getComponent<cro::UIInput>().area.bottom -= movement;

        return movement;
    };


    entity.addComponent<cro::UIDraggable>();
    entity.addComponent<cro::UIInput>().callbacks[cro::UIInput::MouseMotion] = m_uiSystem->addCallback(
        [scroll](cro::Entity entity, glm::vec2 delta)
    {
        if (entity.getComponent<cro::UIDraggable>().flags & cro::UISystem::LeftMouse)
        {
            //add some momentum
            entity.getComponent<cro::UIDraggable>().velocity.y += scroll(entity, delta.y);
            entity.getComponent<cro::Callback>().active = true;
        }
    });
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseDown] = m_uiSystem->addCallback(
        [](cro::Entity entity, cro::uint64 flags)
    {
        entity.getComponent<cro::UIDraggable>().flags |= flags;
    });
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseUp] = m_uiSystem->addCallback(
        [](cro::Entity entity, cro::uint64 flags)
    {
        entity.getComponent<cro::UIDraggable>().flags &= ~flags;
    });
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseExit] = m_uiSystem->addCallback(
        [](cro::Entity entity, glm::vec2)
    {
        entity.getComponent<cro::UIDraggable>().flags = 0;
    });
    entity.getComponent<cro::UIInput>().area = croppingArea;

    //callback gives scrolling some momentum
    entity.addComponent<cro::Callback>().function = 
        [scroll](cro::Entity entity, cro::Time dt)
    {
        auto& drag = entity.getComponent<cro::UIDraggable>();

        scroll(entity, drag.velocity.y * dt.asSeconds());
        drag.velocity *= 0.993f;

        if (glm::length2(drag.velocity) < 0.1f)
        {
            entity.getComponent<cro::Callback>().active = false;
        }
    };
    auto scoreEnt = entity;

    //scroll arrows
    auto activeArrow = spriteSheetButtons.getSprite("arrow_active").getTextureRect();
    auto inactiveArrow = spriteSheetButtons.getSprite("arrow_inactive").getTextureRect();

    auto arrowEnter = m_uiSystem->addCallback(
        [activeArrow](cro::Entity e, glm::vec2)
    {
        e.getComponent<cro::Sprite>().setTextureRect(activeArrow);
    });
    auto arrowExit = m_uiSystem->addCallback(
        [inactiveArrow](cro::Entity e, glm::vec2)
    {
        e.getComponent<cro::Sprite>().setTextureRect(inactiveArrow);
        e.getComponent<cro::Callback>().active = false;
    });

    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = spriteSheetButtons.getSprite("arrow_inactive");
    size = entity.getComponent<cro::Sprite>().getSize();
    entity.addComponent<cro::Transform>().setOrigin({ size.x / 2.f, size.y / 2.f, 0.f });
    entity.getComponent<cro::Transform>().setParent(controlEntity);
    entity.getComponent<cro::Transform>().setPosition({ (croppingArea.width / 2.f) + (size.x / 2.f) /** 0.89f*/, 60.f, 0.f });

    entity.addComponent<cro::UIInput>().callbacks[cro::UIInput::MouseEnter] = arrowEnter;
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseExit] = arrowExit;
    entity.getComponent<cro::UIInput>().area.width = activeArrow.width;
    entity.getComponent<cro::UIInput>().area.height = activeArrow.height;

    entity.addComponent<cro::Callback>().function = [scroll, scoreEnt](cro::Entity entity, cro::Time dt)
    {
        scroll(scoreEnt, -scrollSpeed * dt.asSeconds());
    };
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseDown] = m_uiSystem->addCallback(
        [](cro::Entity entity, cro::uint64 flags)
    {
        if (flags & cro::UISystem::LeftMouse)
        {
            entity.getComponent<cro::Callback>().active = true;
        }
    });
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseUp] = m_uiSystem->addCallback(
        [scoreEnt](cro::Entity entity, cro::uint64 flags) mutable
    {
        if (flags & cro::UISystem::LeftMouse)
        {
            entity.getComponent<cro::Callback>().active = false;
            scoreEnt.getComponent<cro::UIDraggable>().velocity.y = -scrollSpeed / 2.f;
            scoreEnt.getComponent<cro::Callback>().active = true;
        }
    });
    auto upArrow = entity;



    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>() = upArrow.getComponent<cro::Sprite>();
    entity.addComponent<cro::Transform>().setParent(controlEntity);
    entity.getComponent<cro::Transform>().setOrigin(upArrow.getComponent<cro::Transform>().getOrigin());
    entity.getComponent<cro::Transform>().setPosition(upArrow.getComponent<cro::Transform>().getPosition());
    entity.getComponent<cro::Transform>().rotate({ 0.f, 0.f, 1.f }, cro::Util::Const::PI);
    entity.getComponent<cro::Transform>().move({ 0.f, croppingArea.height, 0.f });

    entity.addComponent<cro::UIInput>().callbacks[cro::UIInput::MouseEnter] = arrowEnter;
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseExit] = arrowExit;
    entity.getComponent<cro::UIInput>().area.width = activeArrow.width;
    entity.getComponent<cro::UIInput>().area.height = activeArrow.height;

    entity.addComponent<cro::Callback>().function = [scroll, scoreEnt](cro::Entity entity, cro::Time dt)
    {
        scroll(scoreEnt, scrollSpeed * dt.asSeconds());
    };
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseDown] = m_uiSystem->addCallback(
        [](cro::Entity entity, cro::uint64 flags)
    {
        if (flags & cro::UISystem::LeftMouse)
        {
            entity.getComponent<cro::Callback>().active = true;
        }
    });
    entity.getComponent<cro::UIInput>().callbacks[cro::UIInput::MouseUp] = m_uiSystem->addCallback(
        [scoreEnt](cro::Entity entity, cro::uint64 flags) mutable
    {
        if (flags & cro::UISystem::LeftMouse)
        {
            entity.getComponent<cro::Callback>().active = false;
            scoreEnt.getComponent<cro::UIDraggable>().velocity.y = scrollSpeed / 2.f;
            scoreEnt.getComponent<cro::Callback>().active = true;
        }
    });

    //TODO if shared resources contains a player name/score, scroll to it
}