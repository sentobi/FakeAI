#include "GL\glew.h"

#include "Entity.h"
#include "../../Utility.h"

#include "../../MessageBoard/MessageBoard.h"

Entity::Entity(std::string name) : GameObject(name, true), m_dir(DIR_DOWN), m_vel(0, 0, 0), b_MessageSent(false), b_newMsgNotif(false)
{
}

Entity::~Entity()
{
    if (m_mesh)
    {
        delete m_mesh;
        m_mesh = NULL;
    }
}

void Entity::RunFSM(double dt)
{
    Sense(dt);

    int thinkValue = Think();

    if (thinkValue != -1)
    {
        Act(thinkValue);
        //SetSprite();
    }
}

void Entity::SetVelocity(const Vector3& velocity)
{
    this->m_vel = velocity;
}

void Entity::SetDirection(DIRECTION dir)
{
    // if velocity == (1, 0, 0)
    // this->m_dir = DIR_RIGHT;

    this->m_dir = dir;
    this->SetSprite();
}

void Entity::SetSprite()
{
    if (this->m_mesh->textureID > 0)
    {
        float texCountWidth = m_dir;
        float texCountHeight = GetMaxStates() - 1 - GetStateInt();
        float texWidth = 1.f / 4;
        float texHeight = 1.f / GetMaxStates();

        TexCoord texCoords[4] = {
            TexCoord(texWidth * texCountWidth, texHeight * texCountHeight),
            TexCoord(texWidth * (texCountWidth + 1), texHeight * texCountHeight),
            TexCoord(texWidth * (texCountWidth + 1), texHeight * (texCountHeight + 1) - 0.01f),
            TexCoord(texWidth * texCountWidth, texHeight * (texCountHeight + 1) - 0.01f)
        };

        glBindBuffer(GL_ARRAY_BUFFER, m_mesh->vertexBuffer);
        for (unsigned i = 0; i < 4; ++i) {
            glBufferSubData(GL_ARRAY_BUFFER, (sizeof(Vertex)-sizeof(TexCoord)) + (i * sizeof(Vertex)), sizeof(TexCoord), &texCoords[i]);
        }
    }
}

int Entity::GetStateInt()
{
    return 0;
}

int Entity::GetMaxStates()
{
    return 1;
}

Vector3 Entity::CheckVelocity(const Vector3& ownPos, const Vector3& destinationPos)
{
    Vector3 vel = (destinationPos - ownPos);
    if (vel.IsZero())
        return vel;
    
    vel.Normalize();
    vel.x = SmallRoundOff(vel.x);
    vel.y = SmallRoundOff(vel.y);

    return vel;
}

Vector3 Entity::CheckVelocity(DIRECTION dir)
{
    if (dir == DIR_DOWN)
        return Vector3(0, -1, 0);

    if (dir == DIR_UP)
        return Vector3(0, 1, 0);

    if (dir == DIR_RIGHT)
        return Vector3(1, 0, 0);

    if (dir == DIR_LEFT)
        return Vector3(-1, 0, 0);
}

Entity::DIRECTION Entity::CheckDirection(const Vector3& velocity)
{
    if (m_vel.IsZero())     // not moving
        return m_dir;    // return previous direction

    if (m_vel.x == 1)
        return DIR_RIGHT;

    if (m_vel.x == -1)
        return DIR_LEFT;

    if (m_vel.y == 1)
        return DIR_UP;

    if (m_vel.y == -1)
        return DIR_DOWN;
}

Entity::DIRECTION Entity::CheckDirection(const Vector3& ownPos, const Vector3& toFacePos)
{
    Vector3 tempVel = CheckVelocity(ownPos, toFacePos);

    if (tempVel.IsZero())     // not moving
        return m_dir;    // return previous direction

    if (tempVel.x == 1)
        return DIR_RIGHT;

    if (tempVel.x == -1)
        return DIR_LEFT;

    if (tempVel.y == 1)
        return DIR_UP;

    if (tempVel.y == -1)
        return DIR_DOWN;
}

Message* Entity::ReadMessageBoard(MessageBoard* mb)
{
    b_newMsgNotif = false;     // messages will be processed now

    int maxSize = mb->GetMessageListSize();

    for (int i = maxSize - 1; i >= 0; --i)  // read the message log from oldest message
    {
        Message* msg = mb->GetAMessage(i);
        if (msg->GetMessageTo() == this->GetName() && !msg->IsAcknowledged())
        {
            msg->SetAcknowledged(true);
            return msg;
        }
    }

    return NULL;
}

void Entity::SetNewMessageNotif(bool b_notif)
{
    b_newMsgNotif = b_notif;
}