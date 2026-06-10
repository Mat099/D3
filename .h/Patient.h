#ifndef PATIENT_H
#define PATIENT_H

#include "User.h"
#include <string>
using namespace std;

class Patient: public User {
private:
    string insurance;
    string password;
    Appointment* appointments;
    Payment* payments;
    MedicalRecord* medicalRecord;  

public:
    Patient(string& id, string& name, string& surname, string& email, string& insurance, string& password);

    
    //UC1
    bool login(string& email, string &password);

    //UC 2
    Appointment* bookAppointment();
    bool cancelAppointment();
    Appointment* rescehduleAppointment();
    Appointment* viewAppointment();

    //UC
    MedicalRecord* viewMedicalRecords();

    //UC
    Payment* makePayment();

    
    // getters
    string getInsurance() {return insurance;}
    
    // setters
    void setInsurance(string& newInsurance) { insurance = newInsurance; }
};

#endif