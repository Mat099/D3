#ifndef PRESCRIPTION_H
#define PRESCRIPTION_H

#include <string>
using namespace std;

class Database;   // forward declaration — full type in .cpp

class Prescription {
private:
    // ── DB-aligned fields ───────────────────────────────────────────────────
    string prescriptionId;
    string recordId;
    string medicineName;
    string dosage;
    string description;
    bool   active;
    string doctorId;
    string createdAt;

    // ── Legacy / convenience fields (not persisted) ─────────────────────────
    string patientId;   // set by caller for display; not in prescriptions table
    string frequency;   // Doctor.cpp compat — maps to free-text description
    string duration;    // Doctor.cpp compat — maps to free-text description

public:
    Prescription(const string& prescriptionId, const string& recordId,
                 const string& medicineName, const string& dosage,
                 const string& description, bool active,
                 const string& doctorId, const string& createdAt);

    // ── DB-aligned getters ───────────────────────────────────────────────────
    string getPrescriptionId() const { return prescriptionId; }
    string getRecordId()       const { return recordId;       }
    string getMedicineName()   const { return medicineName;   }
    string getDosage()         const { return dosage;         }
    string getDescription()    const { return description;    }
    bool   isActive()          const { return active;         }
    string getDoctorId()       const { return doctorId;       }
    string getCreatedAt()      const { return createdAt;      }

    // ── DB-aligned setters (in-memory only — use Database methods to persist) ─
    void setMedicineName(const string& n) { medicineName = n; }
    void setDosage(const string& d)       { dosage       = d; }
    void setDescription(const string& d)  { description  = d; }
    void setActive(bool a)                { active       = a; }

    // ── Mutating methods — update in-memory AND persist via db ───────────────
    void updateDosage(const string& newDosage, Database& db);
    void cancel(Database& db);

    // ── Doctor.cpp compatibility aliases ────────────────────────────────────
    string getMedication() const { return medicineName; }
    string getPatientId()  const { return patientId;    }
    string getFrequency()  const { return frequency;    }
    string getDuration()   const { return duration;     }
    string getIssueDate()  const { return createdAt;    }

    void setMedication(const string& m) { medicineName = m; }
    void setPatientId(const string& p)  { patientId   = p;  }
    void setFrequency(const string& f)  { frequency   = f;  }
    void setDuration(const string& d)   { duration    = d;  }
};

#endif
