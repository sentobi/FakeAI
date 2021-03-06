#include "MaintenanceMan.h"

#include "../../SharedData.h"

MaintenanceMan::MaintenanceMan() : Entity("Maintenance Man")
{
}

MaintenanceMan::~MaintenanceMan()
{
    if (m_pathfinder)
        delete m_pathfinder;
}

void MaintenanceMan::Init()
{
    m_pathfinder = new Pathfinder();
    b_reachedDestination = false;

    m_breakCharge = Math::RandFloatMinMax(-1000, 1000);
    m_timer = 0;
    m_state = IDLE;
    m_targetMachine = NULL;
    m_doingWork = false;
    m_breakDone = false;
    m_inToilet = false;
    m_doOnce = false;
    m_toiletIdx = -1;
    m_shouldMoveForward = true;
    m_toilet = NULL;

    randNum = 0;
}

void MaintenanceMan::SetPos(Vector3 pos)
{
    m_pos = pos;
    m_origSpawn = m_pos;
}

void MaintenanceMan::Update(double dt)
{
    // Update message notification
    if (b_newMsgNotif)
    {
        UpdateMessageNotif(dt);
    }
    if (b_renderAcknowledgeMsg)
    {
        UpdateMessageAcknowledged(dt);
    }

    // Update inactive level
    bool UntargetedMachines = false;
    for (int i = 0; i < SharedData::GetInstance()->m_goList.size(); ++i)
    {
        if (!SharedData::GetInstance()->m_goList[i]->IsEntity())
            continue;

        Entity* checkEntity = dynamic_cast<Entity*>(SharedData::GetInstance()->m_goList[i]);

        if (checkEntity->GetName() == "Machine")
        {
            if (!dynamic_cast<Machine*>(checkEntity)->GetIsBeingWorkedOn())
            {
                UntargetedMachines = true;
                break;
            }
        }
        
    }

    if ((m_state == IDLE || m_state == BREAK) && !UntargetedMachines)
    {
        d_inactive_level += dt;
        d_inactive_level = Math::Min(10.0, d_inactive_level);
    }
    else
    {
        d_inactive_level -= dt;
        d_inactive_level = Math::Max(0.0, d_inactive_level);
    }

    if (tempRole)
    {
        tempRole->Update(dt);
        m_pos = tempRole->GetPos();
        SetDirection(tempRole->GetDirection());
        //m_dir = tempRole->GetDirection();
        return;
    }

    if (m_state == BREAK)
    {
        // Check if toilet is close, if so add to queue and walk to it
        if ((m_pos - m_toilet->GetPos()).Length() < 4)  // within toilet range
        {
            if (!m_doOnce && m_state == BREAK)
            {
                m_toiletIdx = m_toilet->AddToQueue(this);
                //std::cout << "ADDED" << std::endl;
                m_doOnce = true;
            }

            Vector3 dir = (m_toilet->GetQueuePosition(m_toiletIdx) - m_pos);
            if (!dir.IsZero())
                dir.Normalize();

            m_pos += dir * dt;
        }
        else
        {
            //pathfind to toilet
            m_pos += m_vel * f_walkSpeed * dt;
            if (m_pathfinder->hasReachedNode(this->m_pos))
            {
                // reached destination; can get a part and move on.
                if (m_pathfinder->hasReachedDestination(this->m_pos))
                {
                    WhenReachedDestination();
                }
                else
                {
                    WhenReachedPathNode();
                }
            }

            //Vector3 dir = (m_toilet->GetPos() - m_pos).Normalized();
            //m_pos += dir * dt;
        }

        if ((m_pos - m_toilet->GetPos()).Length() < 0.1)
        {
            m_inToilet = true;
            m_toilet->SetOccupied(true);
        }
        else
            m_inToilet = false;

    }
    else if (m_targetMachine)   // there is work to do
    {
        if (!m_doingWork)
        {
            // Move Towards TargetMachine, then do work
            //Vector3 dir = (m_targetMachine->GetPos() - m_pos).Normalized();
            //m_pos += dir * dt;

            // follow found path to TargetMachine
            m_pos += m_vel * f_walkSpeed * dt;
            if (m_pathfinder->hasReachedNode(this->m_pos))
            {
                // reached destination; can get a part and move on.
                if (m_pathfinder->hasReachedDestination(this->m_pos))
                {
                    WhenReachedDestination();
                    m_doingWork = true;
                    //SetDirection(CheckDirection(this->m_pos, m_targetMachine->GetPos()));
                }
                else
                {
                    WhenReachedPathNode();
                }
            }

            if ((m_pos - m_targetMachine->GetPos()).Length() < 1.001)
            {
                m_doingWork = true;
                SetDirection(CheckDirection(this->m_pos, m_targetMachine->GetPos()));
            }
        }
    }
    else if (m_state == OFFWORK)
    {
        if (!m_pathfinder->IsPathEmpty())
        {
            // Walk to door
            m_pos += m_vel * f_walkSpeed * dt;
            if (m_pathfinder->hasReachedNode(this->m_pos))
            {
                // check if reached destination; 
                if (m_pathfinder->hasReachedDestination(this->m_pos))
                {
                    // reached
                    WhenReachedDestination();
                }
                else
                {
                    WhenReachedPathNode();
                }
            }
        }
    }
    else // idle state
    {
        // Move to workstation if no TargetMachine
        if ((m_origSpawn - m_pos).Length() > 0.1)
        {
            // pathfind to workstation
            m_pos += m_vel * f_walkSpeed * dt;
            if (m_pathfinder->hasReachedNode(this->m_pos))
            {
                // reached destination; can get a part and move on.
                if (m_pathfinder->hasReachedDestination(this->m_pos))
                {
                    WhenReachedDestination();
                }
                else
                {
                    WhenReachedPathNode();
                }
            }

            //Vector3 dir = (m_origSpawn - m_pos);
            //
            //if (!dir.IsZero())
            //    dir.Normalize();
            //
            //m_pos += dir * dt;

            if ((m_pos - m_origSpawn).Length() < 0.1)
            {
                
            }
        }
        else
        {
            b_reachedDestination = true;
        }
    }

    
    switch (m_state)
    {
    case IDLE:
        DoIdle();
        break;

    case REPAIR:
        DoRepair();
        break;

    case REFILL:
        DoRefill();
        break;

    case BREAK:
        DoBreak();
        break;

    case OFFWORK:
        DoOffWork();
        break;
    }
}

