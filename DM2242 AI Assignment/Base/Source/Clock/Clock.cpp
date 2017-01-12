#include "Clock.h"

Clock::Clock(Time time) : m_time(time)
{
}

Clock::~Clock()
{
}

std::string Clock::GetCurrDay()
{
    return m_time.GetDay();
}

std::string Clock::GetCurrDayAbbreviation()
{
    return m_time.GetDayAbbreviation();
}

std::string Clock::GetCurrTime()
{
    return m_time.GetTime();
}

void Clock::Update(const double dt)
{
    m_time.Update(dt);
}