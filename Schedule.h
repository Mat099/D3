#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <string>

using namespace std;

class Schedule {
private:
    string slotId;
    string doctorId;
    string date;
    string timeSlot;
    bool available;

public:
    Schedule();

    Schedule(const string& slotId,
             const string& doctorId,
             const string& date,
             const string& timeSlot,
             bool available);

    // ───────── getters ─────────
    string getSlotId() const;
    string getDoctorId() const;
    string getDate() const;
    string getTimeSlot() const;
    bool isAvailable() const;

    // ───────── setters ─────────
    void setDate(const string& d);
    void setTimeSlot(const string& t);
    void setAvailable(bool a);

    // ───────── helpers ─────────
    void display() const;
};

#endif