void MaintenanceMan::Sense(double dt)
{
    if ((m_state == REFILL || m_state == REPAIR) && m_doingWork)
        m_timer += dt;
    else if (m_state == BREAK && m_inToilet)
        m_timer += dt;
    else if (m_state == IDLE)
    {
        m_timer += dt;
        if (m_timer > 1)
        {
            m_timer = 0;
            m_breakCharge += Math::RandFloatMinMax(0, 120);
        }
    }
}

int MaintenanceMan::Think()
{
    switch (m_state)
    {

    case IDLE:
    {
        if (!SharedData::GetInstance()->m_clock->GetIsWorkDay() && !SharedData::GetInstance()->m_clock->GetIsWorkStarted())
            return OFFWORK;

        Vector3 temp = m_workstation->GetPos();
        //temp.y -= 1;

        // Check if at workstation
        if ((m_pos - temp).Length() < 1.2)
        {
            // Read Messages
            if (b_newMsgNotif && d_msgNotifTimer >= 2.0)
            {
                Message* retrievedMsg = this->ReadMessageBoard(SharedData::GetInstance()->m_messageBoard);

                // Check if retrieved message is invalid
                if (retrievedMsg)
                {
                    AcknowledgeMessage();

                    switch (retrievedMsg->GetMessageType())
                    {
                    case Message::MACHINE_BROKEN:
                        m_targetMachine = dynamic_cast<Machine*>(retrievedMsg->GetMessageFromObject());
                        m_targetMachine->SetIsBeingWorkedOn(true);
                        return REPAIR;

                    case Message::MACHINE_REFILL:
                        m_targetMachine = dynamic_cast<Machine*>(retrievedMsg->GetMessageFromObject());
                        m_targetMachine->SetIsBeingWorkedOn(true);
                        return REFILL;

                    case Message::INCREASE_URGENCY:
                        if (!b_urgencyChanged)
                        {
                            i_currUrgencyLevel++;
                            b_urgencyChanged = true;
                        }
                        break;

                    case Message::DECREASE_URGENCY:
                        if (!b_urgencyChanged)
                        {
                            i_currUrgencyLevel--;
                            i_currUrgencyLevel = Math::Max(0, i_currUrgencyLevel);
                            b_urgencyChanged = true;
                        }
                        break;

                    case Message::COMPLETED_URGENCY_CHANGE:
                        b_urgencyChanged = false;
                        break;

                    case Message::ENTITY_ROLECHANGE:
                        SetTempRole(retrievedMsg->GetMessageSubject());
                        break;
                    }

                    // Update walk speed if needed
                    f_walkSpeed = 1 + i_currUrgencyLevel * 0.25;
                }
            }
        }

        if (m_breakCharge >= 2000)
        {
            randNum = Math::RandIntMinMax(0, 100);
            if (randNum < 50)
            {
                m_breakCharge = 0;
            }
            else
            {
                m_doOnce = false;
                return BREAK;
            }
        }

        //// Check if at workstation
        //if ((m_pos - temp).Length() < 1.2)
        //{
        //    int value = ScanMachines();
        //    if (value != IDLE)
        //        return value;
        //}

        break;
    }

    case REFILL:
        if (m_targetMachine && !m_targetMachine->IsEmpty())
        {
            m_targetMachine->SetIsBeingWorkedOn(false);
            m_targetMachine = NULL;
            m_doingWork = false;
            return IDLE;
        }
        //else if (m_targetMachine && m_targetMachine->IsEmpty())
        //{
        //    return REFILL;
        //}
        break;

    case REPAIR:
        if (m_targetMachine && !m_targetMachine->IsBroken())
        {
            m_targetMachine->SetIsBeingWorkedOn(false);
            m_targetMachine = NULL;
            m_doingWork = false;
            return IDLE;
        }
        //else if (m_targetMachine && m_targetMachine->IsBroken())
        //{
        //    return REPAIR;
        //}
        break;

    case BREAK:
        if (m_breakDone)
        {
            m_breakDone = false;
            m_breakCharge = 0;
            return IDLE;
        }
        else
        {
            m_targetMachine = NULL;
            m_doingWork = false;
        }
        break;

    case OFFWORK:
        if (this->IsOnLeave())
            break;

        if (!Entity::IsSomeoneOnLeave())
        {
            // run the probability codes
            float rand = Math::RandFloatMinMax(0, 100);
            if (rand < 0.01f) {
                this->SetOnLeave(true);
                Entity::SetSomeoneOnLeave(true);
                break;
            }
        }

        if (SharedData::GetInstance()->m_clock->GetIsWorkDay())
        {
            float randNum = Math::RandFloatMinMax(0, 100);
            if (randNum < 0.1 || SharedData::GetInstance()->m_clock->GetIsWorkStarted())
            {
                m_breakCharge = 0;
                return IDLE;
            }
        }
        break;

    }

    return -1;
}

