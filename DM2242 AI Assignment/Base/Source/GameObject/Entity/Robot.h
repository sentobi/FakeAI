#ifndef ROBOT_H
#define ROBOT_H

#include "Entity.h"
#include "../ConveyorBelt.h"

class Robot : public Entity
{
public:
    enum ROBOT_STATE
    {
        INCOMPLETE_1, // body //being built in progress
        INCOMPLETE_2, // body + limbs
        INCOMPLETE_3, // body + limbs + head

        STARTUP,
        WORK_WITHOUTPART,
        WORK_WITHPART,
        CHEER,
        SHUTDOWN,

        ROBOT_STATES_TOTAL
    };

private:
    double m_lifetime;
    double d_cheerCounter;
    
    ROBOT_STATE m_state;

	int m_currWaypoint;
	ConveyorBelt* m_beltToFollow;

    bool b_workedOn;
    bool b_ornamentCompleted;
    ROBOT_STATE m_stateBeforeCheer;

    virtual void Sense(double dt);
    virtual int Think();
    virtual void Act(int value);


public:
    Robot();
    virtual ~Robot();

	void SetBelt(ConveyorBelt* belt);
	void SetWaypoint(int idx);

    void SetWorkedOn(bool status);

    virtual void Init();
    virtual void Update(double dt);

    ROBOT_STATE GetState();
    void SetState(ROBOT_STATE state);
    virtual int GetStateInt();
    virtual int GetMaxStates();
};

#endif