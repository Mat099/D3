#include "MedicalRecord.h"
#include "Database.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// ── Helper ───────────────────────────────────────────────────────────────────

static vector<string> splitStr(const string& s, const string& delim) {
    vector<string> out;
    size_t start = 0, pos;
    while ((pos = s.find(delim, start)) != string::npos) {
        string tok = s.substr(start, pos - start);
        if (!tok.empty()) out.push_back(tok);
        start = pos + delim.size();
    }
    if (start < s.size()) out.push_back(s.substr(start));
    return out;
}

// ── Constructors ─────────────────────────────────────────────────────────────

MedicalRecord::MedicalRecord()
    : recordId(""), patientId(""), lastModified("") {}

MedicalRecord::MedicalRecord(const string& recordId,
                              const string& patientId,
                              const string& allergiesStr,
                              const string& medicalHistoryStr,
                              const string& lastModified)
    : recordId(recordId), patientId(patientId), lastModified(lastModified) {
    for (const string& a : splitStr(allergiesStr, ", "))
        allergies.push_back(a);

    // Split medical history on ". " sentence boundaries
    for (const string& h : splitStr(medicalHistoryStr, ". ")) {
        string entry = h;
        if (!entry.empty() && entry.back() == '.') entry.pop_back();
        if (!entry.empty()) medicalHistory.push_back(entry);
    }
}

// ── Loaders (called by Database::loadMedicalRecord) ──────────────────────────

void MedicalRecord::addAllergy(const string& allergy) {
    allergies.push_back(allergy);
}

void MedicalRecord::addMedicalHistoryEntry(const string& entry) {
    medicalHistory.push_back(entry);
}

void MedicalRecord::addPrescriptionToRecord(const Prescription& p) {
    prescriptions.push_back(p);
}

void MedicalRecord::addHospitalizationToRecord(const Hospitalization& h) {
    hospitalizations.push_back(h);
}

// ── Prescription mutations ────────────────────────────────────────────────────

void MedicalRecord::addPrescription(const Prescription& p, Database& db) {
    db.insertPrescription(p);
    prescriptions.push_back(p);
    string ts = Database::currentTimestamp();
    lastModified = ts;
    db.updateMedicalRecordTimestamp(recordId, ts);
}

bool MedicalRecord::cancelPrescription(const string& prescriptionId,
                                        Database& db) {
    for (Prescription& p : prescriptions) {
        if (p.getPrescriptionId() == prescriptionId) {
            p.cancel(db);
            string ts = Database::currentTimestamp();
            lastModified = ts;
            db.updateMedicalRecordTimestamp(recordId, ts);
            return true;
        }
    }
    return false;
}

bool MedicalRecord::updatePrescriptionDosage(const string& prescriptionId,
                                              const string& newDosage,
                                              Database& db) {
    for (Prescription& p : prescriptions) {
        if (p.getPrescriptionId() == prescriptionId) {
            p.updateDosage(newDosage, db);
            string ts = Database::currentTimestamp();
            lastModified = ts;
            db.updateMedicalRecordTimestamp(recordId, ts);
            return true;
        }
    }
    return false;
}

// ── Hospitalization mutations ─────────────────────────────────────────────────

void MedicalRecord::addHospitalization(const Hospitalization& h, Database& db) {
    db.insertHospitalization(h);
    hospitalizations.push_back(h);
    string ts = Database::currentTimestamp();
    lastModified = ts;
    db.updateMedicalRecordTimestamp(recordId, ts);
}

bool MedicalRecord::transferPatient(const string& hospitalizationId,
                                     const string& newDepartment,
                                     const string& newHospital,
                                     const string& newBedId,
                                     Database& db) {
    for (Hospitalization& h : hospitalizations) {
        if (h.getHospitalizationId() == hospitalizationId) {
            if (!h.transfer(newDepartment, newHospital, newBedId, db))
                return false;
            string ts = Database::currentTimestamp();
            lastModified = ts;
            db.updateMedicalRecordTimestamp(recordId, ts);
            return true;
        }
    }
    return false;
}

bool MedicalRecord::dischargePatient(const string& hospitalizationId,
                                      const string& date,
                                      Database& db) {
    for (Hospitalization& h : hospitalizations) {
        if (h.getHospitalizationId() == hospitalizationId) {
            h.discharge(date, db);
            string ts = Database::currentTimestamp();
            lastModified = ts;
            db.updateMedicalRecordTimestamp(recordId, ts);
            return true;
        }
    }
    return false;
}

// ── UC 10: display ────────────────────────────────────────────────────────────

void MedicalRecord::display() const {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Medical Record                                  ║" << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  Patient ID: " << patientId << endl;
    cout << "║  Record ID:  " << recordId  << endl;

    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  Allergies                                       ║" << endl;
    if (allergies.empty()) {
        cout << "║    None known                                    ║" << endl;
    } else {
        for (const string& a : allergies)
            cout << "║    - " << a << endl;
    }

    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  Medical History                                 ║" << endl;
    if (medicalHistory.empty()) {
        cout << "║    No history recorded                           ║" << endl;
    } else {
        for (const string& h : medicalHistory)
            cout << "║    - " << h << endl;
    }

    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  Prescriptions                                   ║" << endl;
    if (prescriptions.empty()) {
        cout << "║    No prescriptions on record                    ║" << endl;
    } else {
        for (const Prescription& p : prescriptions) {
            string tag = p.isActive() ? "ACTIVE   " : "CANCELLED";
            cout << "║    [" << tag << "] "
                 << p.getMedicineName() << " — " << p.getDosage() << endl;
            if (!p.getDescription().empty())
                cout << "║             " << p.getDescription() << endl;
        }
    }

    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  Hospitalizations                                ║" << endl;
    if (hospitalizations.empty()) {
        cout << "║    No hospitalization history                    ║" << endl;
    } else {
        for (const Hospitalization& h : hospitalizations) {
            cout << "║    " << h.getDepartment() << " @ "
                 << h.getHospitalName()
                 << "  Bed: " << h.getBedId() << endl;
            cout << "║      Admitted:  " << h.getAdmissionDate() << endl;
            if (h.isDischarged())
                cout << "║      Discharged: " << h.getDischargeDate() << endl;
            else
                cout << "║      Status:    currently admitted" << endl;
        }
    }

    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

// ── Doctor.cpp compatibility ──────────────────────────────────────────────────

string MedicalRecord::getDiagnosis() const {
    string out;
    for (size_t i = 0; i < medicalHistory.size(); ++i) {
        if (i > 0) out += ". ";
        out += medicalHistory[i];
    }
    return out;
}

string MedicalRecord::getNotes() const {
    string out;
    for (size_t i = 0; i < allergies.size(); ++i) {
        if (i > 0) out += ", ";
        out += allergies[i];
    }
    return out;
}

void MedicalRecord::setDiagnosis(const string& d) {
    medicalHistory.clear();
    if (!d.empty()) medicalHistory.push_back(d);
}

void MedicalRecord::setNotes(const string& n) {
    allergies = splitStr(n, ", ");
}