void MaintenanceMan::Act(int value)
{
    switch (value)
    {
    case IDLE:
        SetState(IDLE);
        b_reachedDestination = false;
        //DoIdle();

        // pathfind to workstation
        m_pathfinder->EmptyPath();
        m_pathfinder->ReceiveCurrentPos(Vector3(RoundOff(m_pos.x), RoundOff(m_pos.y), m_pos.z));
        m_pathfinder->ReceiveDestination(m_origSpawn);
        m_pathfinder->FindPathGreedyBestFirst();

        SetVelocity(CheckVelocity(m_pos, m_pathfinder->foundPath.back().GetPosition()));
        SetDirection(CheckDirection(m_vel));
        m_pathfinder->ReceiveDirection(m_dir);
        break;

    case REPAIR:
        b_doneTempJob = true;

        SetState(REPAIR);
        b_reachedDestination = false;
        //DoRepair();

        // pathfind to target machine
        m_pathfinder->EmptyPath();
        m_pathfinder->ReceiveCurrentPos(Vector3(RoundOff(m_pos.x), RoundOff(m_pos.y), m_pos.z));
        m_pathfinder->ReceiveDestination(m_targetMachine->GetPos());
        m_pathfinder->FindPathGreedyBestFirst();

        SetVelocity(CheckVelocity(m_pos, m_pathfinder->foundPath.back().GetPosition()));
        SetDirection(CheckDirection(m_vel));
        m_pathfinder->ReceiveDirection(m_dir);
        break;

    case REFILL:
        b_doneTempJob = true;

        SetState(REFILL);
        b_reachedDestination = false;
        //DoRefill();

        // pathfind to target machine
        m_pathfinder->EmptyPath();
        m_pathfinder->ReceiveCurrentPos(Vector3(RoundOff(m_pos.x), RoundOff(m_pos.y), m_pos.z));
        m_pathfinder->ReceiveDestination(m_targetMachine->GetPos());
        m_pathfinder->FindPathGreedyBestFirst();

        SetVelocity(CheckVelocity(m_pos, m_pathfinder->foundPath.back().GetPosition()));
        SetDirection(CheckDirection(m_vel));
        m_pathfinder->ReceiveDirection(m_dir);
        break;

    case BREAK:
        SetState(BREAK);
        b_reachedDestination = false;
        //DoBreak();

        // pathfind to toilet
        m_pathfinder->EmptyPath();
        m_pathfinder->ReceiveCurrentPos(Vector3(RoundOff(m_pos.x), RoundOff(m_pos.y), m_pos.z));
        m_pathfinder->ReceiveDestination(m_toilet->GetPos());
        m_pathfinder->FindPathGreedyBestFirst();

        SetVelocity(CheckVelocity(m_pos, m_pathfinder->foundPath.back().GetPosition()));
        SetDirection(CheckDirection(m_vel));
        m_pathfinder->ReceiveDirection(m_dir);
        break;

    case MAINTENANCEMAN_STATES_TOTAL:   // going back to workstation from toilet
        SetState(IDLE);
        b_reachedDestination = false;
        //DoIdle();

        // pathfind to workstation
        m_pathfinder->EmptyPath();
        m_pathfinder->ReceiveCurrentPos(Vector3(14, 14, m_pos.z));
        m_pathfinder->ReceiveDestination(m_origSpawn);
        m_pathfinder->FindPathGreedyBestFirst();

        SetVelocity(CheckVelocity(m_pos, m_pathfinder->foundPath.back().GetPosition()));
        SetDirection(CheckDirection(m_vel));
        m_pathfinder->ReceiveDirection(m_dir);
        break;

    case OFFWORK:
    {
        SetState(OFFWORK);
        // pathfind to door
        m_pathfinder->EmptyPath();
        m_pathfinder->ReceiveCurrentPos(Vector3(RoundOff(m_pos.x), RoundOff(m_pos.y), m_pos.z));

        GameObject* door;
        for (auto i : SharedData::GetInstance()->m_goList)
        {
            if (i->GetName() == "Door")
                door = i;
        }

        m_pathfinder->ReceiveDestination(door->GetPos());
        m_pathfinder->FindPathGreedyBestFirst();

        SetVelocity(CheckVelocity(m_pos, m_pathfinder->foundPath.back().GetPosition()));
        SetDirection(CheckDirection(m_vel));
        m_pathfinder->ReceiveDirection(m_dir);

        d_inactive_level = 0;
        i_currUrgencyLevel = 1;
        f_walkSpeed = 1;
        b_urgencyChanged = false;
        break;
    }
    }
}

