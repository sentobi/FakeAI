#ifndef SCENE_AI_H
#define SCENE_AI_H

#include "SceneBase.h"

#include "GridMap.h"
#include "GameObject/GameObject.h"
#include "GameObject/Entity/Machine.h"
#include "GameObject/Entity/Pathfinder.h"

class SceneAI : public SceneBase
{
private:
    bool b_renderDebugInfo;
    int index_renderDebugInfo;
    double d_keypressTimer;

    float brightnessFactor;     // for factory brightness

public:
	SceneAI();
	~SceneAI();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

    void RenderBackground();
	void RenderGO(GameObject *go);

    void RenderMessageBoard();
    void RenderClock();

    void RenderGameObject(GameObject* go);
    //void RenderEntity(Entity* entity);
    void RenderTempRole(Entity* tempRole);
    void RenderMessageNotification(Entity* entity);
    void RenderAcknowledgeNotification(Entity* entity);

    void CheckEntityTempRoleComplete(Entity* entity);

    void RenderDebugInfo();

	//GameObject* FetchGameObject(OBJECT_TYPE ObjectType);
	//void SpawnGameObject(OBJECT_TYPE ObjectType, GAMEOBJECT_TYPE GoType, Vector3 Position, Vector3 Scale, bool Collidable, bool Visible);

    // Machine pointer for debug info
    Machine* debugMachine;
};

#endif