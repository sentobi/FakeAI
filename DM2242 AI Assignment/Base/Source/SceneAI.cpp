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
#include "GameObject/Entity/Supervisor.h"

#include "GameObject/Toilet.h"
#include "GameObject/BuildingBlockStack.h"
#include "GameObject/Ornament.h"
#include "GameObject/ScrapPile.h"
#include "GameObject/DeliveryTruck.h"
#include "GameObject/Door.h"

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

    b_renderDebugInfo = false;
    index_renderDebugInfo = 0;
    d_keypressTimer = 0.0;

    brightnessFactor = 1.f;

    //===================
    // Create GameObjects
    //===================

    // Conveyor Belt
    ConveyorBelt* conveyor = new ConveyorBelt();
    conveyor->SetPos(Vector3(2, 12, 0));
    conveyor->Init();
    conveyor->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CONVEYORBELT_HORIZONTAL));
    conveyor->AddCheckpoint(Vector3(6, 12, 0));
    conveyor->AddCheckpoint(Vector3(8, 12, 0));
    conveyor->AddCheckpoint(Vector3(8, 10, 0));
    conveyor->AddCheckpoint(Vector3(8, 9, 0));
    conveyor->AddCheckpoint(Vector3(5, 9, 0));
    conveyor->AddCheckpoint(Vector3(3, 9, 0));
    conveyor->AddCheckpoint(Vector3(3, 6, 0));
    SharedData::GetInstance()->m_goList.push_back(conveyor);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[12][2] = true;

    for (int idx = 0; idx < conveyor->m_Checkpoints.size() - 1; ++idx)
    {
        int idx2 = idx + 1;
        Vector3 spawn = conveyor->m_Checkpoints[idx];
        while (spawn != conveyor->m_Checkpoints[idx2])
        {
            if (spawn.x < conveyor->m_Checkpoints[idx2].x)
            {
                spawn.x += 1;
            }
            else if (spawn.x > conveyor->m_Checkpoints[idx2].x)
            {
                spawn.x -= 1;
            }
            else if (spawn.y < conveyor->m_Checkpoints[idx2].y)
            {
                spawn.y += 1;
            }
            else if (spawn.y > conveyor->m_Checkpoints[idx2].y)
            {
                spawn.y -= 1;
            }

            // this should be moved somewhere else, init maybe
            SharedData::GetInstance()->m_gridMap->m_collisionGrid[(int)spawn.y][(int)spawn.x] = true;
        }
    }

    // Toilet
    //SharedData::GetInstance()->AddGameObject(new Toilet(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_TOILET), 14, 14);

    // Building Blocks Pile
    SharedData::GetInstance()->AddGameObject(new BuildingBlockStack(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_BUILDINGBLOCK_STACK), 6, 5);
    SharedData::GetInstance()->AddGameObject(new BuildingBlockStack(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_BUILDINGBLOCK_STACK), 7, 5);
    SharedData::GetInstance()->AddGameObject(new BuildingBlockStack(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_BUILDINGBLOCK_STACK), 8, 5);

    // Ornaments
    SharedData::GetInstance()->AddGameObject(new Ornament(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_ORNAMENT), 6, 1, true);
    SharedData::GetInstance()->AddGameObject(new Ornament(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_ORNAMENT), 7, 1, true);
    SharedData::GetInstance()->AddGameObject(new Ornament(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_ORNAMENT), 8, 1, true);

    SharedData::GetInstance()->m_ornamentSystem->SetLaneToBlock(6);
    SharedData::GetInstance()->m_ornamentSystem->SetLaneToOrnament(8);

    // Door 
    SharedData::GetInstance()->AddGameObject(new Door(), SharedData::GetInstance()->m_meshList->GetMesh(GEO_DOOR), 14, 8);

    //===================
    // Create Entities
    //===================

    // Toilet
    Toilet* tempToilet = new Toilet();
    tempToilet->Init();
    tempToilet->SetPos(Vector3(14.f, 14.f, 0));
    tempToilet->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TOILET));
    SharedData::GetInstance()->m_goList.push_back(tempToilet);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[14][14] = true;

    // Machine
    debugMachine = new Machine();
    debugMachine->Init();
    debugMachine->SetPartToCreate(RobotPart::BODY);
    debugMachine->SetPos(Vector3(1.f, 12.f, 0));
    debugMachine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    debugMachine->SetSpawnLocation(conveyor->GetCheckpoint(0));
    SharedData::GetInstance()->m_goList.push_back(debugMachine);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[12][1] = true;

    Machine* machine = new Machine();
    machine->Init();
    machine->SetPartToCreate(RobotPart::HEAD);
    machine->SetPos(Vector3(8.f, 13.f, 0));
    machine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    machine->SetSpawnLocation(conveyor->GetCheckpoint(2));
    SharedData::GetInstance()->m_goList.push_back(machine);
    SharedData::GetInstance()->m_gridMap->m_collisionGrid[13][8] = true;

    machine = new Machine();
    machine->Init();
    machine->SetPartToCreate(RobotPart::LIMB);
    machine->SetPos(Vector3(8.f, 8.f, 0));
    machine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    machine->SetSpawnLocation(conveyor->GetCheckpoint(4));
    SharedData::GetInstance()->m_goList.push_back(machine);
    SharedData::GetInstance()->m_gridMap->m_collisionGrid[8][8] = true;

    machine = new Machine();
    machine->Init();
    machine->SetPartToCreate(RobotPart::MICROCHIP);
    machine->SetPos(Vector3(2.f, 9.f, 0));
    machine->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MACHINE));
    machine->SetSpawnLocation(conveyor->GetCheckpoint(6));
    SharedData::GetInstance()->m_goList.push_back(machine);
    SharedData::GetInstance()->m_gridMap->m_collisionGrid[9][2] = true;

    // Maintenance Man + Assosiated Workstation ( 1 )
    Workstation* tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(13, 6, 0));
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[6][13] = true;

    MaintenanceMan* maintenance = new MaintenanceMan();
    maintenance->Init();
    maintenance->SetActive();
    maintenance->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MAINTENANCEMAN));
    maintenance->SetPos(Vector3(13, 5, 0));
    maintenance->SetWorkstation(tempStation);
    maintenance->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(maintenance);

    //SharedData::GetInstance()->m_gridMap->m_collisionGrid[5][13] = true;

    // Maintenance Man + Assosiated Workstation ( 2 )
    tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(11, 6, 0));
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[6][11] = true;

    maintenance = new MaintenanceMan();
    maintenance->Init();
    maintenance->SetActive();
    maintenance->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MAINTENANCEMAN));
    maintenance->SetPos(Vector3(11, 5, 0));
    maintenance->SetWorkstation(tempStation);
    maintenance->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(maintenance);

    //SharedData::GetInstance()->m_gridMap->m_collisionGrid[5][11] = true;

    // Scrap Man + Assosiated Scrap Pile ( 1 )
    ScrapPile* pile = new ScrapPile();
    SharedData::GetInstance()->AddGameObject(pile, SharedData::GetInstance()->m_meshList->GetMesh(GEO_SCRAP_PILE), 12, 2);

    ScrapMan* scrapMan = new ScrapMan();
    SharedData::GetInstance()->AddGameObject(scrapMan, SharedData::GetInstance()->m_meshList->GetMesh(GEO_SCRAPMAN), 11, 2);
    scrapMan->SetPos(Vector3(11, 2, 0));
    scrapMan->AssignScrapPile(pile);
    scrapMan->SetToilet(tempToilet);

    //scrapMan->SetTempRole(maintenance);

    // Worker + Assosiated Workstation ( 1 )
    tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(5, 13, 0));
    tempStation->SetTypeStored(RobotPart::BODY);
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[13][5] = true;

    Worker* tempWorker = new Worker();
    tempWorker->Init();
    tempWorker->SetActive();
    tempWorker->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER));
    tempWorker->SetPos(Vector3(6, 13, 0));
    tempWorker->SetWorkstation(tempStation);
    tempWorker->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(tempWorker);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[13][6] = true;

    // Worker + Assosiated Workstation ( 2 )
    tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(9, 10, 0));
    tempStation->SetTypeStored(RobotPart::HEAD);
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[10][9] = true;

    tempWorker = new Worker();
    tempWorker->Init();
    tempWorker->SetActive();
    tempWorker->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER));
    tempWorker->SetPos(Vector3(9, 9, 0));
    tempWorker->SetWorkstation(tempStation);
    tempWorker->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(tempWorker);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[9][9] = true;

    // Worker + Assosiated Workstation ( 3 )
    tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(6, 8, 0));
    tempStation->SetTypeStored(RobotPart::LIMB);
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[8][6] = true;

    tempWorker = new Worker();
    tempWorker->Init();
    tempWorker->SetActive();
    tempWorker->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER));
    tempWorker->SetPos(Vector3(5, 8, 0));
    tempWorker->SetWorkstation(tempStation);
    tempWorker->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(tempWorker);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[8][5] = true;

    // Worker + Assosiated Workstation ( 4 )
    tempStation = new Workstation();
    tempStation->Init();
    tempStation->SetActive();
    tempStation->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKSTATION));
    tempStation->SetPos(Vector3(2, 7, 0));
    tempStation->SetTypeStored(RobotPart::MICROCHIP);
    SharedData::GetInstance()->m_goList.push_back(tempStation);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[7][2] = true;

    tempWorker = new Worker();
    tempWorker->Init();
    tempWorker->SetActive();
    tempWorker->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WORKER));
    tempWorker->SetPos(Vector3(2, 6, 0));
    tempWorker->SetWorkstation(tempStation);
    tempWorker->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(tempWorker);

    //tempWorker->SetTempRole(maintenance);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[6][2] = true;

    // Delivery Man + Delivery Truck
    DeliveryTruck* truck = new DeliveryTruck();
    SharedData::GetInstance()->AddGameObject(truck, SharedData::GetInstance()->m_meshList->GetMesh(GEO_DELIVERYTRUCK), 1, 2);
    DeliveryMan* deliveryMan = new DeliveryMan();
    SharedData::GetInstance()->AddGameObject(deliveryMan, SharedData::GetInstance()->m_meshList->GetMesh(GEO_DELIVERYMAN), 1, 2);
    deliveryMan->AssignDeliveryTruck(truck);

    // Supervisor
    Supervisor* sup = new Supervisor();
    sup->Init();
    sup->SetActive();
    sup->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_SUPERVISOR));
    sup->SetPos(Vector3(12, 10, 0));
    sup->SetToilet(tempToilet);
    SharedData::GetInstance()->m_goList.push_back(sup);

    SharedData::GetInstance()->m_gridMap->m_collisionGrid[10][12] = true;

    // spawn temp Robot
    Robot* robot = new Robot();
    robot->Init();
    robot->SetActive();
    robot->SetMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_ROBOT));
    robot->SetPos(Vector3(7, 3, 0));
    robot->SetState(Robot::STARTUP);
    SharedData::GetInstance()->m_goList.push_back(robot);

    //// Scrap Pile
    //ScrapPile* pile = new ScrapPile();
    //SharedData::GetInstance()->AddGameObject(pile, SharedData::GetInstance()->m_meshList->GetMesh(GEO_SCRAP_PILE), 12, 2);

    //// Scrap Man
    //ScrapMan* scrapMan = new ScrapMan();
    //SharedData::GetInstance()->AddGameObject(scrapMan, SharedData::GetInstance()->m_meshList->GetMesh(GEO_SCRAPMAN), 11, 2);
    //scrapMan->AssignScrapPile(pile);
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

    d_keypressTimer += dt;

    // Checking for keypress for rendering of debug info
    if (d_keypressTimer > 0.2)
    if (Application::IsKeyPressed(VK_TAB))
    {
        b_renderDebugInfo = !b_renderDebugInfo;
        d_keypressTimer = 0.0;
    }
    else if (Application::IsKeyPressed(VK_LEFT) && b_renderDebugInfo)
    {
        do {
            --index_renderDebugInfo;
            if (index_renderDebugInfo == -1) {
                index_renderDebugInfo = SharedData::GetInstance()->m_goList.size() - 1;
            }
        } while (!SharedData::GetInstance()->m_goList[index_renderDebugInfo]->IsActive());

        d_keypressTimer = 0.0;
    }
    else if (Application::IsKeyPressed(VK_RIGHT) && b_renderDebugInfo)
    {
        do {
            ++index_renderDebugInfo;
            if (index_renderDebugInfo == SharedData::GetInstance()->m_goList.size()) {
                index_renderDebugInfo = 0;
            }
        } while (!SharedData::GetInstance()->m_goList[index_renderDebugInfo]->IsActive());

        d_keypressTimer = 0.0;
    }

    // Update the clock (day/night cycle)
    int hours = SharedData::GetInstance()->m_clock->GetCurrHours();
    std::string dayAbbr = SharedData::GetInstance()->m_clock->GetCurrDayAbbreviation();
    double deltaTime = 300 * dt;
    if (dayAbbr == "SAT" || dayAbbr == "SUN" || hours < 8 || hours > 18) {
        if (Entity::IsSomeoneOnLeave()) {
            Entity::SetSomeoneOnLeave(false);
            for (int i = 0; i < SharedData::GetInstance()->m_goList.size(); ++i)
            {
                GameObject* go = SharedData::GetInstance()->m_goList[i];
                if (go->IsEntity()) {
                    Entity* entity = dynamic_cast<Entity*>(go);
                    entity->SetOnLeave(false);
                }
            }
        }
        deltaTime *= 10;
    }
    SharedData::GetInstance()->m_clock->Update(deltaTime);

    // factory brightness
    if (dayAbbr != "SAT" && dayAbbr != "SUN")
    {
        if (hours >= 18) {
            if (brightnessFactor > 0.3f)
            {
                brightnessFactor -= 0.15f * (float)dt;
                brightnessFactor = Math::Max(brightnessFactor, 0.3f);
            }
        }
        else if (hours >= 8) {
            if (brightnessFactor < 1.f)
            {
                brightnessFactor += 0.15f * (float)dt;
                brightnessFactor = Math::Min(brightnessFactor, 1.f);
            }
        }
    }

    // Update messageboard
    SharedData::GetInstance()->m_messageBoard->Update(dt);

    // Update GameObjects
    for (int i = 0; i < SharedData::GetInstance()->m_goList.size(); ++i)
    {
        GameObject* go = SharedData::GetInstance()->m_goList[i];
        go->Update(dt);

        if (go->IsEntity())
        {
            Entity* entity = dynamic_cast<Entity*>(go);
            entity->RunFSM(dt);

            if (entity->GetTempRole())
                CheckEntityTempRoleComplete(entity);

            if (entity->b_newRoleVisual) {
                entity->d_newRoleVisual += dt;
                if (entity->d_newRoleVisual > 1.0) {
                    entity->d_newRoleVisual = 0.0;
                    entity->b_newRoleVisual = false;
                }
            }
        }
    }

    // Update the Ornament Handler System
    SharedData::GetInstance()->m_ornamentSystem->Update(dt);
}

