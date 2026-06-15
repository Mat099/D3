#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <string>
using namespace std;

class Schedule {
private:
    string doctorId;
    string date;
    string timeSlot;
    bool available;

public:
    Schedule(string doctorId,
             string date,
             string timeSlot);

    bool isAvailable();
};

#endif