int MaintenanceMan::ScanMachines()
{
    for (int i = 0; i < SharedData::GetInstance()->m_goList.size(); ++i)
    {
        GameObject* go = dynamic_cast<GameObject*>(SharedData::GetInstance()->m_goList[i]);

        if (!go->IsActive())
            continue;

        if (go->GetName() == "Machine")
        {
            Machine* machine = dynamic_cast<Machine*>(go);

            if (machine->GetIsBeingWorkedOn())
                continue;

            if (machine->GetState() == Machine::BROKEN)
            {
                m_targetMachine = machine;
                m_targetMachine->SetIsBeingWorkedOn(true);
                return REPAIR;
            }
            else if (machine->GetState() == Machine::WAITFORREFILL)
            {
                m_targetMachine = machine;
                m_targetMachine->SetIsBeingWorkedOn(true);
                return REFILL;
            }
        }
    }

    return IDLE;
}

void MaintenanceMan::DoIdle()
{

}

void MaintenanceMan::DoRepair()
{       
    if (m_timer > 4)
    {
        m_targetMachine->SetIsBroken(false);
        m_timer = 0;
    }
}

void MaintenanceMan::DoRefill()
{
    if (m_timer > 4)
    {
        m_targetMachine->SetIsEmpty(false);
        m_targetMachine->Refill();
        m_timer = 0;
    }
}

