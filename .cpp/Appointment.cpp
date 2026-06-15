#include "Appointment.h"
#include <iostream>

using namespace std;

Appointment::Appointment(string id,
                         string patient,
                         string doctor,
                         string date,
                         string time)
{
    appointmentId = id;
    patientId = patient;
    doctorId = doctor;
    this->date = date;
    this->time = time;
    status = "Booked";
}

void Appointment::display()
{
    cout << "Appointment ID: " << appointmentId << endl;
    cout << "Patient ID: " << patientId << endl;
    cout << "Doctor ID: " << doctorId << endl;
    cout << "Date: " << date << endl;
    cout << "Time: " << time << endl;
    cout << "Status: " << status << endl;
}