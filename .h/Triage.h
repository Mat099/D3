#ifndef TRIAGE_H
#define TRIAGE_H

#include <string>
#include <vector>
using namespace std;

class Database; // forward declaration

class Triage {
private:
    string triageId;
    string patientId;
    string hospitalName;
    string code;        // "Red" | "Orange" | "Azure" | "Green" | "White"
    string diagnosis;
    string admittedAt; // timestamp
    bool   discharged;
    string dischargedAt; // timestamp; empty while still in ER

public:
    // Default constructor — also the "not found" sentinel (empty triageId)
    Triage() : discharged(false) {}

    Triage(const string& triageId,
           const string& patientId,
           const string& hospitalName,
           const string& code,
           const string& diagnosis,
           const string& admittedAt,
           bool discharged = false,
           const string& dischargedAt = "");

    // getters
    string getTriageId() const;
    string getPatientId() const;
    string getHospitalName() const;
    string getCode() const;
    string getDiagnosis() const;
    string getAdmittedAt() const;
    bool   isDischarged() const;
    string getDischargedAt() const;

    // setters
    void setCode(const string& newCode);
    void setDischarged(const string& timestamp);

    // UC 8 — triage update
    void admitPatient     (Database& db); // admits a new patient, assigns the first code
    void updateTriageCode (Database& db); // changes the code of an existing ER patient
    void dischargeFromER  (Database& db); // removes a patient from the ER queue
    void viewStatus       (Database& db); // shows the current triage record for a patient
 
    // UC 3 — E.R. line check
    void checkERCrowding  (Database& db); // displays the number of active ER patients at a hospital
};

#endif