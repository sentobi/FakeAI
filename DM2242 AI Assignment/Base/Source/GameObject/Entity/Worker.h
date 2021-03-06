#ifndef WORKER_H
#define WORKER_H

#include "Entity.h"
#include "../Workstation.h"
#include "../Toilet.h"

class Worker : public Entity
{
public:
    enum WORKER_STATE
    {
        IDLE,
        WORK,
        BREAK,
        OFFWORK,
        WORKER_STATES_TOTAL
    };

private:

    double m_breakCharge;
    bool m_workCompleted;
    bool m_inToilet;
    bool m_breakDone;
    bool m_atWorkstation;
    bool m_doOnce;
    bool m_shouldMoveForward;
    int m_toiletIdx;
    Vector3 m_origSpawn; 

    Workstation* m_workstation;
    Toilet* m_toilet;

    //Pathfinder* m_pathfinder;
    //bool b_reachedDestination;
    
    WORKER_STATE m_state;

    void DoIdle();
    void DoWork();
    void DoBreak();
    void DoOffWork();

    virtual void Sense(double dt);
    virtual int Think();
    virtual void Act(int value);

public:
    Worker();
    virtual ~Worker();

    bool IsPartAtWorkstation();
    bool IsAbleToWork();
    bool IsOnBreak();

    void SetWorkstation(Workstation* station);
    Workstation* GetWorkstation();

    void SetOriginalSpawn(const Vector3& origSpawn);
    Vector3 GetOriginalSpawn();

    virtual void Init();
    virtual void Update(double dt);
    virtual void SetPos(Vector3 pos);

    WORKER_STATE GetState();
    void SetState(WORKER_STATE state);
    virtual int GetStateInt();
    virtual int GetMaxStates();

    void SetToilet(Toilet* toilet);
    Toilet* GetToilet();

    double m_timer;

    // Getters
    double GetBreakCharge();

    // DEBUG UI
    int randNum;
};

#endif