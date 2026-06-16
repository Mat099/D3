#include "Appointment.h"
#include "Database.h"
#include <iostream>

using namespace std;

Appointment::Appointment(const string& id,
                         const string& patient,
                         const string& doctor,
                         const string& date,
                         const string& time,
                         const string& status)
{
    this->appointmentId = id;
    this->patientId = patient;
    this->doctorId = doctor;
    this->date = date;
    this->time = time;
    this->status = status;
}

void Appointment::confirm(Database& db)
{
    status = "confirmed";

    db.updateAppointmentStatus(appointmentId, status, Database::currentTimestamp());

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Appointment confirmed successfully!             ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Appointment::cancel(Database& db)
{
    status = "cancelled";

    db.updateAppointmentStatus(appointmentId, status, Database::currentTimestamp());

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Appointment cancelled.                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Appointment::reschedule(const string& newDate,
                             const string& newTime,
                             Database& db)
{
    date = newDate;
    time = newTime;

    db.updateAppointmentSchedule(appointmentId, newDate, newTime,
                                Database::currentTimestamp());

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Appointment rescheduled successfully!           ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Appointment::display()
{
   cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Appointment                                     ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  ID:       " << appointmentId << endl;
    cout << "║  Patient:  " << patientId << endl;
    cout << "║  Doctor:   " << doctorId << endl;
    cout << "║  Date:     " << date << endl;
    cout << "║  Time:     " << time << endl;
    cout << "║  Status:   " << status << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}