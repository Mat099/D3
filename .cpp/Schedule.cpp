#include "Schedule.h"
#include <iostream>

using namespace std;

Schedule::Schedule(const string& slotId,
                   const string& doctorId,
                   const string& date,
                   const string& timeSlot,
                   bool available)
{
    this->slotId = slotId;
    this->doctorId = doctorId;
    this->date = date;
    this->timeSlot = timeSlot;
    this->available = available;
}

string Schedule::getSlotId() const { return slotId; }
string Schedule::getDoctorId() const { return doctorId; }
string Schedule::getDate() const { return date; }
string Schedule::getTimeSlot() const { return timeSlot; }
bool Schedule::isAvailable() const { return available; }


void Schedule::setDate(const string& d)
{
    date = d;
}

void Schedule::setTimeSlot(const string& t)
{
    timeSlot = t;
}

void Schedule::setAvailable(bool a)
{
    available = a;
}

void Schedule::display() const
{
    cout << "Slot: " << slotId
         << " | Doctor: " << doctorId
         << " | Date: " << date
         << " | Time: " << timeSlot
         << " | Available: " << (available ? "YES" : "NO")
         << endl;
}