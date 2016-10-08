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
    m_pController(engine->session()->active_profile(0)->controller()),
    m_pRoot(make_shared<Node>()),
    m_pScrRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_Disable(engine->timer()->timeline()),
    m_Shield(engine->timer()->timeline())
{}

void Game :: preload()
{
    auto sw = m_pQor->window()->size().x;
    auto sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pCamera->fov(54.0f);
    m_pScrCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pScrRoot->add(m_pScrCamera);
    
    m_Shader = m_pPipeline->load_shaders({"green"});
    
    m_pMusic = m_pQor->make<Sound>("chip.ogg");
    m_pRoot->add(m_pMusic);

    m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
    
    //m_pScene = m_pQor->make<Scene>("1.json");
    //m_pRoot->add(m_pScene->root());
    //auto meshes = m_pScene->root()->find_type<Mesh>();
    //for(auto&& mesh: meshes)
    //    mesh->set_physics(Node::STATIC);
    
    m_pPlayer = m_pQor->make<Mesh>("bumpership.obj");
    m_pPlayer->visible(false);
    auto sz = 1.0f;
    auto height = 0.5f;
    m_pPlayer->set_box(Box(
        glm::vec3(-sz, -height, -sz),
        glm::vec3(sz, height, sz)
    ));
    m_pPlayer->set_physics(Node::DYNAMIC);
    m_pPlayer->set_physics_shape(Node::CYLINDER);
    m_pPlayer->mass(1.0f);
    m_pPlayer->inertia(false);
    m_pPlayer->add(m_pCamera);
    //m_pCamera->position(glm::vec3(0.0f, -1.0f, 0.0f));
    m_pRoot->add(m_pPlayer);
    m_pPipeline->partitioner()->register_object(m_pPlayer, PLAYER);
    auto _this = this;
    m_pPipeline->partitioner()->on_collision(PLAYER, PLAYER_FLAG, [_this](Node* a, Node* b){
        if(b->visible()){
            LOG("flag");
            Sound::play(_this->m_pCamera.get(), "flag.wav", _this->m_pQor->resources());
            ++_this->m_Flags;
            if(_this->m_Flags == _this->m_MaxFlags)
                _this->next_map();

            b->detach();
            b->visible(false);
        }
    });

    
    auto mesh = m_pQor->make<Mesh>(
        string("level") + m_pQor->args().value_or("map","1") + ".obj"
    );
    m_pRoot->add(mesh);
    
    auto meshes = mesh->find_type<Mesh>();
    for(auto&& mesh: meshes){
        glm::vec3 pmin(mesh->geometry()->verts()[0]);
        for(auto&& v: mesh->geometry()->verts())
        {
            if(v.x < pmin.x)
                pmin.x = v.x;
            if(v.y < pmin.y)
                pmin.y = v.y;
            if(v.z < pmin.z)
                pmin.z = v.z;
        }
        if(mesh->material()->texture()->filename().find("flag") != string::npos)
        {
            auto flag = m_pQor->make<Mesh>("flag.obj");//->instance();
            flag->position(pmin);
            m_pRoot->add(flag);
            mesh->detach();
            m_FlagSpawns.push_back(flag.get());
        }
        else if(mesh->material()->texture()->filename().find("e_spawn") != string::npos)
        {
            auto e = m_pQor->make<Mesh>("bumpership.obj");
            e->set_box(Box(
                glm::vec3(-sz, -height, -sz),
                glm::vec3(sz, height, sz)
            ));
            e->position(glm::vec3(0.0f, 0.25f, -2.0f));
            e->set_physics(Node::DYNAMIC);
            e->set_physics_shape(Node::CYLINDER);
            e->mass(1.0f);
            e->inertia(false);
            m_pRoot->add(e);
            e->position(pmin);
            mesh->visible(false);
            m_Enemies.push_back(e.get());
        }
        else{
            m_StaticMeshes.push_back(mesh);
            mesh->set_physics(Node::STATIC);
        }
    }
    std::random_shuffle(ENTIRE(m_FlagSpawns));
    m_MaxFlags = m_FlagSpawns.size()/2; // half friendly, half enemy
    for(int i=0; i<m_MaxFlags; ++i) {
        m_pPipeline->partitioner()->register_object(m_FlagSpawns[i]->as_node(), PLAYER_FLAG);
    }
    for(int i=m_MaxFlags; i<m_MaxFlags*2; ++i) {
        auto children = m_FlagSpawns[i]->children();
        for(int j=0; j<children.size(); ++j){
            ((Mesh*)children[j].get())->material("data/e_flag.png", m_pQor->resources());
        }
    }
    
    m_pPhysics->generate(m_pRoot.get(), Physics::GEN_RECURSIVE);
    m_pPhysics->world()->setGravity(btVector3(0.0, -9.8, 0.0));

    for(auto&& mesh: m_StaticMeshes) {
        if(((Mesh*)mesh)->material()->texture()->filename().find("brick") != string::npos) {
            ((btRigidBody*)mesh->body()->body())->setRestitution(1.0);
        }
    }
    for(auto&& mesh: m_Enemies) {
        btRigidBody* body = (btRigidBody*)((Mesh*)mesh)->body()->body();
        body->setRestitution(1.0);
    }
    
    //auto light = make_shared<Light>();
    //light->dist(10000.0f);
    //m_pRoot->add(light);

    btRigidBody* player_body = (btRigidBody*)m_pPlayer->body()->body();
    player_body->setFriction(0.0);
    player_body->setRestitution(1.0);
    player_body->setCcdMotionThreshold(0.001f);
    player_body->setCcdSweptSphereRadius(0.25f);
    player_body->setActivationState(DISABLE_DEACTIVATION);
    m_pPlayer->position(glm::vec3(0.0f, 1.0f, 0.0f));
}