void SceneAI::CheckEntityTempRoleComplete(Entity* entity)
{
    if (entity->GetTempRole()->GetName() == "Worker")
    {
        Worker* worker = dynamic_cast<Worker*>(entity->GetTempRole());
        if (worker->GetState() == Worker::IDLE && !SharedData::GetInstance()->m_clock->GetIsWorkDay() && !SharedData::GetInstance()->m_clock->GetIsWorkStarted())
        {
            if (entity->GetName() == "Maintenance Man") {
                MaintenanceMan* man = dynamic_cast<MaintenanceMan*>(entity);
                man->RemoveTempRole();
                man->SetState(MaintenanceMan::IDLE);

                return;
            }
            else if (entity->GetName() == "Scrap Man") {
                ScrapMan* man = dynamic_cast<ScrapMan*>(entity);
                man->RemoveTempRole();
                man->SetState(ScrapMan::IDLE);

                return;
            }
        }
    }

    else if (entity->GetTempRole()->GetName() == "Maintenance Man")
    {
        MaintenanceMan* man = dynamic_cast<MaintenanceMan*>(entity->GetTempRole());
        if ((man->GetState() == MaintenanceMan::IDLE && man->b_doneTempJob) ||
            (!SharedData::GetInstance()->m_clock->GetIsWorkDay() && !SharedData::GetInstance()->m_clock->GetIsWorkStarted()) )
        {
            if (entity->GetName() == "Worker") {
                Worker* worker = dynamic_cast<Worker*>(entity);
                worker->RemoveTempRole();
                worker->SetState(Worker::IDLE);

                return;
            }
            else if (entity->GetName() == "Scrap Man") {
                ScrapMan* scrapman = dynamic_cast<ScrapMan*>(entity);
                scrapman->RemoveTempRole();
                scrapman->SetState(ScrapMan::IDLE);

                return;
            }
        }
    }

    else if (entity->GetTempRole()->GetName() == "Scrap Man")
    {
        ScrapMan* scrapman = dynamic_cast<ScrapMan*>(entity->GetTempRole());
        if ((scrapman->GetState() == ScrapMan::IDLE && scrapman->b_doneTempJob) ||
            (!SharedData::GetInstance()->m_clock->GetIsWorkDay() && !SharedData::GetInstance()->m_clock->GetIsWorkStarted()) )
        {
            if (entity->GetName() == "Worker") {
                Worker* worker = dynamic_cast<Worker*>(entity);
                worker->RemoveTempRole();
                worker->SetState(Worker::IDLE);

                return;
            }
            else if (entity->GetName() == "Maintenance Man") {
                MaintenanceMan* man = dynamic_cast<MaintenanceMan*>(entity);
                man->RemoveTempRole();
                man->SetState(MaintenanceMan::IDLE);

                return;
            }
        }
    }

}

