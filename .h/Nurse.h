#ifndef NURSE_H
#define NURSE_H

#include "User.h"
#include <string>
using namespace std;

class Nurse : public User {
private:
    string workPass;
    /*Schedule* schedule;
    Triage* triage; 
    Hospitalization* hospitalization; etc*/

public:
    Nurse(const string& id, const string& name, const string& surname, const string& email, 
        const string& phoneNumber, const string& password);

    
    //UC1
    void workLogin(string& workPass, string &password, string& name);

    //UC 12
    void changePassword(string& password);

    /*

    //UC
    MedicalRecord* seeMedicalRecords();

    //UC
    Triage* modTriage();

    //UC
    Hospitalization* seeHospitalization();
    
    etc*/
};

#endif