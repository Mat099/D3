#ifndef PATIENT_H
#define PATIENT_H

#include "User.h"
#include "MedicalRecord.h"
#include "Prescription.h"
#include <string>
using namespace std;

class Patient : public User {
private:
    string insurance;
    /*Appointment* appointments;
    Payment* payments;*/
    MedicalRecord* medicalRecord;

public:
    Patient(const string& id, const string& name, const string& surname, const string& email, 
        const string& phoneNumber, const string& insurance, const string& password);

    
    //UC1
    void login(string& email, string& password, string& name);

    //Medical Records
    void viewMedicalRecord(MedicalRecord& record);

    //Prescriptions
    void viewPrescription(Prescription& prescription);

    void makePayment(Payment& payment);

    /*UC 2
    Appointment* bookAppointment();
    bool cancelAppointment();
    Appointment* rescehduleAppointment();
    Appointment* viewAppointment();

    //UC
    Payment* makePayment();*/

    
    // getters
    string getInsurance() {return insurance;}
    
    // setters
    void setInsurance(string& newInsurance) { insurance = newInsurance; }
};

#endif
