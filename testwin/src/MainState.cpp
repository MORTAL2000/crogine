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
#include "RotateSystem.hpp"
#include "DriftSystem.hpp"
#include "Slider.hpp"
#include "MyApp.hpp"

#include <crogine/core/App.hpp>
#include <crogine/core/Clock.hpp>
#include <crogine/detail/GlobalConsts.hpp>
#include <crogine/ecs/components/Model.hpp>
#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/Camera.hpp>
#include <crogine/ecs/components/Text.hpp>
#include <crogine/ecs/components/Sprite.hpp>
#include <crogine/ecs/components/AudioSource.hpp>
#include <crogine/ecs/components/AudioListener.hpp>
#include <crogine/ecs/components/CommandID.hpp>
#include <crogine/ecs/components/ShadowCaster.hpp>

#include <crogine/ecs/systems/ModelRenderer.hpp>
#include <crogine/ecs/systems/SceneGraph.hpp>
#include <crogine/ecs/systems/UISystem.hpp>
#include <crogine/ecs/systems/SpriteRenderer.hpp>
#include <crogine/ecs/systems/TextRenderer.hpp>
#include <crogine/ecs/systems/CommandSystem.hpp>
#include <crogine/ecs/systems/AudioSystem.hpp>
#include <crogine/ecs/systems/ShadowMapRenderer.hpp>

#include <crogine/graphics/SphereBuilder.hpp>
#include <crogine/graphics/QuadBuilder.hpp>
#include <crogine/graphics/StaticMeshBuilder.hpp>

#include <crogine/graphics/SpriteSheet.hpp>
#include <crogine/graphics/Image.hpp>
#include <crogine/graphics/postprocess/PostChromeAB.hpp>

#include <crogine/util/Constants.hpp>
#include <crogine/gui/Gui.hpp>
#include <crogine/audio/AudioMixer.hpp>

#include <iomanip>
namespace
{
#include "MenuConsts.inl"

    void outputIcon(const cro::Image& img)
    {
        CRO_ASSERT(img.getFormat() == cro::ImageFormat::RGBA, "");
        std::stringstream ss;
        ss<< "static const unsigned char icon[] = {" << std::endl;
        ss << std::showbase << std::internal << std::setfill('0');

        auto i = 0;
        for (auto y = 0; y < 16; ++y)
        {
            for (auto x = 0; x < 16; ++x)
            {
                ss << std::hex << (int)img.getPixelData()[i++] << ", ";
                ss << std::hex << (int)img.getPixelData()[i++] << ", ";
                ss << std::hex << (int)img.getPixelData()[i++] << ", ";
                ss << std::hex << (int)img.getPixelData()[i++] << ", ";
            }
            ss << std::endl;
        }

        ss << "};";

        std::ofstream file("icon.hpp");
        file << ss.rdbuf();
        file.close();
    }

    //cro::CommandSystem* buns = nullptr;
}

MainState::MainState(cro::StateStack& stack, cro::State::Context context, ResourcePtr& sharedResources)
    : cro::State        (stack, context),
    m_backgroundScene   (context.appInstance.getMessageBus()),
    m_menuScene         (context.appInstance.getMessageBus()),
    m_sharedResources   (*sharedResources),
    m_commandSystem     (nullptr),
    m_uiSystem          (nullptr)
{
    /*registerStatusControls(
        []()
    {
        static float value;
        value = cro::AudioMixer::getMasterVolume();
        cro::Nim::slider("Master Volume", value, 0.f, 5.f);
        cro::AudioMixer::setMasterVolume(value);

        value = cro::AudioMixer::getVolume(0);
        cro::Nim::slider("Channel 0", value, 0.f, 5.f);
        cro::AudioMixer::setVolume(value, 0);
    });*/
    
    context.mainWindow.loadResources([this, &context]()
    {
        addSystems();
        loadAssets();
        createScene();
        createMenus();
    });

    updateView();

    //context.appInstance.setClearColour(cro::Colour::Red());
}

