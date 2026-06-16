#ifndef PATIENT_H
#define PATIENT_H

#include "User.h"
#include "MedicalRecord.h"
#include "Prescription.h"
#include "Appointment.h"
#include "Payment.h"
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
    bool login(string& email, string& password, string& name);

    //Medical Records
    void viewMedicalRecord(MedicalRecord& record);

    //Prescriptions
    void viewPrescription(Prescription& prescription);


    //UC 2
    void bookAppointment(Database& db);
    //bool cancelAppointment();
    //Appointment* rescehduleAppointment();
    //Appointment* viewAppointment();

    //UC
    void makePayment(Appointment& appointment, Payment& payment, Database& db);

    
    // getters
    string getInsurance() {return insurance;}
    
    // setters
    void setInsurance(string& newInsurance) { insurance = newInsurance; }
};

#endif
