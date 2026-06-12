#ifndef HOSPITALIZATION_H
#define HOSPITALIZATION_H

#include <string>
using namespace std;

class Database;   // forward declaration — full type in .cpp

class Hospitalization {
private:
    // ── DB-aligned fields ───────────────────────────────────────────────────
    string hospitalizationId;
    string recordId;
    string patientId;
    string department;
    string hospitalName;
    string bedId;
    string admissionDate;
    string dischargeDate;
    bool   discharged;
    string doctorId;
    string lastModified;

    // ── Legacy field (not persisted) ─────────────────────────────────────────
    string reason;      // Doctor.cpp compat — no schema column for this

public:
    Hospitalization(const string& hospitalizationId, const string& recordId,
                    const string& patientId, const string& department,
                    const string& hospitalName, const string& bedId,
                    const string& admissionDate,
                    const string& dischargeDate = "",
                    bool discharged = false,
                    const string& doctorId = "",
                    const string& lastModified = "");

    // ── DB-aligned getters ───────────────────────────────────────────────────
    string getHospitalizationId() const { return hospitalizationId; }
    string getRecordId()          const { return recordId;          }
    string getPatientId()         const { return patientId;         }
    string getDepartment()        const { return department;        }
    string getHospitalName()      const { return hospitalName;      }
    string getBedId()             const { return bedId;             }
    string getAdmissionDate()     const { return admissionDate;     }
    string getDischargeDate()     const { return dischargeDate;     }
    bool   isDischarged()         const { return discharged;        }
    string getDoctorId()          const { return doctorId;          }
    string getLastModified()      const { return lastModified;      }

    // ── DB-aligned setters ────────────────────────────────────────────────────
    void setDepartment(const string& d)    { department    = d; }
    void setHospitalName(const string& h)  { hospitalName  = h; }
    void setBedId(const string& b)         { bedId         = b; }
    void setDischargeDate(const string& d) { dischargeDate = d; }
    void setDischarged(bool d)             { discharged    = d; }

    // ── Mutating methods — update in-memory AND persist via db ───────────────
    // Returns false if the regional constraint blocks the transfer.
    bool transfer(const string& newDepartment, const string& newHospital,
                  const string& newBedId, Database& db);
    void discharge(const string& date, Database& db);

    // ── Doctor.cpp compatibility aliases ────────────────────────────────────
    string getRoom()   const { return bedId;       }
    string getReason() const { return reason;      }
    bool   isActive()  const { return !discharged; }

    void setRoom(const string& r)   { bedId  = r; }
    void setReason(const string& r) { reason = r; }
    void setActive(bool a)          { discharged = !a; }
};

#endif