void SceneAI::RenderBackground()
{
    modelStack.PushMatrix();
    modelStack.Translate(-0.5f + 0.5f * SharedData::GetInstance()->m_gridMap->GetRows(), -0.5f + 0.5f * SharedData::GetInstance()->m_gridMap->GetColumns(), -1);
    modelStack.Scale(SharedData::GetInstance()->m_gridMap->GetRows(), SharedData::GetInstance()->m_gridMap->GetColumns(), 1);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_FLOOR), false);
    modelStack.PopMatrix();
}

void SceneAI::RenderGO(GameObject *go)
{
	modelStack.PushMatrix();

    // if GameObject is something unique and needs to change its offset (esp for z-axis)
    if (go->GetName() == "ConveyorBelt")
        modelStack.Translate(0, 0, -0.5f);
    else if (go->GetName() == "DeliveryTruck")
        modelStack.Translate(0, -0.1f, 0.5f);
    else if (go->GetName() == "Door")
        modelStack.Translate(0, 0, 0.5f);
    else if (go->GetName() == "Workstation")
    {
        Workstation* workstation = dynamic_cast<Workstation*>(go);
        if (workstation->IfHasRobotPart())
        {
            modelStack.PushMatrix();
            modelStack.Translate(go->GetPos().x, go->GetPos().y - go->GetScale().y * 0.5f, -0.5f);
            modelStack.Scale(go->GetScale().x * 0.5f, go->GetScale().y * 0.5f, go->GetScale().z);
            RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_ROBOT_PART_PACKAGE), false);
            modelStack.PopMatrix();
        }
    }
    
    // Render the object
    modelStack.Translate(go->GetPos().x, go->GetPos().y, go->GetPos().z);
    modelStack.Scale(go->GetScale().x, go->GetScale().y, go->GetScale().z);

    // Render message notification for Entities
    if (go->IsEntity())
    {
        Entity* entity = dynamic_cast<Entity*>(go);
        if (entity->b_newRoleVisual) {
            if (Math::RandIntMinMax(0, 1))
                RenderMesh(go->GetMesh(), false);
        }
        else
        {
            RenderMesh(go->GetMesh(), false);
        }

        if (entity->GetTempRole()) {
            Entity* tempRole = entity->GetTempRole();
            RenderTempRole(tempRole);
            if (tempRole->HasMessageComeIn())
            {
                glUniform1i(m_parameters[U_HIGHLIGHTED], 0);
                RenderMessageNotification(tempRole);
            }
            else if (tempRole->HasAcknowledgedMessage())
            {
                glUniform1i(m_parameters[U_HIGHLIGHTED], 0);
                RenderAcknowledgeNotification(tempRole);
            }
        }

        else if (entity->HasMessageComeIn())
        {
            glUniform1i(m_parameters[U_HIGHLIGHTED], 0);
            RenderMessageNotification(entity);
        }
        else if (entity->HasAcknowledgedMessage())
        {
            glUniform1i(m_parameters[U_HIGHLIGHTED], 0);
            RenderAcknowledgeNotification(entity);
        }
    }
    else
    {
        RenderMesh(go->GetMesh(), false);
    }

	modelStack.PopMatrix();
}

