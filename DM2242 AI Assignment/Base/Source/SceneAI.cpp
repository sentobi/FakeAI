#include "SceneAI.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "SharedData.h"

#include <fstream>
#include <sstream>

#include "GameObject/Entity/Machine.h"
#include "GameObject/Entity/Worker.h"
#include "GameObject/Entity/MaintenanceMan.h"
#include "GameObject/Entity/Robot.h"
#include "GameObject/Entity/DeliveryMan.h"
#include "GameObject/Entity/ScrapMan.h"

SceneAI::SceneAI()
{
}

SceneAI::~SceneAI()
{
}

void SceneAI::Init()
{
    SceneBase::Init();

    Math::InitRNG();

    // Initialise shared variables
    SharedData::GetInstance()->Init();

    //================
    // Create entities
    //================

    // Conveyor Belt
    ConveyorBelt* conveyor = new ConveyorBelt();
    conveyor->Init();
    conveyor->SetPos(Vector3(3, 16, 0));
    conveyor->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CONVEYORBELT));
    SharedData::GetInstance()->m_goList.push_back(conveyor);

    // Machine
    debugMachine = new Machine();
    debugMachine->Init();
    debugMachine->SetPartToCreate(RobotPart::BODY);
    debugMachine->SetPos(Vector3(2.f, 16.f, 0));
    debugMachine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    debugMachine->SetSpawnLocation(conveyor);
    SharedData::GetInstance()->m_goList.push_back(debugMachine);

    //machine = new Machine();
    //machine->SetPartToCreate(RobotPart::HEAD);
    //machine->SetPos(Vector3(10.f, 17.f, 0));
    //machine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    //SharedData::GetInstance()->m_goList.push_back(machine);

    //machine = new Machine();
    //machine->SetPartToCreate(RobotPart::LIMB);
    //machine->SetPos(Vector3(10.f, 10.f, 0));
    //machine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    //SharedData::GetInstance()->m_goList.push_back(machine);

    //machine = new Machine();
    //machine->SetPartToCreate(RobotPart::MICROCHIP);
    //machine->SetPos(Vector3(3.f, 12.f, 0));
    //machine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    //SharedData::GetInstance()->m_goList.push_back(machine);

    // Worker + Assosiated Workstation
    Workstation* tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(6, 17, 0));
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    Worker* tempWorker = new Worker();
    tempWorker->Init();
    tempWorker->SetActive();
    tempWorker->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER));
    tempWorker->SetPos(Vector3(7, 17, 0));
    tempWorker->SetWorkstation(tempStation);
    SharedData::GetInstance()->m_goList.push_back(tempWorker);

    //SharedData::GetInstance()->AddGameObject(new Worker(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER), 7, 17);
    //SharedData::GetInstance()->AddGameObject(new Workstation(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION), 6, 17);

    //SharedData::GetInstance()->AddGameObject(new Worker(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER), 11, 13);
    //SharedData::GetInstance()->AddGameObject(new Workstation(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION), 11, 14);

    //SharedData::GetInstance()->AddGameObject(new Worker(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER), 6, 10);
    //SharedData::GetInstance()->AddGameObject(new Workstation(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION), 7, 10);

    //SharedData::GetInstance()->AddGameObject(new Worker(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER), 2, 8);
    //SharedData::GetInstance()->AddGameObject(new Workstation(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION), 2, 9);

    // Maintenance Man
    SharedData::GetInstance()->AddGameObject(new MaintenanceMan(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_MAINTENANCEMAN), 15, 7);

    // Scrap Man
    SharedData::GetInstance()->AddGameObject(new ScrapMan(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_SCRAPMAN), 14, 2);
}

//GameObject* SP3::FetchGameObject(OBJECT_TYPE ObjectType)
//{
//	GameObject* TempGameObject = NULL;
//
//	switch (ObjectType)
//	{
//		case OBJECT_TYPE::ENVIRONMENT:
//		{
//			TempGameObject = new Environment;
//			break;
//		}
//
//		case OBJECT_TYPE::PROJECTILE:
//		{
//			break;
//		}
//
//		case OBJECT_TYPE::PLAYER:
//		{		
//			break;
//		}
//
//		case OBJECT_TYPE::ENEMY:
//		{						
//			break;
//		}
//	}
//
//	return TempGameObject;
//}

//void SP3::SpawnGameObject(OBJECT_TYPE ObjectType, GAMEOBJECT_TYPE GoType, Vector3 Position, Vector3 Scale, bool Collidable, bool Visible)
//{
//	GameObject* go = FetchGameObject(ObjectType);
//
//	go->SetType(GoType);
//	go->SetPosition(Position);
//	go->SetScale(Scale);
//	go->SetCollidable(Collidable);
//	go->SetVisible(Visible);
//
//	switch (GoType)
//	{
//		case GO_BALL:
//		{
//			go->SetMesh(meshList[GO_BALL]);
//			break;
//		}
//	}
//
//	m_goList.push_back(go);
//}

void SceneAI::Update(double dt)
{
	SceneBase::Update(dt);
    
    for (int i = 0; i < SharedData::GetInstance()->m_goList.size(); ++i)
    {
        SharedData::GetInstance()->m_goList[i]->Update(dt);

        if (SharedData::GetInstance()->m_goList[i]->IsEntity())
        {
            Entity* entity = dynamic_cast<Entity*>(SharedData::GetInstance()->m_goList[i]);
            entity->RunFSM(dt);
        }
    }

    // DEBUG 

    if (Application::IsKeyPressed('1'))
        debugMachine->SetIsBroken(false);

    if (Application::IsKeyPressed('2'))
        debugMachine->SetIsEmpty(false);
}

void SceneAI::RenderGO(GameObject *go)
{
	modelStack.PushMatrix();
	modelStack.Translate(go->GetPos().x, go->GetPos().y, go->GetPos().z);
	//modelStack.Scale(1, 1, 1);
	RenderMesh(go->GetMesh(), false);
	modelStack.PopMatrix();
}

void SceneAI::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
    projection.SetToOrtho(0.5, SharedData::GetInstance()->m_gridMap->GetRows() - 0.5, -0.5, SharedData::GetInstance()->m_gridMap->GetColumns() - 0.5, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
		);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

    for (std::vector<GameObject*>::iterator it = SharedData::GetInstance()->m_goList.begin(); it != SharedData::GetInstance()->m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (go->IsActive())
		{
			RenderGO(go);
		}
	}


    // DEBUG 
    switch (debugMachine->GetState())
    {
    case Machine::REST:
        std::cout << "Machine: REST             Timer: " << debugMachine->GetTimer() << std::endl;
        break;
    case Machine::PRODUCTION:
        std::cout << "Machine: PRODUCTION       Timer: " << debugMachine->GetTimer() << std::endl;
        break;
    case Machine::BROKEN:
        std::cout << "Machine: BROKEN           Timer: " << debugMachine->GetTimer() << std::endl;
        break;
    case Machine::WAITFORREFILL:
        std::cout << "Machine: WAITFORREFILL    Timer: " << debugMachine->GetTimer() << std::endl;
        break;
    }
    
}

void SceneAI::Exit()
{
	SceneBase::Exit();

    SharedData::GetInstance()->Exit();
}
