#include "Schedule.h"
#include <iostream>
#include <string>
using namespace std;

Schedule :: Schedule(string doctorId,
                   string date,
                   string timeSlot)
{
    this->doctorId = doctorId;
    this->date = date;
    this->timeSlot = timeSlot;
    available = true;
}

bool Schedule::isAvailable()
{
    return available;
}