//public
bool MainState::handleEvent(const cro::Event& evt)
{
    m_uiSystem->handleEvent(evt);
    return true;
}

void MainState::handleMessage(const cro::Message& msg)
{
    m_backgroundScene.forwardMessage(msg);
    m_menuScene.forwardMessage(msg);

    if (msg.id == cro::Message::WindowMessage)
    {
        const auto& data = msg.getData<cro::Message::WindowEvent>();
        if (data.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            updateView();
        }
    }
}

bool MainState::simulate(cro::Time dt)
{    
    m_backgroundScene.simulate(dt);
    m_menuScene.simulate(dt);

    //auto size = getContext().mainWindow.getSize();
    //DPRINT("window size", std::to_string(size.x) + ", " + std::to_string(size.y));

    return true;
}

void MainState::render()
{    
    m_backgroundScene.render();
    m_menuScene.render();
}

//private
void MainState::addSystems()
{
    auto& mb = getContext().appInstance.getMessageBus();
    //buns = &m_backgroundScene.addSystem<cro::CommandSystem>(mb);
    m_backgroundScene.addSystem<RotateSystem>(mb);
    m_backgroundScene.addSystem<DriftSystem>(mb);
    m_backgroundScene.addSystem<cro::AudioSystem>(mb);
    m_backgroundScene.addSystem<cro::SceneGraph>(mb);
    m_backgroundScene.addSystem<cro::ShadowMapRenderer>(mb);
    m_backgroundScene.addSystem<cro::ModelRenderer>(mb);

#ifdef PLATFORM_DESKTOP
    m_backgroundScene.addPostProcess<cro::PostChromeAB>();
#endif
    
    m_commandSystem = &m_menuScene.addSystem<cro::CommandSystem>(mb);
    //m_menuScene.addSystem<cro::DebugInfo>(mb);
    m_menuScene.addSystem<SliderSystem>(mb);
    //m_menuScene.addSystem<RotateSystem>(mb);
    m_menuScene.addSystem<cro::SceneGraph>(mb);
    m_menuScene.addSystem<cro::SpriteRenderer>(mb);
    m_menuScene.addSystem<cro::TextRenderer>(mb);
    m_uiSystem = &m_menuScene.addSystem<cro::UISystem>(mb);
}

void MainState::loadAssets()
{
    m_modelDefs[MenuModelID::LookoutBase].loadFromFile("assets/models/lookout_base.cmt", m_resources);
    m_modelDefs[MenuModelID::ArcticPost].loadFromFile("assets/models/arctic_outpost.cmt", m_resources);
    m_modelDefs[MenuModelID::GasPlanet].loadFromFile("assets/models/planet.cmt", m_resources);
    m_modelDefs[MenuModelID::Moon].loadFromFile("assets/models/moon.cmt", m_resources);
    m_modelDefs[MenuModelID::Roids].loadFromFile("assets/models/roid_belt.cmt", m_resources);
    m_modelDefs[MenuModelID::Stars].loadFromFile("assets/models/stars.cmt", m_resources);
    m_modelDefs[MenuModelID::Sun].loadFromFile("assets/models/sun.cmt", m_resources);

    //sprite sheet
    auto& menuFont = m_sharedResources.fonts.get(FontID::MenuFont);
    menuFont.loadFromFile("assets/fonts/Audiowide-Regular.ttf");

    //audio
    m_resources.audio.load(AudioID::Test, "assets/audio/boop_loop.ogg");
    m_resources.audio.load(AudioID::TestStream, "assets/audio/game.ogg", true);
}

