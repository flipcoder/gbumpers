#include "Enemy.h"
#include "Game.h"
using namespace std;
using namespace glm;

Enemy :: Enemy(string fn, Game* game, Cache<Resource, string>* cache):
    Mesh(cache->transform("bumpership.obj"), cache),
    m_pGame(game)
{
    Node::filename(cache->transform(fn));
    m_History = boost::circular_buffer<Node*>(2);
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
        if(m_pTarget)
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
        try{
            m_pTarget = nav.at(std::rand() % 2);
        }catch(...){}
    }
    
    // set appropriate rotation/velocity for target
    if(m_pTarget)
    {
        velocity(3.0f * glm::normalize(posdelta));
    }
}

