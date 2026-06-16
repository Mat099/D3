#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>
#include <iostream>

using namespace std;

class Database;
class Appointment {
private:
    string appointmentId;
    string patientId;
    string doctorId;
    string date;
    string time;
    string status; //pending, comfirmed, cancelled or competed 
    bool paid;

public:
    Appointment(const string& id,
                const string& patient,
                const string& doctor,
                const string& date,
                const string& time,
                const string& status = "pending");

    void confirm(Database& db);
    void cancel(Database& db);
    void reschedule(const string& newDate,
                     const string& newTime,
                     Database& db);

    void markPaid(Database& db);

    void display();

    string getAppointmentId() const { return appointmentId; }
    string getPatientId()     const { return patientId; }
    string getDoctorId()      const { return doctorId; }
    string getDate()          const { return date; }
    string getTime()          const { return time; }
    string getStatus()        const { return status; }
    bool isPaid()             const { return paid; }

    void setDate(const string& d)   { date = d; }
    void setTime(const string& t)   { time = t; }
    void setStatus(const string& s) { status = s; }
    void setPaid(bool p) { paid = p; }
};

#endif
