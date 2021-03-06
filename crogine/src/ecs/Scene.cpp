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

#include <crogine/ecs/Scene.hpp>
#include <crogine/ecs/components/Camera.hpp>
#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/AudioListener.hpp>
#include <crogine/ecs/Renderable.hpp>

#include <crogine/core/Clock.hpp>
#include <crogine/core/App.hpp>

#include <crogine/util/Constants.hpp>
#include <crogine/detail/glm/gtc/matrix_transform.hpp>

#include "../detail/GLCheck.hpp"

using namespace cro;

namespace
{
    const float DefaultFOV = 35.f * Util::Const::degToRad;

    void updateView(cro::Entity entity)
    {
        glm::vec2 size(cro::App::getWindow().getSize());
        size.y = ((size.x / 16.f) * 9.f) / size.y;
        size.x = 1.f;

        auto& cam3D = entity.getComponent<cro::Camera>();
        cam3D.projectionMatrix = glm::perspective(DefaultFOV, 16.f / 9.f, 0.1f, 280.f);
        cam3D.viewport.bottom = (1.f - size.y) / 2.f;
        cam3D.viewport.height = size.y;
    }
}

Scene::Scene(MessageBus& mb)
    : m_messageBus      (mb),
    m_entityManager     (mb),
    m_systemManager     (*this),
    m_projectionMapCount(0)
{
    auto defaultCamera = createEntity();
    defaultCamera.addComponent<Transform>();
    defaultCamera.addComponent<Camera>();
    defaultCamera.addComponent<AudioListener>();
    updateView(defaultCamera);

    m_defaultCamera = defaultCamera.getIndex();
    m_activeCamera = m_defaultCamera;
    m_activeListener = m_defaultCamera;

    currentRenderPath = [this]()
    {
        auto camera = m_entityManager.getEntity(m_activeCamera);
        for (auto r : m_renderables) r->render(camera);
    };
}

//public
void Scene::simulate(Time dt)
{
    //update directors first as they'll be working on data from the last frame
    for (auto& d : m_directors)
    {
        d->process(dt);
    }

    for (const auto& entity : m_pendingEntities)
    {
        m_systemManager.addToSystems(entity);
    }
    m_pendingEntities.clear();

    for (const auto& entity : m_destroyedEntities)
    {
        m_systemManager.removeFromSystems(entity);
        m_entityManager.destroyEntity(entity);
    }
    m_destroyedEntities.clear();

    m_systemManager.process(dt);
    for (auto& p : m_postEffects) p->process(dt);
}

Entity Scene::createEntity()
{
    m_pendingEntities.push_back(m_entityManager.createEntity());
    return m_pendingEntities.back();
}

void Scene::destroyEntity(Entity entity)
{
    m_destroyedEntities.push_back(entity);
}

Entity Scene::getEntity(Entity::ID id) const
{
    return m_entityManager.getEntity(id);
}

void Scene::setPostEnabled(bool enabled)
{
    if (enabled && !m_postEffects.empty())
    {
        currentRenderPath = std::bind(&Scene::postRenderPath, this);
        auto size = App::getWindow().getSize();
        m_sceneBuffer.create(size.x, size.y, true);
        for (auto& p : m_postEffects) p->resizeBuffer(size.x, size.y);
    }
    else
    {       
        currentRenderPath = [this]()
        {
            auto camera = m_entityManager.getEntity(m_activeCamera);
            for (auto& r : m_renderables) r->render(camera);
        };
    }
}

void Scene::setSunlight(const Sunlight& sunlight)
{
    m_sunlight = sunlight;
}

const Sunlight& Scene::getSunlight() const
{
    return m_sunlight;
}

Sunlight& Scene::getSunlight()
{
    return m_sunlight;
}

Entity Scene::getDefaultCamera() const
{
    return m_entityManager.getEntity(m_defaultCamera);
}

Entity Scene::setActiveCamera(Entity entity)
{
    CRO_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<Camera>(), "Entity requires at least a transform and a camera component");
    CRO_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    auto oldCam = m_entityManager.getEntity(m_activeCamera);
    m_activeCamera = entity.getIndex();

    return oldCam;
}

Entity Scene::setActiveListener(Entity entity)
{
    CRO_ASSERT(entity.hasComponent<Transform>() && entity.hasComponent<AudioListener>(), "Entity requires at least a transform and a camera component");
    CRO_ASSERT(m_entityManager.owns(entity), "This entity must belong to this scene!");
    auto oldListener = m_entityManager.getEntity(m_activeListener);
    m_activeListener = entity.getIndex();
    return oldListener;
}

Entity Scene::getActiveListener() const
{
    return m_entityManager.getEntity(m_activeListener);
}

Entity Scene::getActiveCamera() const
{
    return m_entityManager.getEntity(m_activeCamera);
}

void Scene::forwardEvent(const Event& evt)
{
    for (auto& d : m_directors)
    {
        d->handleEvent(evt);
    }
}

void Scene::forwardMessage(const Message& msg)
{
    if (msg.id == cro::Message::WindowMessage)
    {
        const auto& data = msg.getData<cro::Message::WindowEvent>();
        if (data.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            updateView(getEntity(m_defaultCamera));
        }
    }

    m_systemManager.forwardMessage(msg);
    for (auto& d : m_directors)
    {
        d->handleMessage(msg);
    }

    if (msg.id == Message::WindowMessage)
    {
        const auto& data = msg.getData<Message::WindowEvent>();
        if (data.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            //resizes the post effect buffer if it is in use
            if (m_sceneBuffer.available())
            {
                m_sceneBuffer.create(data.data0, data.data1);
                for (auto& p : m_postEffects) p->resizeBuffer(data.data0, data.data1);
            }

            if (m_postBuffers[0].available())
            {
                m_postBuffers[0].create(data.data0, data.data1, false);
            }

            if (m_postBuffers[1].available())
            {
                m_postBuffers[1].create(data.data0, data.data1, false);
            }
        }
    }
}

void Scene::render()
{
    currentRenderPath();
}

std::pair<const float*, std::size_t> Scene::getActiveProjectionMaps() const
{
    return std::pair<const float*, std::size_t>(&m_projectionMaps[0][0].x, m_projectionMapCount);
}

//private
void Scene::postRenderPath()
{
    auto camera = m_entityManager.getEntity(m_activeCamera);
    
    m_sceneBuffer.clear();
    for (auto r : m_renderables) r->render(camera);
    m_sceneBuffer.display();

    RenderTexture* inTex = &m_sceneBuffer;
    RenderTexture* outTex = nullptr;

    for (auto i = 0u; i < m_postEffects.size() - 1; ++i)
    {
        outTex = &m_postBuffers[i % 2];
        outTex->clear();
        m_postEffects[i]->apply(*inTex);
        outTex->display();
        inTex = outTex;
    }


    auto vp = m_sceneBuffer.getDefaultViewport();
    glViewport(vp.left, vp.bottom, vp.width, vp.height);
    m_postEffects.back()->apply(*inTex);
}