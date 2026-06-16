#include "Triage.h"
#include "Database.h"
#include <iostream>
#include <string>
using namespace std;
 
// Triage codes (UC 8 / UC 3)
// Italian triage colour system: Red > Orange > Azure > Green > White
 
// Constructor
 
Triage::Triage(const string& triageId,
               const string& patientId,
               const string& hospitalName,
               const string& code,
               const string& admittedAt)
    : triageId(triageId), patientId(patientId), hospitalName(hospitalName),
      code(code), admittedAt(admittedAt),
      discharged(false), dischargedAt("") {}
 
// Getters 
 
string Triage::getTriageId()    const { return triageId;    }
string Triage::getPatientId()   const { return patientId;   }
string Triage::getHospitalName()const { return hospitalName;}
string Triage::getCode()        const { return code;        }
string Triage::getAdmittedAt()  const { return admittedAt;  }
bool   Triage::isDischarged()   const { return discharged;  }
string Triage::getDischargedAt()const { return dischargedAt;}
 
// Setters
 
void Triage::setCode(const string& newCode)          { code = newCode;         }
void Triage::setDischarged(const string& timestamp)  {
    discharged   = true;
    dischargedAt = timestamp;
}
 
// UC 8: admit a new patient to the ER
 
void Triage::admitPatient(Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  E.R. Admission                                  ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Patient ID:                                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    string pId;
    cin >> pId;
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Hospital name:                                  ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    string hosp;
    cin >> hosp;
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Triage code:                                    ║" << endl;
    cout << "║  1. Red    (immediately life-threatening)        ║" << endl;
    cout << "║  2. Orange (urgent)                              ║" << endl;
    cout << "║  3. Azure  (semi-urgent)                         ║" << endl;
    cout << "║  4. Green  (minorly urgent)                      ║" << endl;
    cout << "║  5. White  (not urgent)                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    int codeChoice;
    cin >> codeChoice;
 
    string triageCode;
    switch (codeChoice) {
        case 1: triageCode = "Red";    break;
        case 2: triageCode = "Orange"; break;
        case 3: triageCode = "Azure";  break;
        case 4: triageCode = "Green";  break;
        case 5: triageCode = "White";  break;
        default:
            cout << "╔═════════════════════════════════════════════════╗" << endl;
            cout << "║  Invalid code. Admission cancelled.             ║" << endl;
            cout << "╚═════════════════════════════════════════════════╝" << endl;
            return;
    }
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Confirm admission? (y/n):                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Admission cancelled.                           ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    string ts    = Database::currentTimestamp();
    string newId = db.generateNextId("triage", "triage_id", "TRG");
 
    if (!db.insertTriageRecord(newId, pId, hosp, triageCode, diag, ts)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  System error: admission could not be saved.    ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    // Update this object's state to reflect the newly admitted patient
  
    triageId    = newId;
    patientId   = pId;
    hospitalName= hosp;
    code        = triageCode;
    admittedAt  = ts;
    discharged  = false;
    dischargedAt= "";
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Patient admitted to E.R.                        ║" << endl;
    cout << "║  Triage ID: " << newId                            << endl;
    cout << "║  Code:      " << triageCode                       << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}
 
// UC 8: change the triage code of an existing ER patient
 
void Triage::updateTriageCode(Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Update Triage Code                              ║" << endl;
    cout << "║  Triage ID:                                      ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    string tId;
    cin >> tId;
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  New triage code:                                ║" << endl;
    cout << "║  1. Red    (immediately life-threatening)        ║" << endl;
    cout << "║  2. Orange (urgent)                              ║" << endl;
    cout << "║  3. Azure  (semi-urgent)                         ║" << endl;
    cout << "║  4. Green  (minorly urgent)                      ║" << endl;
    cout << "║  5. White  (not urgent)                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    int codeChoice;
    cin >> codeChoice;
 
    string newCode;
    switch (codeChoice) {
        case 1: newCode = "Red";    break;
        case 2: newCode = "Orange"; break;
        case 3: newCode = "Azure";  break;
        case 4: newCode = "Green";  break;
        case 5: newCode = "White";  break;
        default:
            cout << "╔═════════════════════════════════════════════════╗" << endl;
            cout << "║  Invalid code. Update cancelled.                ║" << endl;
            cout << "╚═════════════════════════════════════════════════╝" << endl;
            return;
    }
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Confirm update? (y/n):                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Update cancelled.                              ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    string ts = Database::currentTimestamp();
    if (!db.updateTriageCode(tId, newCode, ts)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Triage record not found or already discharged. ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    // Keep in-object state consistent if this object represents that record
  
    if (triageId == tId) {
        code = newCode;
    }
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Triage code updated to: " << newCode             << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}
 
// UC 8: discharge a patient from the ER
/* A patient leaves the ER either because they are sent home or because they
are admitted for full hospitalization (handled separately in UC 7).*/
 
void Triage::dischargeFromER(Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Discharge from E.R.                             ║" << endl;
    cout << "║  Triage ID:                                      ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    string tId;
    cin >> tId;
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Confirm discharge? (y/n):                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Discharge cancelled.                           ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    string ts = Database::currentTimestamp();
    if (!db.dischargeTriageRecord(tId, ts)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Triage record not found or already discharged. ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    if (triageId == tId) {
        discharged   = true;
        dischargedAt = ts;
    }
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Patient discharged from E.R.                    ║" << endl;
    cout << "║  At: " << ts                                      << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}
 
// UC 8: view triage status of a specific patient
 
void Triage::viewStatus(Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Check Triage Status                             ║" << endl;
    cout << "║  Triage ID:                                      ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    string tId;
    cin >> tId;
 
    Triage record = db.loadTriageRecord(tId);
    if (record.getTriageId().empty()) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  No triage record found for that ID.            ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Triage Record                                   ║" << endl;
    cout << "║  Triage ID:   " << record.getTriageId()          << endl;
    cout << "║  Patient ID:  " << record.getPatientId()         << endl;
    cout << "║  Hospital:    " << record.getHospitalName()      << endl;
    cout << "║  Code:        " << record.getCode()              << endl;
    cout << "║  Diagnosis:   " << record.getDiagnosis()         << endl;
    cout << "║  Admitted at: " << record.getAdmittedAt()        << endl;
    if (record.isDischarged())
        cout << "║  Discharged:  " << record.getDischargedAt() << endl;
    else
    cout << "║  Status:      currently in E.R.                  ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}
 
// UC 3: ER line check (shows the number of active ER patients)
// Available to any user (including anonymous)
 
void Triage::checkERCrowding(Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  E.R. Line Check                                 ║" << endl;
    cout << "║  Select a hospital:                              ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    string hosp;
    cin >> hosp;
 
    vector<Triage> patients = db.loadActiveTriageByHospital(hosp);
 
    if (patients.empty()) {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  No patients currently in E.R. at:               ║" << endl;
        cout << "║  " << hosp                                        << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }
    
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  E.R. patients at: " << hosp                      << endl;
    cout << "║  Total: " << patients.size()                       << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

}