void MainState::createScene()
{
    //-----background-----//
    //create planet / moon
    auto entity = m_backgroundScene.createEntity();
    entity.addComponent<cro::Transform>().setPosition({ 4.f, -0.7f, -8.f });
    entity.getComponent<cro::Transform>().setRotation({ -0.5f, 0.f, 0.4f });
    //entity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::GasPlanet].meshID),
    //                                m_resources.materials.get(m_modelDefs[MenuModelID::GasPlanet].materialIDs[0]));
    m_modelDefs[MenuModelID::GasPlanet].createModel(entity, m_resources);
    auto& planetRotator = entity.addComponent<Rotator>();
    planetRotator.speed = 0.02f;
    planetRotator.axis.y = 0.2f;

    auto moonAxis = m_backgroundScene.createEntity();
    auto& moonAxisTx = moonAxis.addComponent<cro::Transform>();
    moonAxisTx.setOrigin({ 0.f, 0.f, -5.6f });
    moonAxisTx.setParent(entity);
    
    auto moonEntity = m_backgroundScene.createEntity();
    auto& moonTx = moonEntity.addComponent<cro::Transform>();
    moonTx.setScale(glm::vec3(0.34f));
    //moonTx.setOrigin({ 11.f, 0.f, 0.f });
    moonTx.setParent(moonAxis);
    moonEntity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::Moon].meshID),
                                        m_resources.materials.get(m_modelDefs[MenuModelID::Moon].materialIDs[0]));
    moonEntity.getComponent<cro::Model>().setShadowMaterial(0, m_resources.materials.get(m_modelDefs[MenuModelID::Moon].shadowIDs[0]));
    moonEntity.addComponent<cro::ShadowCaster>();
    //m_modelDefs[MenuModelID::Moon].createModel(moonEntity, m_resources);
    auto& moonRotator = moonEntity.addComponent<Rotator>();
    moonRotator.axis.y = 1.f;
    moonRotator.speed = 0.1f;   

    auto arcticEntity = m_backgroundScene.createEntity();
    auto& arcticTx = arcticEntity.addComponent<cro::Transform>();
    arcticTx.setScale(glm::vec3(0.8f));
    arcticTx.setOrigin({ 5.8f, 0.f, 5.f });
    arcticTx.setParent(entity);
    auto& arcticModel = arcticEntity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::ArcticPost].meshID),
                                                                m_resources.materials.get(m_modelDefs[MenuModelID::ArcticPost].materialIDs[0]));
    for (auto i = 0u; i < m_modelDefs[MenuModelID::ArcticPost].materialCount; ++i)
    {
        arcticModel.setMaterial(i, m_resources.materials.get(m_modelDefs[MenuModelID::ArcticPost].materialIDs[i]));
        arcticModel.setShadowMaterial(i, m_resources.materials.get(m_modelDefs[MenuModelID::ArcticPost].shadowIDs[i]));
    }
    arcticEntity.addComponent<cro::ShadowCaster>();
    //m_modelDefs[MenuModelID::ArcticPost].createModel(arcticEntity, m_resources);

    arcticEntity.addComponent<cro::AudioSource>(m_resources.audio.get(AudioID::TestStream)).play(true);
    //arcticEntity.getComponent<cro::AudioSource>().setRolloff(20.f);
    arcticEntity.getComponent<cro::AudioSource>().setVolume(0.1f);
    arcticEntity.addComponent<cro::CommandTarget>().ID = (1 << 30);
    
    
    auto lookoutEntity = m_backgroundScene.createEntity();
    auto& lookoutTx = lookoutEntity.addComponent<cro::Transform>();
    lookoutTx.setScale(glm::vec3(0.7f));
    lookoutTx.setOrigin({ -8.f, 0.f, 2.f });
    lookoutTx.setParent(entity);
    /*auto& lookoutModel = lookoutEntity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::LookoutBase].meshID),
                                                                m_resources.materials.get(m_modelDefs[MenuModelID::LookoutBase].materialIDs[0]));
    for (auto i = 0u; i < m_modelDefs[MenuModelID::LookoutBase].materialCount; ++i)
    {
        lookoutModel.setMaterial(i, m_resources.materials.get(m_modelDefs[MenuModelID::LookoutBase].materialIDs[i]));
        lookoutModel.setShadowMaterial(i, m_resources.materials.get(m_modelDefs[MenuModelID::LookoutBase].shadowIDs[i]));
    }
    lookoutEntity.addComponent<cro::ShadowCaster>();*/
    m_modelDefs[MenuModelID::LookoutBase].createModel(lookoutEntity, m_resources);
    lookoutEntity.addComponent<cro::AudioSource>(m_resources.audio.get(AudioID::Test)).play(true);
    lookoutEntity.getComponent<cro::AudioSource>().setPitch(2.4f);
    //lookoutEntity.getComponent<cro::AudioSource>().setRolloff(20.f);
    //lookoutEntity.getComponent<cro::AudioSource>().setVolume(2.4f);
    

    auto roidEntity = m_backgroundScene.createEntity();  
    roidEntity.addComponent<cro::Transform>().setScale({ 0.7f, 0.7f, 0.7f });
    roidEntity.getComponent<cro::Transform>().setParent(entity);
    roidEntity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::Roids].meshID),
                                        m_resources.materials.get(m_modelDefs[MenuModelID::Roids].materialIDs[0]));
    auto& roidRotator = roidEntity.addComponent<Rotator>();
    roidRotator.speed = -0.03f;
    roidRotator.axis.y = 1.f;

    /*auto cloudEntity = m_backgroundScene.createEntity();
    cloudEntity.addComponent<cro::Transform>().setScale({ 1.01f, 1.01f, 1.01f });
    cloudEntity.getComponent<cro::Transform>().setParent(entity);
    cloudEntity.addComponent<cro::Model>(m_meshResource.getMesh(cro::Mesh::SphereMesh), m_materialResource.get(MaterialID::PlanetClouds));
*/
    //create stars / sun
    entity = m_backgroundScene.createEntity();
    entity.addComponent<cro::Transform>().setPosition({ 0.f, 0.f, -39.f });
    entity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::Stars].meshID),
                                    m_resources.materials.get(m_modelDefs[MenuModelID::Stars].materialIDs[0]));

    entity = m_backgroundScene.createEntity();
    entity.addComponent<cro::Transform>().setPosition({ 0.f, 0.f, -28.f });
    entity.getComponent<cro::Transform>().rotate({ 0.f, 0.f, 1.f }, 3.14f);
    entity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::Stars].meshID),
                                    m_resources.materials.get(m_modelDefs[MenuModelID::Stars].materialIDs[0]));
    entity.addComponent<Drifter>().amplitude = -0.1f;

    entity = m_backgroundScene.createEntity();
    entity.addComponent<cro::Transform>().setPosition({ -4.f, 2.6f, -11.9f });
    entity.addComponent<cro::Model>(m_resources.meshes.getMesh(m_modelDefs[MenuModelID::Sun].meshID),
                                    m_resources.materials.get(m_modelDefs[MenuModelID::Sun].materialIDs[0]));

    //set up lighting
    m_backgroundScene.getSunlight().setDirection({ 0.1f, -0.8f, -0.2f });
    //m_backgroundScene.getSunlight().setColour(cro::Colour(0.48f, 0.48f, 0.48f));
    m_backgroundScene.getSunlight().setProjectionMatrix(glm::ortho(-6.f, 6.f, -6.f, 6.f, 0.1f, 20.f));
    m_backgroundScene.getSystem<cro::ShadowMapRenderer>().setProjectionOffset({ -0.1f, 1.6f, 0.2f });


    //2D and 3D cameras
    entity = m_backgroundScene.createEntity();
    entity.addComponent<cro::Transform>();// .setPosition({ 0.f, 0.f, 4.f });
    entity.addComponent<cro::Camera>();
    entity.addComponent<cro::AudioListener>();

    /*auto &camRot = entity.addComponent<Rotator>();
    camRot.axis.y = 1.f; camRot.speed = 0.2f;*/

    entity.addComponent<Drifter>().amplitude = 0.1f;
    m_backgroundScene.setActiveCamera(entity);
    m_backgroundScene.setActiveListener(entity);

    //used for menu scene
    entity = m_menuScene.createEntity();
    entity.addComponent<cro::Transform>();
    auto& cam2D = entity.addComponent<cro::Camera>();
    cam2D.projection = glm::ortho(0.f, static_cast<float>(cro::DefaultSceneSize.x), 0.f, static_cast<float>(cro::DefaultSceneSize.y), -2.f, 100.f);
    m_menuScene.setActiveCamera(entity);
}