void SceneAI::RenderTempRole(Entity* tempRole)
{
    //std::string name = tempRole->GetName();
    //RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), name, Color(0, 0, 0), 1, ((tempRole->GetPos().x + 1) * 5.3f) - (name.size() * 0.5f), ((1 + tempRole->GetPos().y) * 4.f) - 1.5f);

    modelStack.PushMatrix();
    modelStack.Translate(0.f, 0.3f, 0.f);
    modelStack.Scale(0.5f, 0.5f, 0.5f);

    if (tempRole->GetName() == "Worker")
        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEMPROLE_WORKER), false);
    else if (tempRole->GetName() == "Maintenance Man")
        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEMPROLE_MAINTENANCEMAN), false);
    else if (tempRole->GetName() == "Scrap Man")
        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEMPROLE_SCRAPMAN), false);

    modelStack.PopMatrix();
}

void SceneAI::RenderMessageNotification(Entity* entity)
{
    modelStack.PushMatrix();
    modelStack.Translate(0.f, 0.1f + entity->GetRenderNotifOffset(), 1.f);
    modelStack.Scale(0.5f, 0.5f, 0.5f);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MESSAGENOTIF), false);
    modelStack.PopMatrix();
}

void SceneAI::RenderAcknowledgeNotification(Entity* entity)
{
    modelStack.PushMatrix();
    modelStack.Translate(0.f, 0.1f + entity->GetRenderNotifOffset(), 1.f);
    modelStack.Scale(0.5f, 0.5f, 0.5f);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_ACKNOWLEDGETICK), false);
    modelStack.PopMatrix();
}

void SceneAI::RenderMessageBoard()
{
    // Render the board
    modelStack.PushMatrix();
    modelStack.Translate(1.5f, SharedData::GetInstance()->m_gridMap->GetColumns() - 12.5f, 1.5f);
    modelStack.Scale(4.f, 4.f, 1);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_MESSAGEBOARD), false);
    modelStack.PopMatrix();

    // Render the messages
    MessageBoard* mb = SharedData::GetInstance()->m_messageBoard;
    Color msgColor(0, 0, 0);
    for (int i = 0; i < mb->GetMessageListSize(); ++i)
    {
        Message* msg = mb->GetAMessage(mb->GetMessageListSize() - 1 - i);

        if (msg->IsAcknowledged()) {
            msgColor.Set(0, 1, 0);
        }
        else {
            msgColor.Set(0, 0, 0);
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), msg->GetTime(), msgColor, 1, 15, 15 - i * 3);
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), msg->GetMessageString(), msgColor, 1, 1, 15.f - i * 3);
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "From: " + msg->GetMessageFrom(), msgColor, 1, 1, 16.f - i * 3);
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "To: " + msg->GetMessageTo(), msgColor, 1, 10, 16.f - i * 3);
    }
    
}

