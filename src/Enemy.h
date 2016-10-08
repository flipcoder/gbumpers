#ifndef ENEMY_H_0TOM1I2B
#define ENEMY_H_0TOM1I2B

#include "Qor/Mesh.h"
#include <boost/circular_buffer.hpp>

class Game;

class Enemy:
    public Mesh
{
    public:

        Enemy(std::string fn, Game* game, Cache<Resource, std::string>* cache);
        virtual ~Enemy() {}

        virtual void logic_self(Freq::Time t) override;

        Enemy(const Enemy&) = default;
        Enemy(Enemy&&) = default;
        Enemy& operator=(const Enemy&) = default;
        Enemy& operator=(Enemy&&) = default;

    private:

        Game* m_pGame;
        Node* m_pTarget = nullptr;
        boost::circular_buffer<Node*> m_History;
};

#endif
