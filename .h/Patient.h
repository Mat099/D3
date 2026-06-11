#ifndef PATIENT_H
#define PATIENT_H

#include "User.h"
#include <string>
using namespace std;

class Patient : public User {
private:
    string insurance;
    /*Appointment* appointments;
    Payment* payments;
    MedicalRecord* medicalRecord;  */

public:
    Patient(const string& id, const string& name, const string& surname, const string& email, 
        const string& phoneNumber, const string& insurance, const string& password);

    
    //UC1
    bool login(string& email, string& password, string& name);

    /*UC 2
    Appointment* bookAppointment();
    bool cancelAppointment();
    Appointment* rescehduleAppointment();
    Appointment* viewAppointment();

    //UC
    MedicalRecord* viewMedicalRecords();

    //UC
    Payment* makePayment();*/

    
    // getters
    string getInsurance() {return insurance;}
    
    // setters
    void setInsurance(string& newInsurance) { insurance = newInsurance; }
};

#endif
