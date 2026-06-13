#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>
using namespace std;

class Appointment {
private:
    string appointmentId;
    string patientId;
    string doctorId;
    string date;
    string time;
    string status;

public:
    Appointment(string id, string patient,
                string doctor, string date,
                string time);

    void display();
};

#endif
