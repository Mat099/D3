#ifndef NURSE_H
#define NURSE_H

#include "User.h"
#include "MedicalRecord.h"
#include "Prescription.h"
#include "Hospitalization.h"
#include <string>
using namespace std;

class Nurse : public User {
private:
    string workPass;

public:
    Nurse(const string& id, const string& name, const string& surname, const string& email, 
        const string& phoneNumber, const string& password);

    
    //UC1
    bool workLogin(string& workPass, string &password, string& name);

    //UC 12
    void changePassword(string& password);

    

    //UC
    void viewMedicalRecord(MedicalRecord& record);

    //UC
    //Triage* modTriage();

    //UC
    void viewHospitalization(Hospitalization& hospitalization);

    
    
};

#endif
