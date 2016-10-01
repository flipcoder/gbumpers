#include "Game.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Light.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

Game :: Game(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline())
{}

void Game :: preload()
{
    auto sw = m_pQor->window()->size().x;
    auto sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);

    auto light = make_shared<Light>();
    light->dist(10000.0f);
    //light->specular(Color::white());
    light->position(glm::vec3(sw/2.0f, sh/2.0f, 10.0f));
    m_pRoot->add(light);
}

Game :: ~Game()
{
    m_pPipeline->partitioner()->clear();
}

void Game :: enter()
{
    auto sw = m_pQor->window()->size().x;
    auto sh = m_pQor->window()->size().y;
    
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color("0d190d"));
    m_pInput->relative_mouse(false);
    
    auto mat = make_shared<MeshMaterial>("bg.png", m_pQor->resources());
    auto mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(sw, sh))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
        }, mat
    );
    m_pRoot->add(mesh);
}

void Game :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);
}

void Game :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get()/*, nullptr, Pipeline::LIGHTS*/);
}

