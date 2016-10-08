#include "Enemy.h"
#include "Game.h"
using namespace std;

Enemy :: Enemy(string fn, Game* game, Cache<Resource, string>* cache):
    Mesh(cache->transform("bumpership.obj"), cache),
    m_pGame(game)
{
    Node::filename(cache->transform(fn));
    m_History = boost::circular_buffer<Node*>(3);
}

void Enemy :: logic_self(Freq::Time t)
{
    Node::logic_self(t);

    if(m_pTarget == nullptr ||
        glm::length(m_pTarget->position() - position()) < 1.0f
    ){
        auto nav = m_pGame->nav();
        
        // sort nav based on dist from this
        
        // set target to first nav element not in history

        // set appropriate rotation/velocity for target
    }
}