Game :: ~Game()
{
    m_pPipeline->partitioner()->clear();
}

void Game :: enter()
{
    auto sw = m_pQor->window()->size().x;
    auto sh = m_pQor->window()->size().y;

    m_pCamera->size(ivec2(160,144));
    m_pCamera->perspective();
    m_pCamera->listen();
    //m_pCamera->ortho();
    m_pScrCamera->ortho();
    //m_pPipeline->winding(true);
    m_pRenderBuffer = std::make_shared<RenderBuffer>(160,144);
    //m_pPipeline->bg_color(Color("0d540d"));
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
    
    //auto mat = make_shared<MeshMaterial>("bg.png", m_pQor->resources());
    //auto mesh = make_shared<Mesh>(
    //    make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(160, 144))),
    //    vector<shared_ptr<IMeshModifier>>{
    //        make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
    //    }, mat
    //);
    //m_pRoot->add(mesh);
    
    auto scr = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(sw, sh))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,0.0f), vec2(1.0f,1.0f)))
        }, std::make_shared<MeshMaterial>(m_pRenderBuffer->texture())
    );
    m_pScrRoot->add(scr);

    m_pMusic->play();

    m_Disable.set(Freq::Time::ms(0));
    m_Shield.set(Freq::Time::ms(0));
}

void Game :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pPhysics->logic(t);
    m_pRoot->logic(t);
    
    float accel = 2.0f;
    float turn_speed = 1.0f / 2.0f;
    float max_speed = 3.0f + m_Light*1.0f;
    
    auto p = m_pPlayer->position();
    auto hit = m_pPhysics->first_hit(
        p, p - m_pPlayer->box().size().y - glm::vec3(0.0f, 1.0f, 0.0f)
    );
    Node* hitnode = std::get<0>(hit);
    auto norm = std::get<2>(hit);
    if(norm.y > 0.8 && norm.y < 1.0 - K_EPSILON)
        accel = 5.0f;

    glm::vec3 v = m_pPlayer->velocity();
    if(m_Disable.elapsed()){
        if(m_pController->button("up"))
            v += t.s() * accel * Matrix::heading(*m_pCamera->matrix_c()) *
                m_pController->button("up").pressure();
        if(m_pController->button("down"))
            v -= t.s() * accel * Matrix::heading(*m_pCamera->matrix_c()) *
                m_pController->button("down").pressure();
    }
    if(m_pController->button("left"))
        m_pCamera->rotate(turn_speed * t.s(), Axis::Y);
    if(m_pController->button("right"))
        m_pCamera->rotate(turn_speed * -t.s(), Axis::Y);

    glm::vec3 xz = glm::vec3(v.x, 0.0f, v.z);
    auto y = v.y;
    if(glm::length(xz) >= max_speed){
        v = glm::normalize(xz) * max_speed;
        v.y = y;
    }
    
    //m_pCamera->position(glm::vec3(p.x, p.y, p.z));
    
    m_pPlayer->velocity(v);

    if(m_Shield.elapsed()) {
        if(hitnode)
        {
            auto mesh = ((Mesh*)hitnode);
            string tex = mesh->material()->texture()->filename();
            if(tex.find("ramp") != string::npos)
            {
                if(m_Retrigger){
                    auto geom = mesh->geometry()->verts();
                    auto wrap = mesh->get_modifier<Wrap>()->data();
                    LOG("wrap");
                    for(auto&& w: wrap)
                    {
                        LOG(Vector::to_string(w));
                    }
                }
                m_Retrigger = false;
            }
            else if(tex.find("flagmarker") != string::npos)
            {
                if(m_Retrigger){
                    m_Flags = std::max(0, m_Flags - 1);
                }
                m_Retrigger = false;
            }
            else if(tex.find("target") != string::npos)
            {
                if(m_Retrigger){
                    Sound::play(m_pCamera.get(), "block.wav", m_pQor->resources());
                    m_Disable.set(Freq::Time::seconds(3.0f));
                    m_pPlayer->velocity(glm::vec3(0.0f));
                }
                m_Retrigger = false;
            }
            else // normal ground
            {
                m_Retrigger = true;
            }
        }
    }
    
    //if(m_pInput->key(SDLK_a))
    //    m_pCamera->move(glm::vec3(0.0f, -t.s(), 0.0f));
    //if(m_pInput->key(SDLK_SPACE))
    //    m_pCamera->move(glm::vec3(0.0f, t.s(), 0.0f));

}

void Game :: render() const
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;

    m_pPipeline->winding(false);
    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
    m_pRenderBuffer->push();
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
    RenderBuffer::pop();

    m_pPipeline->winding(true);
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    m_pPipeline->render(m_pScrRoot.get(), m_pScrCamera.get());
}

void Game :: next_map()
{
    auto mapname = m_pQor->args().value_or("map","1");
    auto nextmap = to_string(boost::lexical_cast<int>(mapname) + 1);

    m_pQor->args().set("map", nextmap);
    m_pQor->change_state("game");
}

