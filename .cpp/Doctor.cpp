#include "User.h"
#include "Doctor.h"
#include "Database.h"
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;


Doctor::Doctor(const string& id, 
                const string& name, 
                const string& surname, 
                const string& email, 
                const string& phoneNumber, 
                const string& specialization, 
                const string& password)
    : User(id, name, surname, email, phoneNumber, password), specialization(specialization) {
    //appointments(nullptr), medicalRecord(nullptr) etc
}

void Doctor::workLogin(string& workPass, string &password, string& name) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Please enter your work credentials:             ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  workPass:                                       ║" << endl;
    cout << "║  Password:                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string credential1, credential2;
    cin >> credential1;
    cin >> credential2;

    if (credential1 != workPass || credential2 != password) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid credentials. Please try again.         ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }
              
    else{
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Login successful! Welcome, " << name << "!     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }
}

void Doctor::changePassword(string& password){
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Please enter your new password. It must be at   ║" << endl;
    cout << "║  least 10 characters long, using  at  least      ║" << endl;
    cout << "║  one uppercase character, one lowercase          ║" << endl;
    cout << "║  character, one digit, and one special character ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  New password:                                   ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newPassword;
    cin >> newPassword;

    if (newPassword.length() < 10) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must be at least 10 characters long.  ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::isupper)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one uppercase   ║" << endl;
        cout << "║  character.                                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::islower)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one lowercase   ║" << endl;
        cout << "║  character.                                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::isdigit)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one digit.      ║" << endl;
        cout << "║                                                 ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::ispunct)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one special     ║" << endl;
        cout << "║  character.                                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else {
        password = newPassword;
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password changed successfully!                 ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }

}

void Doctor::viewMedicalRecord(MedicalRecord& record) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Medical Record                                  ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Record ID:  " << record.getRecordId()  << endl;
    cout << "║  Patient ID: " << record.getPatientId() << endl;
    cout << "║  Date:       " << record.getDate()      << endl;
    cout << "║  Diagnosis:  " << record.getDiagnosis() << endl;
    cout << "║  Notes:      " << record.getNotes()     << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::updateMedicalRecord(MedicalRecord& record) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Update Medical Record                           ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Current diagnosis: " << record.getDiagnosis()  << endl;
    cout << "║  New diagnosis:                                  ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newDiagnosis;
    cin >> newDiagnosis;
    record.setDiagnosis(newDiagnosis);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Current notes: " << record.getNotes()           << endl;
    cout << "║  New notes:                                      ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newNotes;
    cin >> newNotes;
    record.setNotes(newNotes);

    cout << "╔═════════════════════════════════════════════════╗" << endl;
    cout << "║  Medical record updated successfully.           ║" << endl;
    cout << "╚═════════════════════════════════════════════════╝" << endl;
}

void Doctor::issuePrescription(Prescription& prescription) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Issue Prescription                              ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Medication:                                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string medication;
    cin >> medication;
    prescription.setMedication(medication);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Dosage (e.g. 500mg):                            ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string dosage;
    cin >> dosage;
    prescription.setDosage(dosage);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Frequency (e.g. twice_daily):                   ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string frequency;
    cin >> frequency;
    prescription.setFrequency(frequency);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Duration (e.g. 7_days):                         ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string duration;
    cin >> duration;
    prescription.setDuration(duration);

    cout << "╔═════════════════════════════════════════════════╗" << endl;
    cout << "║  Prescription issued successfully.              ║" << endl;
    cout << "╚═════════════════════════════════════════════════╝" << endl;
}

void Doctor::viewPrescription(Prescription& prescription) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Prescription                                    ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  ID:         " << prescription.getPrescriptionId() << endl;
    cout << "║  Patient ID: " << prescription.getPatientId()      << endl;
    cout << "║  Medication: " << prescription.getMedication()     << endl;
    cout << "║  Dosage:     " << prescription.getDosage()         << endl;
    cout << "║  Frequency:  " << prescription.getFrequency()      << endl;
    cout << "║  Duration:   " << prescription.getDuration()       << endl;
    cout << "║  Issued:     " << prescription.getIssueDate()      << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::admitPatient(Hospitalization& hospitalization) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Admit Patient                                   ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Room number:                                    ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string room;
    cin >> room;
    hospitalization.setRoom(room);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Reason for admission:                           ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string reason;
    cin >> reason;
    hospitalization.setReason(reason);

    hospitalization.setActive(true);

    cout << "╔═════════════════════════════════════════════════╗" << endl;
    cout << "║  Patient admitted successfully.                 ║" << endl;
    cout << "╚═════════════════════════════════════════════════╝" << endl;
}

void Doctor::dischargePatient(Hospitalization& hospitalization) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Discharge Patient                               ║" << endl;
    cout << "║  Patient ID: " << hospitalization.getPatientId()   << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Confirm discharge? (y/n):                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    char choice;
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Enter discharge date (YYYY-MM-DD):              ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;

        string dischargeDate;
        cin >> dischargeDate;
        hospitalization.setDischargeDate(dischargeDate);
        hospitalization.setActive(false);

        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Patient discharged successfully.               ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }
    else {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Discharge cancelled.                           ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }
}

void Doctor::viewHospitalization(Hospitalization& hospitalization) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Hospitalization Record                          ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  ID:        " << hospitalization.getHospitalizationId() << endl;
    cout << "║  Patient:   " << hospitalization.getPatientId()         << endl;
    cout << "║  Room:      " << hospitalization.getRoom()               << endl;
    cout << "║  Reason:    " << hospitalization.getReason()             << endl;
    cout << "║  Admitted:  " << hospitalization.getAdmissionDate()      << endl;
    cout << "║  Status:    " << (hospitalization.isActive() ? "Active" : "Discharged on " + hospitalization.getDischargeDate()) << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

