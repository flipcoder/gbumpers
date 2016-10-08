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
            ENEMY_FLAG,
            SPRING,
            GREEN_LIGHT,
            RED_LIGHT,
            SHIELD,
        };
 
    private:

        void next_map();
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        std::shared_ptr<Controller> m_pController;
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

        int m_Springs = 0;
        int m_Light = 0; // -1 red, 1 green

        Freq::Alarm m_Shield;
        Freq::Alarm m_Disable;
        bool m_Retrigger = true;

        std::vector<Node*> m_FlagSpawns;
        std::vector<Node*> m_StaticMeshes;
        std::vector<Node*> m_Enemies;
};

#endif


