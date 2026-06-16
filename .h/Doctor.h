#ifndef DOCTOR_H
#define DOCTOR_H

#include "User.h"
#include "MedicalRecord.h"
#include "Prescription.h"
#include "Hospitalization.h"
#include "Databse.h"
#include <string>
using namespace std;

class Database;

class Doctor : public User {
protected:
    string specialization;
    string workPass;

public:
    Doctor(const string& id, const string& name, const string& surname,
           const string& email, const string& phoneNumber,
           const string& specialization, const string& password);

    // UC1
    bool workLogin(string& workPass, string& password, string& name);

    // UC12
    void changePassword(string& password);

    // Medical Records
    void viewMedicalRecord(MedicalRecord& record);

    void viewPrescription(Prescription& prescription);

    // DB-aware methods
    MedicalRecord loadPatientRecord(Database& db);
    void updateMedicalRecord(MedicalRecord& record, Database& db);
    void issuePrescription(MedicalRecord& record, Database& db);
    void cancelPrescription(MedicalRecord& record, Database& db);
    void updatePrescriptionDosage(MedicalRecord& record, Database& db);
    void admitPatient(MedicalRecord& record, Database& db);
    void transferPatient(MedicalRecord& record, Database& db);
    void dischargePatient(MedicalRecord& record, Database& db);
    void manageAvailability(Database& db);

    // getters
    string getSpecialization() const { return specialization; }

    // setters
    void setSpecialization(const string& newSpec) { specialization = newSpec; }
};

#endif
