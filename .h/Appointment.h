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

public:
    // ─────────────────────────────────────────────
    // Constructor
    // ─────────────────────────────────────────────
    Appointment(const string& id,
                const string& patient,
                const string& doctor,
                const string& date,
                const string& time,
                const string& status = "pending");

    // ─────────────────────────────────────────────
    // UC 2 core behavior
    // ─────────────────────────────────────────────
    void confirm(Database& db);
    void cancel(Database& db);
    void reschedule(const string& newDate,
                     const string& newTime,
                     Database& db);

    // ─────────────────────────────────────────────
    // Display (UI style like your other classes)
    // ─────────────────────────────────────────────
    void display() const;

    // ─────────────────────────────────────────────
    // Getters (READ ONLY access)
    // ─────────────────────────────────────────────
    string getAppointmentId() const { return appointmentId; }
    string getPatientId()     const { return patientId; }
    string getDoctorId()      const { return doctorId; }
    string getDate()          const { return date; }
    string getTime()          const { return time; }
    string getStatus()        const { return status; }

    // ─────────────────────────────────────────────
    // Setters (ONLY for controlled updates)
    // ─────────────────────────────────────────────
    void setDate(const string& d)   { date = d; }
    void setTime(const string& t)   { time = t; }
    void setStatus(const string& s) { status = s; }
};

#endif