void SceneAI::RenderClock()
{
    // base beneath clock
    modelStack.PushMatrix();
    modelStack.Translate(0.5f, SharedData::GetInstance()->m_gridMap->GetColumns() - 1.5f, 0.5f);
    modelStack.Scale(1.75f, 1.75f, 1);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CLOCKBASE), false);
    modelStack.PopMatrix();

    // Minutes nodes beneath clock
    int minutes = SharedData::GetInstance()->m_clock->GetCurrMinutes() + 1;
    float anglePerMinute = -360.f / 60;
    for (int i = 0; i < minutes; ++i)
    {
        modelStack.PushMatrix();
        modelStack.Translate(0.5f, SharedData::GetInstance()->m_gridMap->GetColumns() - 0.71f, 1);
        modelStack.Translate(0, -0.8f, 0);
        modelStack.Rotate(anglePerMinute * i, 0, 0, 1);
        modelStack.Translate(0, 0.8f, 0);
        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CLOCKMINUTE), false);
        modelStack.PopMatrix();
    }
    
    // Hours nodes beneath clock
    int hours = SharedData::GetInstance()->m_clock->GetCurrHours() % 12;
    float anglePerHour = -360.f / 12;
    //for (int i = 0; i < hours + 1; ++i)
    //{
    //    modelStack.PushMatrix();
    //    modelStack.Translate(0.5f, SharedData::GetInstance()->m_gridMap->GetColumns() - 0.8f, 1);
    //    modelStack.Translate(0, -0.7f, 0);
    //    modelStack.Rotate(anglePerHour * i, 0, 0, 1);
    //    modelStack.Translate(0, 0.7f, 0);
    //    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CLOCKHOUR), false);
    //    modelStack.PopMatrix();
    //}

    // Render a single Hour node
    modelStack.PushMatrix();
    modelStack.Translate(0.5f, SharedData::GetInstance()->m_gridMap->GetColumns() - 0.8f, 1);
    modelStack.Translate(0, -0.7f, 0);
    modelStack.Rotate(anglePerHour * hours, 0, 0, 1);
    modelStack.Translate(0, 0.7f, 0);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CLOCKHOUR), false);
    modelStack.PopMatrix();
    
    // Clock body
    modelStack.PushMatrix();
    modelStack.Translate(0.5f, SharedData::GetInstance()->m_gridMap->GetColumns() - 1.5f, 2);
    modelStack.Scale(1.75f, 1.75f, 1);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_CLOCK), false);
    modelStack.PopMatrix();
    
    RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), SharedData::GetInstance()->m_clock->GetCurrTime(), Color(1, 0, 0), 2, 3, 55.25f, true);
    RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), SharedData::GetInstance()->m_clock->GetCurrDayAbbreviation(), Color(1, 0, 0), 1.5f, 4.25f, 54);
}

void SceneAI::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
    projection.SetToOrtho(-0.5, SharedData::GetInstance()->m_gridMap->GetRows() - 0.5, -0.5, SharedData::GetInstance()->m_gridMap->GetColumns() - 0.5, -10, 10);
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

    glUniform1f(m_parameters[U_BRIGHTNESS], brightnessFactor);

    RenderBackground();

    //for (std::vector<GameObject*>::iterator it = SharedData::GetInstance()->m_goList.begin(); it != SharedData::GetInstance()->m_goList.end(); ++it)
    //{
    for (int i = 0; i < SharedData::GetInstance()->m_goList.size(); ++i)
    {
        GameObject *go = SharedData::GetInstance()->m_goList[i];
        if (i == index_renderDebugInfo && b_renderDebugInfo)
            glUniform1i(m_parameters[U_HIGHLIGHTED], 1);

        if (go->IsActive())
        {
            // Special Case for conveyor belt
            if (go->GetName() == "ConveyorBelt")
            {
                ConveyorBelt* belt = dynamic_cast<ConveyorBelt*>(go);

                for (int idx = 0; idx < belt->m_Checkpoints.size() - 1; ++idx)
                {
                    GEOMETRY_TYPE geo_type = GEO_CONVEYORBELT_HORIZONTAL;   // default

                    int idx2 = idx + 1;
                    Vector3 spawn = belt->m_Checkpoints[idx];
                    while (spawn != belt->m_Checkpoints[idx2])
                    {
                        if (spawn.x < belt->m_Checkpoints[idx2].x)
                        {
                            spawn.x += 1;
                            if (spawn.x == belt->m_Checkpoints[idx2 + 1].x)
                                geo_type = GEO_CONVEYORBELT_TOPRIGHT;
                        }
                        else if (spawn.x > belt->m_Checkpoints[idx2].x)
                        {
                            spawn.x -= 1;
                            if (geo_type == GEO_CONVEYORBELT_BOTRIGHT)
                                geo_type = GEO_CONVEYORBELT_HORIZONTAL;

                            else if (spawn.x == belt->m_Checkpoints[idx2 + 1].x)
                                geo_type = GEO_CONVEYORBELT_TOPLEFT;
                        }
                        else if (spawn.y < belt->m_Checkpoints[idx2].y)
                        {
                            spawn.y += 1;
                        }
                        else if (spawn.y > belt->m_Checkpoints[idx2].y)
                        {
                            spawn.y -= 1;
                            geo_type = GEO_CONVEYORBELT_VERTICAL;
                            if (idx2 + 1 == belt->m_Checkpoints.size())
                                geo_type = GEO_CONVEYORBELT_VERTICALEND;

                            else if (spawn.y == belt->m_Checkpoints[idx2 + 1].y)
                                geo_type = GEO_CONVEYORBELT_BOTRIGHT;
                        }

                        modelStack.PushMatrix();
                        modelStack.Translate(spawn.x, spawn.y, -0.5f);
                        modelStack.Scale(go->GetScale().x, go->GetScale().y, go->GetScale().z);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(geo_type), false);
                        modelStack.PopMatrix();
                    }
                    
                }
            }
            
            RenderGO(go);
        }

        if (i == index_renderDebugInfo)
            glUniform1i(m_parameters[U_HIGHLIGHTED], 0);
    }

    glUniform1f(m_parameters[U_BRIGHTNESS], 1.f);

    //============================
    // Render GUI/on-screen stuff
    //============================

    // Render header
    RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "ROBOTOPIA", Color(0, 0, 0), 4, 11, 56);

    // Render message board
    RenderMessageBoard();

    // Render clock
    RenderClock();

    if (b_renderDebugInfo)
    {
        RenderDebugInfo();
    }

    /*// pathfinder test
    modelStack.PushMatrix();
    modelStack.Translate(pathfinderTest.position.x, pathfinderTest.position.y, 0);
    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_ROBOT_HEAD), false);
    modelStack.PopMatrix();

    for (std::vector<Node>::iterator it = pathfinderTest.foundPath.begin(); it != pathfinderTest.foundPath.end(); ++it)
    {
        modelStack.PushMatrix();
        modelStack.Translate((*it).col, (*it).row, 0);
        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
        modelStack.PopMatrix();
    }*/
    
}