void MaintenanceMan::DoBreak()
{
    if (m_toilet->CheckIfChange())
    {
        if (m_shouldMoveForward)
        {
            m_toiletIdx = Math::Max(--m_toiletIdx, 0);
            m_shouldMoveForward = false;
        }
    }
    else
    {
        m_shouldMoveForward = true;
    }

    if (m_timer > 4)
    {
        m_breakCharge = 0;
        m_breakDone = true;
        m_timer = 0;
        m_toilet->RemoveFromQueue();
        //std::cout << "POPPED" << std::endl;
 
        m_toilet->SetOccupied(false);
    }
}

void MaintenanceMan::DoOffWork()
{
}

MaintenanceMan::MAINTENANCEMAN_STATE MaintenanceMan::GetState()
{
    return m_state;
}

void MaintenanceMan::SetState(MAINTENANCEMAN_STATE state)
{
    this->m_state = state;
    this->SetSprite();
}

int MaintenanceMan::GetStateInt()
{
    return m_state;
}

int MaintenanceMan::GetMaxStates()
{
    return MAINTENANCEMAN_STATES_TOTAL;
}

void MaintenanceMan::SetWorkstation(Workstation* station)
{
    m_workstation = station;
}

Workstation* MaintenanceMan::GetWorkstation()
{
    return m_workstation;
}

void MaintenanceMan::SetOriginalSpawn(const Vector3& origSpawn)
{
    m_origSpawn = origSpawn;
}

Vector3 MaintenanceMan::GetOriginalSpawn()
{
    return m_origSpawn;
}

void MaintenanceMan::SetToilet(Toilet* toilet)
{
    m_toilet = toilet;
}

Toilet* MaintenanceMan::GetToilet()
{
    return m_toilet;
}

double MaintenanceMan::GetBreakCharge()
{
    return m_breakCharge;
}

//Pathfinder* MaintenanceMan::GetPathfinder()
//{
//    return m_pathfinder;
//}

Machine* MaintenanceMan::GetTargetMachine()
{
    return m_targetMachine;
}