void MainState::createMenus()
{
    cro::SpriteSheet spriteSheetButtons;
    spriteSheetButtons.loadFromFile("assets/sprites/ui_menu.spt", m_sharedResources.textures);
    const auto buttonNormalArea = spriteSheetButtons.getSprite("button_inactive").getTextureRect();
    const auto buttonHighlightArea = spriteSheetButtons.getSprite("button_active").getTextureRect();

    auto mouseEnterCallback = m_uiSystem->addCallback([&, buttonHighlightArea](cro::Entity e, cro::uint64)
    {
        e.getComponent<cro::Sprite>().setTextureRect(buttonHighlightArea);
        const auto& children = e.getComponent<cro::Transform>().getChildIDs();
        std::size_t i = 0;
        while (children[i] != -1)
        {
            auto c = children[i++];
            auto child = m_menuScene.getEntity(c);
            if (child.hasComponent<cro::Text>())
            {
                child.getComponent<cro::Text>().setColour(textColourSelected);
            }
            else if (child.hasComponent<cro::Sprite>())
            {
                child.getComponent<cro::Sprite>().setColour(textColourSelected);
            }
        }
    });
    auto mouseExitCallback = m_uiSystem->addCallback([&, buttonNormalArea](cro::Entity e, cro::uint64)
    {
        e.getComponent<cro::Sprite>().setTextureRect(buttonNormalArea);
        const auto& children = e.getComponent<cro::Transform>().getChildIDs();
        std::size_t i = 0;
        while (children[i] != -1)
        {
            auto c = children[i++];
            auto child = m_menuScene.getEntity(c);
            if (child.hasComponent<cro::Text>())
            {
                child.getComponent<cro::Text>().setColour(textColourNormal);
            }
            else if (child.hasComponent<cro::Sprite>())
            {
                child.getComponent<cro::Sprite>().setColour(textColourNormal);
            }
        }
    });

    createMainMenu(mouseEnterCallback, mouseExitCallback);
    createOptionsMenu(mouseEnterCallback, mouseExitCallback);
    createScoreMenu(mouseEnterCallback, mouseExitCallback);

    //preview shadow map
    auto entity = m_menuScene.createEntity();
    entity.addComponent<cro::Sprite>().setTexture(m_backgroundScene.getSystem<cro::ShadowMapRenderer>().getDepthMapTexture());
    entity.addComponent<cro::Transform>().setPosition({ 10.f, 10.f, 0.f });
    entity.getComponent<cro::Transform>().setScale(glm::vec3(0.5f));
}

void MainState::updateView()
{
    glm::vec2 size(cro::App::getWindow().getSize());
    size.y = ((size.x / 16.f) * 9.f) / size.y;
    size.x = 1.f;

    //cro::Logger::log("resized to: " + std::to_string(size.x) + ", " + std::to_string(size.y));

    auto& cam3D = m_backgroundScene.getActiveCamera().getComponent<cro::Camera>();
    cam3D.projection = glm::perspective(0.6f, 16.f / 9.f, 0.1f, 100.f);
    cam3D.viewport.bottom = (1.f - size.y) / 2.f;
    cam3D.viewport.height = size.y;

    auto& cam2D = m_menuScene.getActiveCamera().getComponent<cro::Camera>();
    cam2D.viewport = cam3D.viewport;
}