// ── DB-aware methods ─────────────────────────────────────────────────────────

MedicalRecord Doctor::loadPatientRecord(Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Load Patient Record                             ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Patient ID:                                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string patientId;
    cin >> patientId;

    MedicalRecord rec = db.loadMedicalRecord(patientId);
    if (rec.getRecordId().empty()) {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  No record found for: " << patientId              << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return MedicalRecord();
    }
    rec.display();
    return rec;
}

void Doctor::issuePrescription(MedicalRecord& record, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Issue Prescription                              ║" << endl;
    cout << "║  Record: " << record.getRecordId()               << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Medicine name:                                  ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string medicine;
    cin >> medicine;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Dosage (e.g. 500mg_once_daily):                 ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string dosage;
    cin >> dosage;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Description:                                    ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string description;
    cin >> description;

    string newId = db.generateNextId("prescriptions", "prescription_id", "PRX");
    Prescription p(newId, record.getRecordId(), medicine, dosage, description,
                   true, getId(), Database::currentTimestamp());
    record.addPrescription(p, db);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Prescription issued: " << newId                 << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::cancelPrescription(MedicalRecord& record, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Cancel Prescription                             ║" << endl;
    cout << "║  Active prescriptions:                           ║" << endl;
    for (const Prescription& p : record.getPrescriptions()) {
        if (p.isActive())
            cout << "║    " << p.getPrescriptionId() << "  " << p.getMedicineName() << endl;
    }
    cout << "║                                                  ║" << endl;
    cout << "║  Prescription ID to cancel:                      ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string prescriptionId;
    cin >> prescriptionId;

    bool ok = record.cancelPrescription(prescriptionId, db);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    if (ok)
        cout << "║  Prescription cancelled: " << prescriptionId     << endl;
    else
        cout << "║  Not found or already inactive.               ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::updatePrescriptionDosage(MedicalRecord& record, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Update Prescription Dosage                      ║" << endl;
    cout << "║  Active prescriptions:                           ║" << endl;
    for (const Prescription& p : record.getPrescriptions()) {
        if (p.isActive())
            cout << "║    " << p.getPrescriptionId() << "  "
                 << p.getMedicineName() << " — " << p.getDosage() << endl;
    }
    cout << "║                                                  ║" << endl;
    cout << "║  Prescription ID:                                ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string prescriptionId;
    cin >> prescriptionId;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  New dosage:                                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newDosage;
    cin >> newDosage;

    bool ok = record.updatePrescriptionDosage(prescriptionId, newDosage, db);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    if (ok)
        cout << "║  Dosage updated for: " << prescriptionId          << endl;
    else
        cout << "║  Prescription not found or inactive.          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::admitPatient(MedicalRecord& record, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Admit Patient                                   ║" << endl;
    cout << "║  Patient: " << record.getPatientId()             << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Department:                                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string department;
    cin >> department;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Hospital name:                                  ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string hospitalName;
    cin >> hospitalName;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Bed ID (e.g. BED-301):                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string bedId;
    cin >> bedId;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Admission date (YYYY-MM-DD):                    ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string admissionDate;
    cin >> admissionDate;

    string newId = db.generateNextId("hospitalizations", "hospitalization_id", "HSP");
    Hospitalization h(newId, record.getRecordId(), record.getPatientId(),
                      department, hospitalName, bedId, admissionDate,
                      "", false, getId(), Database::currentTimestamp());
    record.addHospitalization(h, db);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Patient admitted: " << newId                    << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::transferPatient(MedicalRecord& record, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Transfer Patient                                ║" << endl;
    cout << "║  Active hospitalizations:                        ║" << endl;
    for (const Hospitalization& h : record.getHospitalizations()) {
        if (h.isActive())
            cout << "║    " << h.getHospitalizationId() << "  "
                 << h.getDepartment() << " @ " << h.getHospitalName() << endl;
    }
    cout << "║                                                  ║" << endl;
    cout << "║  Hospitalization ID:                             ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string hospitalizationId;
    cin >> hospitalizationId;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  New department:                                 ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newDept;
    cin >> newDept;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  New hospital name:                              ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newHospital;
    cin >> newHospital;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  New bed ID:                                     ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newBed;
    cin >> newBed;

    bool ok = record.transferPatient(hospitalizationId, newDept, newHospital, newBed, db);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    if (ok)
        cout << "║  Transfer completed to: " << newHospital          << endl;
    else {
        cout << "║  Transfer blocked: hospitals are in different ║" << endl;
        cout << "║  regions. Only same-region transfers allowed. ║" << endl;
    }
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Doctor::dischargePatient(MedicalRecord& record, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Discharge Patient                               ║" << endl;
    cout << "║  Active hospitalizations:                        ║" << endl;
    for (const Hospitalization& h : record.getHospitalizations()) {
        if (h.isActive())
            cout << "║    " << h.getHospitalizationId() << "  "
                 << h.getHospitalName() << " — " << h.getBedId() << endl;
    }
    cout << "║                                                  ║" << endl;
    cout << "║  Hospitalization ID to discharge:                ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string hospitalizationId;
    cin >> hospitalizationId;

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Discharge date (YYYY-MM-DD):                    ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string dischargeDate;
    cin >> dischargeDate;

    bool ok = record.dischargePatient(hospitalizationId, dischargeDate, db);

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    if (ok)
        cout << "║  Patient discharged on: " << dischargeDate        << endl;
    else {
        cout << "║  Hospitalization not found or already         ║" << endl;
        cout << "║  discharged.                                  ║" << endl;
    }
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}