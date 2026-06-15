#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include "Prescription.h"
#include "Hospitalization.h"
#include <string>
#include <vector>
using namespace std;

class Database;   // forward declaration — full type in .cpp

class MedicalRecord {
private:
    string                   recordId;
    string                   patientId;
    vector<string>           allergies;
    vector<string>           medicalHistory;
    vector<Prescription>     prescriptions;
    vector<Hospitalization>  hospitalizations;
    string                   lastModified;

public:
    MedicalRecord();   // default-constructed = "not found" sentinel
    MedicalRecord(const string& recordId, const string& patientId,
                  const string& allergiesStr, const string& medicalHistoryStr,
                  const string& lastModified = "");

    // ── Getters ──────────────────────────────────────────────────────────────
    string                          getRecordId()          const { return recordId;         }
    string                          getPatientId()         const { return patientId;        }
    const vector<string>&           getAllergies()          const { return allergies;        }
    const vector<string>&           getMedicalHistory()    const { return medicalHistory;   }
    const vector<Prescription>&     getPrescriptions()     const { return prescriptions;    }
    const vector<Hospitalization>&  getHospitalizations()  const { return hospitalizations; }
    string                          getLastModified()      const { return lastModified;     }

    // ── Called by Database::loadMedicalRecord to populate in-memory state ────
    void addAllergy(const string& allergy);
    void addMedicalHistoryEntry(const string& entry);
    void addPrescriptionToRecord(const Prescription& p);
    void addHospitalizationToRecord(const Hospitalization& h);

    // ── Prescription mutations — update in-memory AND persist via db ─────────
    void addPrescription(const Prescription& p, Database& db);
    bool cancelPrescription(const string& prescriptionId, Database& db);
    bool updatePrescriptionDosage(const string& prescriptionId,
                                   const string& newDosage, Database& db);

    // ── Hospitalization mutations — update in-memory AND persist via db ──────
    void addHospitalization(const Hospitalization& h, Database& db);
    bool transferPatient(const string& hospitalizationId,
                          const string& newDepartment, const string& newHospital,
                          const string& newBedId, Database& db);
    bool dischargePatient(const string& hospitalizationId,
                           const string& date, Database& db);

    // ── UC 10: display to console ────────────────────────────────────────────
    void display() const;

    // ── Doctor.cpp compatibility aliases (in-memory only, no DB write) ───────
    string getDate()      const { return lastModified; }
    string getDiagnosis() const;   // medicalHistory joined by ". "
    string getNotes()     const;   // allergies joined by ", "
    void   setDiagnosis(const string& d);
    void   setNotes(const string& n);
};

#endif
