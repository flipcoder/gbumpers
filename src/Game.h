#ifndef _PREGAMESTATE_H
#define _PREGAMESTATE_H

#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/Scene.h"
#include "Qor/RenderBuffer.h"
#include "Qor/Sound.h"

class Qor;

class Game:
    public State
{
    public:
        
        Game(Qor* engine);
        virtual ~Game();

        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

        enum ObjectTypes {
            PLAYER,
            ENEMY,
            PLAYER_FLAG,
            ENEMY_FLAG
        };

    private:
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;
        std::shared_ptr<RenderBuffer> m_pRenderBuffer;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Mesh> m_pPlayer;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Node> m_pScrRoot;
        std::shared_ptr<Camera> m_pScrCamera;

        unsigned m_Shader = (unsigned)PassType::NONE;

        std::shared_ptr<Physics> m_pPhysics;
        std::shared_ptr<Scene> m_pScene;
        
        std::shared_ptr<Sound> m_pMusic;

        int m_Flags = 0;
        int m_MaxFlags = 0;
};

#endif


