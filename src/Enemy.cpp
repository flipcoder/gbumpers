#include "Enemy.h"
#include "Game.h"
#include <glm/gtx/vector_angle.hpp>
using namespace std;
using namespace glm;

Enemy :: Enemy(string fn, Game* game, Cache<Resource, string>* cache):
    Mesh(),
    m_pGame(game),
    m_pMesh(make_shared<Mesh>(cache->transform("bumpership.obj"), cache))
{
    Node::filename(cache->transform(fn));
    m_History.set_capacity(4);
    add(m_pMesh);
}

void Enemy :: logic_self(Freq::Time t)
{
    Node::logic_self(t);

    vec3 tpos;
    if(m_pTarget)
        tpos = m_pTarget->position();
    auto pos = position();
    tpos.y = pos.y = 0.0f;
    auto posdelta = tpos - pos;

    if(m_pTarget == nullptr ||
        glm::length(posdelta) < 1.0f
    ){
        if(m_pTarget && not in_history(m_pTarget))
            m_History.push_back(m_pTarget);
        
        auto nav = m_pGame->nav();
        
        // sort nav based on dist from this
        auto _this = this;
        std::sort(ENTIRE(nav), [_this,pos](Node* a, Node* b){
            auto apos = a->position();
            auto bpos = b->position();
            apos.y = bpos.y = 0.0f;
            return glm::length(apos - pos) <
                glm::length(bpos - pos);
        });
        
        // set target to first nav element not in history
        m_pTarget = closest_not_in_history(nav);
    }
    
    // set appropriate rotation/velocity for target
    if(m_pTarget)
    {
        velocity(3.0f * glm::normalize(posdelta));
        //auto heading = Matrix::heading(*m_pMesh->matrix());
        //auto ang = glm::angle(posdelta.x,posdelta.z)/K_TAU;
        //auto head = glm::angle(heading.x,heading.z)/K_TAU;
        //head -= 0.25f;
        //LOGf("ang %s", ang);
        //LOGf("head %s", head);
        ////if(not floatcmp(fmod(ang,1.0f), fmod(head,1.0f)))
        //auto f = (ang - head);
        //LOGf("f %s", f);
        //m_pMesh->rotate(f, Axis::Y);
    }
}

bool Enemy :: in_history(Node* node)
{
    for(auto&& t: m_History)
    {
        if(t == node)
            return true;
    }
    return false;
}

Node* Enemy :: closest_not_in_history(std::vector<Node*>& nav)
{
    for(auto&& n: nav)
    {
        if(not in_history(n))
            return n;
    }
    assert(false);
}
    