void SceneAI::RenderDebugInfo()
{
    // GENERIC INFO
    std::stringstream ss;

    //grid info
    ss << "Grid: " << SharedData::GetInstance()->m_gridMap->GetRows() << " x " << SharedData::GetInstance()->m_gridMap->GetColumns();
    RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2.5, 35, 57.5);

    //fps
    ss.str("");
    ss << "FPS: " << fps;
    RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2.5, 56, 57.5);


    // SPECIFIC INFO

    //CONVEYOR BELT
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "ConveyorBelt")
    {
        ConveyorBelt* conveyorbelt = dynamic_cast<ConveyorBelt*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        // Render Waypoints
        Vector3 waypointPos;
        int idx = 0;

        for (int idx = 0; idx < conveyorbelt->m_Checkpoints.size(); ++idx)
        {
            waypointPos = conveyorbelt->GetCheckpoint(idx);
            modelStack.PushMatrix();
            modelStack.Translate(waypointPos.x, waypointPos.y, 0.5f);
            RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WAYPOINT_NODE), false);
            modelStack.PopMatrix();
        }

    }

    //MACHINE
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Machine")
    {
        Machine* machine = dynamic_cast<Machine*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        // State
        std::string stateStr = "";
        switch (machine->GetState())
        {
        case Machine::REST:
            stateStr = "Rest";
            break;

        case Machine::PRODUCTION:
            stateStr = "Production";
            break;

        case Machine::WAITFORREFILL:
            stateStr = "Refill";
            break;

        case Machine::BROKEN:
            stateStr = "Broken";
            break;

        case Machine::SHUTDOWN:
            stateStr = "Powered Off";
            break;
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State: " + stateStr, Color(0, 0, 0), 2, 18, 0);

        // Overheat Charge
        ss.str("");
        ss << "Breakdown: " << (int)machine->GetOverheatCharge() << " > " << machine->randNum;
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 37, 0);

        // Scrap Quantity
        ss.str("");
        ss << "Scrap: " << machine->GetScrapQuantity() << " / " << machine->GetMaxScrapQuantity() << " Cost: " << machine->GetPartToCreate();
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 57, 0);
    }

    //WORKER
    else if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Worker")
    {
        Worker* worker = dynamic_cast<Worker*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);
        
        if (worker->GetTempRole())
        {
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "Temp Role: " + worker->GetTempRole()->GetName(), Color(0, 0, 0), 2, 18, 0);

            Pathfinder* pathfinder = worker->GetTempRole()->GetPathfinder();
            if (pathfinder)
            {
                if (!pathfinder->IsPathEmpty())
                {
                    for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                    {
                        modelStack.PushMatrix();
                        modelStack.Translate((*it).col, (*it).row, -0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                        modelStack.PopMatrix();
                    }
                }
            }   // end of pathfinder
        }
        else
        {
            // State
            std::string stateStr = "";
            switch (worker->GetState())
            {
            case Worker::IDLE:
                stateStr = "Idle";
                break;

            case Worker::WORK:
                stateStr = "Work";
                break;

            case Worker::BREAK:
                stateStr = "Break";
                break;

            case Worker::OFFWORK:
                stateStr = "Off Work";
                break;
            }

            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State: " + stateStr, Color(0, 0, 0), 2, 18, 0);

            // Break Charge
            ss.str("");
            ss << "Break: " << (int)worker->GetBreakCharge() << " / 2000   " << (int)worker->randNum << " > 50";
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 32, 0);

            // Timer
            ss.str("");
            ss.precision(2);
            ss << "Timer: " << worker->m_timer;
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 62, 0);

            Pathfinder* pathfinder = worker->GetPathfinder();
            if (pathfinder)
            {
                if (!pathfinder->IsPathEmpty())
                {
                    for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                    {
                        modelStack.PushMatrix();
                        modelStack.Translate((*it).col, (*it).row, -0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                        modelStack.PopMatrix();
                    }
                }
            }   // end of pathfinder
        }

    }

    //MAINTENANCE MAN
    else if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Maintenance Man")
    {
        MaintenanceMan* maintenanceman = dynamic_cast<MaintenanceMan*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        if (maintenanceman->GetTempRole())
        {
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "Temp Role: " + maintenanceman->GetTempRole()->GetName(), Color(0, 0, 0), 2, 32, 0);

            Pathfinder* pathfinder = maintenanceman->GetTempRole()->GetPathfinder();
            if (pathfinder)
            {
                if (!pathfinder->IsPathEmpty())
                {
                    // nodes
                    for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                    {
                        modelStack.PushMatrix();
                        modelStack.Translate((*it).col, (*it).row, -0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                        modelStack.PopMatrix();
                    }
                }
            }   // end of pathfinder
        }
        else
        {
            // State
            std::string stateStr = "";
            switch (maintenanceman->GetState())
            {
            case MaintenanceMan::IDLE:
                stateStr = "Idle";
                break;

            case MaintenanceMan::REPAIR:
                stateStr = "Repair";
                break;

            case MaintenanceMan::REFILL:
                stateStr = "Refill";
                break;

            case MaintenanceMan::BREAK:
                stateStr = "Break";
                break;

            case MaintenanceMan::OFFWORK:
                stateStr = "Off Work";
                break;
            }

            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State: " + stateStr, Color(0, 0, 0), 2, 32, 0);

            // Break Charge
            ss.str("");
            ss << "Break: " << (int)maintenanceman->GetBreakCharge() << " / 2000   " << (int)maintenanceman->randNum << " > 50";
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 50, 0);

            Pathfinder* pathfinder = maintenanceman->GetPathfinder();
            if (pathfinder)
            {
                if (!pathfinder->IsPathEmpty())
                {
                    if (maintenanceman->GetTargetMachine() != NULL)
                    {
                        // destination machine
                        modelStack.PushMatrix();
                        modelStack.Translate(pathfinder->foundPath.begin()->GetPosition().x, pathfinder->foundPath.begin()->GetPosition().y, 0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_HIGHLIGHTBOX), false);
                        modelStack.PopMatrix();
                    }

                    // nodes
                    for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                    {
                        modelStack.PushMatrix();
                        modelStack.Translate((*it).col, (*it).row, -0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                        modelStack.PopMatrix();
                    }
                }
            }   // end of pathfinder
        }

    }

    //TOILET
    else if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Toilet")
    {
        Toilet* toilet = dynamic_cast<Toilet*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        // Occupy
        ss.str("");

        if (toilet->IsOccupied())
            ss << "Occupied: TRUE";
        else
            ss << "Occupied: FALSE";

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 18, 0);
    
        // Queue size
        ss.str("");
        ss << "Queue Size: " << toilet->GetToiletSize(); 
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 38, 0);
    }

    //WORKSTATION
    else if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Workstation")
    {
        Workstation* station = dynamic_cast<Workstation*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        // Occupy
        ss.str("");
        ss << "Type: ";

        switch (station->GetTypeStored())
        {
        case RobotPart::HEAD:
            ss << "Head";
            break;

        case RobotPart::BODY:
            ss << "Body";
            break;

        case RobotPart::LIMB:
            ss << "Limb";
            break;

        case RobotPart::MICROCHIP:
            ss << "Chip";
            break;

        default:
            ss << "NULL";
            break;
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 22, 0);

        // Number Stored
        ss.str("");
        ss << "Stored Amount: " << station->GetStoredAmount();
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 35, 0);

        // Robot on station
        ss.str("");

        if (station->IfRobotAtStation())
            ss << "Robot Present: TRUE";
        else
            ss << "Robot Present: FALSE";

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 55, 0);
    }

    //ROBOT
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Robot")
    {
        Robot* robot = dynamic_cast<Robot*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        // State
        std::string stateStr = "";
        switch (robot->GetState())
        {
        case Robot::INCOMPLETE_1:
            stateStr = "Incomplete";
            break;

        case Robot::INCOMPLETE_2:
            stateStr = "Incomplete";
            break;

        case Robot::INCOMPLETE_3:
            stateStr = "Incomplete";
            break;

        case Robot::STARTUP:
            stateStr = "Starting Up";
            break;

        case Robot::WORK_WITHOUTPART:
            stateStr = "Work";
            break;

        case Robot::WORK_WITHPART:
            stateStr = "Work";
            break;

        case Robot::CHEER:
            stateStr = "Cheering";
            break;

        case Robot::SHUTDOWN:
            stateStr = "Shut Down";
            break;

        case Robot::POWEROFF:
            stateStr = "Powered Off";
            break;
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State: " + stateStr, Color(0, 0, 0), 2, 18, 0);

        // Lifetime
        ss.str("");
        ss << "Lifetime: " << (int)robot->GetLifetime();
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 40, 0);

        // Whether Robot is carrying a part
        if (robot->GetState() == Robot::WORK_WITHOUTPART || robot->GetState() == Robot::WORK_WITHPART)
        {
            ss.str("");
            ss << "Carrying Part: ";
            if (robot->GetState() == Robot::WORK_WITHOUTPART)
                ss << "No";
            else
                ss << "Yes";
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 60, 0);
        }

        Pathfinder* pathfinder = robot->GetPathfinder();
        if (pathfinder)
        {
            if (!pathfinder->IsPathEmpty())
            {
                for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                {
                    modelStack.PushMatrix();
                    modelStack.Translate((*it).col, (*it).row, -0.5f);
                    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                    modelStack.PopMatrix();
                }
            }
        }   // end of pathfinder

        if (robot->GetState() == Robot::WORK_WITHOUTPART && !pathfinder->IsPathEmpty())
        {
            Vector3 stackPos = pathfinder->foundPath.begin()->GetPosition() + Vector3(0, 1, 0);
            modelStack.PushMatrix();
            modelStack.Translate(stackPos.x, stackPos.y, 0.5f);
            RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_HIGHLIGHTBOX), false);
            modelStack.PopMatrix();
        }
        else if (robot->GetState() == Robot::WORK_WITHPART && !pathfinder->IsPathEmpty())
        {
            Vector3 ornamentPos = pathfinder->foundPath.begin()->GetPosition() + Vector3(0, -1, 0);
            modelStack.PushMatrix();
            modelStack.Translate(ornamentPos.x, ornamentPos.y, 0.5f);
            RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_HIGHLIGHTBOX), false);
            modelStack.PopMatrix();
        }

    }

    //DELIVERY MAN
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Delivery Man")
    {
        DeliveryMan* deliveryman = dynamic_cast<DeliveryMan*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        // State
        std::string stateStr = "";
        switch (deliveryman->GetState())
        {
        case DeliveryMan::IDLE:
            stateStr = "Idle";
            break;

        case DeliveryMan::DRIVING:
            stateStr = "Driving";
            break;

        case DeliveryMan::WALK:
            stateStr = "Collecting";
            break;

        case DeliveryMan::COLLECT_PRODUCT:
            stateStr = "Collected";
            break;
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State: " + stateStr, Color(0, 0, 0), 2, 30, 0);

        Pathfinder* pathfinder = deliveryman->GetPathfinder();
        if (pathfinder)
        {
            if (!pathfinder->IsPathEmpty())
            {
                for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                {
                    modelStack.PushMatrix();
                    modelStack.Translate((*it).col, (*it).row, -0.5f);
                    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                    modelStack.PopMatrix();
                }
            }
        }   // end of pathfinder

    }

    //DELIVERY TRUCK - moving or not moving
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "DeliveryTruck")
    {
        DeliveryTruck* deliverytruck = dynamic_cast<DeliveryTruck*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        std::string isMoving = "";

        if (deliverytruck->IsMoving())
            isMoving = "Yes";
        else
            isMoving = "No";

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "Moving: " + isMoving, Color(0, 0, 0), 2, 30, 0);
    }

    //SCRAP MAN
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Scrap Man")
    {
        ScrapMan* scrapman = dynamic_cast<ScrapMan*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        if (scrapman->GetTempRole())
        {
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "Temp Role:" + scrapman->GetTempRole()->GetName(), Color(0, 0, 0), 2, 20, 0);

            Pathfinder* pathfinder = scrapman->GetTempRole()->GetPathfinder();
            if (pathfinder)
            {
                if (!pathfinder->IsPathEmpty())
                {
                    for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                    {
                        modelStack.PushMatrix();
                        modelStack.Translate((*it).col, (*it).row, -0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                        modelStack.PopMatrix();
                    }
                }
            }   // end of pathfinder

        }
        else
        {
            // State
            std::string stateStr = "";
            switch (scrapman->GetState())
            {
            case ScrapMan::IDLE:
                stateStr = "Idle";
                break;

            case ScrapMan::COLLECT_ROBOT:
                stateStr = "Work";
                break;

            case ScrapMan::BREAKDOWN_ROBOT:
                stateStr = "Work";
                break;

            case ScrapMan::BREAK:
                stateStr = "Break";
                break;

            case ScrapMan::OFFWORK:
                stateStr = "Off Work";
                break;
            }

            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State:" + stateStr, Color(0, 0, 0), 2, 20, 0);

            // Holding on to Robot
            //if (scrapman->GetState() == ScrapMan::COLLECT_ROBOT || scrapman->GetState() == ScrapMan::BREAKDOWN_ROBOT)   // working
            //{
            std::stringstream ss;

            ss << "Got Robot:";
            if (scrapman->GotRobot())
                ss << "Yes";
            else
                ss << "No";
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 36, 0);

            if (scrapman->IsBreakingRobot())
            {
                ss.str("");
                ss << "Breaking Robot...";
                RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 60, 0);
            }

            if (scrapman->GetState() == ScrapMan::IDLE)
            {
                // Break Charge
                ss.str("");
                ss << "Break: " << (int)scrapman->GetBreakCharge() << " / 2000   " << (int)scrapman->randNum << " > 50";
                RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 52, 0);
            }

            Pathfinder* pathfinder = scrapman->GetPathfinder();
            if (pathfinder)
            {
                if (!pathfinder->IsPathEmpty())
                {
                    for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                    {
                        modelStack.PushMatrix();
                        modelStack.Translate((*it).col, (*it).row, -0.5f);
                        RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                        modelStack.PopMatrix();
                    }
                }
            }   // end of pathfinder

        }

    }

    //SUPERVISOR
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Supervisor")
    {
        Supervisor* sup = dynamic_cast<Supervisor*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);
        
        // State
        std::string stateStr = "";
        switch (sup->GetState())
        {
        case Supervisor::IDLE:
            stateStr = "Idle";
            break;

        case Supervisor::PATROL:
            stateStr = "Patrol";
            break;

        case Supervisor::BREAK:
            stateStr = "Break";
            break;

        case Supervisor::MAKEDECISION:
            stateStr = "Thinking";
            break;

        case Supervisor::OFFWORK:
            stateStr = "Off Work";
            break;
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), "State:" + stateStr, Color(0, 0, 0), 2, 23, 0);

        if (sup->GetState() == Supervisor::IDLE)
        {
            // Break Charge
            ss.str("");
            ss << "Break: " << (int)sup->GetBreakCharge() << " / 2000   " << sup->randNum << " > 50";
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 52, 0);
        }
        else if (sup->GetState() == Supervisor::PATROL)
        {
            for (int i = sup->i_currWaypointIdx; i < sup->m_Waypoints.size(); ++i)
            {
                modelStack.PushMatrix();
                modelStack.Translate(sup->m_Waypoints[i].x, sup->m_Waypoints[i].y, -0.5f);
                RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_WAYPOINT_NODE), false);
                modelStack.PopMatrix();
            }
        }

        Pathfinder* pathfinder = sup->GetPathfinder();
        if (pathfinder)
        {
            if (!pathfinder->IsPathEmpty())
            {
                for (std::vector<Node>::iterator it = pathfinder->foundPath.begin(); it != pathfinder->foundPath.end(); ++it)
                {
                    modelStack.PushMatrix();
                    modelStack.Translate((*it).col, (*it).row, -0.5f);
                    RenderMesh(SharedData::GetInstance()->m_meshList->GetMesh(GEO_PATHFINDING_NODE), false);
                    modelStack.PopMatrix();
                }
            }
        }   // end of pathfinder
    }

    //BUILDING BLOCK STACK - selected or not, and if selected, the timer left
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Building Block Stack")
    {
        BuildingBlockStack* stack = dynamic_cast<BuildingBlockStack*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        std::string str = "Being Used: ";

        if (SharedData::GetInstance()->m_ornamentSystem->GetBuildingBlockCoord() == stack->GetPos())
        {
            str += "Yes";

            std::stringstream ss;
            ss << "Time Left: " << 20.0 - SharedData::GetInstance()->m_ornamentSystem->GetBuildingBlocksTimer();
            RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 55, 0);
        }
        else
        {
            str += "No";
        }

        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), str, Color(0, 0, 0), 2, 35, 0);
    }

    //ORNAMENT - selected or not, and if selected, the stage of completion and how many parts
    if (SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName() == "Ornament")
    {
        Ornament* ornament = dynamic_cast<Ornament*>(SharedData::GetInstance()->m_goList[index_renderDebugInfo]);

        std::stringstream ss;
        ss << "Parts: " << ornament->GetPartCount() << " / 11";
        if (ornament->isComplete())
            ss << " [COMPLETED]";
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), ss.str(), Color(0, 0, 0), 2, 25, 0);

        std::string str2 = "To Be Built: ";

        if (SharedData::GetInstance()->m_ornamentSystem->GetOrnamentCoord() == ornament->GetPos())
            str2 += "Yes";
        else
            str2 += "No";
        RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT), str2, Color(0, 0, 0), 2, 57, 0);
    }

    // name
    RenderTextOnScreen(SharedData::GetInstance()->m_meshList->GetMesh(GEO_TEXT),
        "Viewing: " + SharedData::GetInstance()->m_goList[index_renderDebugInfo]->GetName(),
        Color(0, 0, 0), 2, 0, 0);
}

void SceneAI::Exit()
{
	SceneBase::Exit();

    SharedData::GetInstance()->Exit();
}
