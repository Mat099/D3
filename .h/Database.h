#ifndef DATABASE_H
#define DATABASE_H

#include "MedicalRecord.h"
#include <string>
#include "sqlite3.h"
using namespace std;

class Database {
private:
    sqlite3* db;

    bool execSQL(const string& sql);
    bool execFile(const string& filePath);

public:
    Database();
    ~Database();

    bool open(const string& dbPath);
    bool close();
    bool execSchema(const string& schemaPath);
    bool execSeed(const string& seedPath);

    // ── Utilities ────────────────────────────────────────────────────────────
    static string currentTimestamp();
    string generateNextId(const string& table, const string& idColumn,
                           const string& prefix);

    // ── Medical Records ──────────────────────────────────────────────────────
    MedicalRecord loadMedicalRecord(const string& patientId);
    bool updateMedicalRecordTimestamp(const string& recordId,
                                       const string& timestamp);

    // ── Prescriptions ────────────────────────────────────────────────────────
    bool insertPrescription(const Prescription& p);
    bool cancelPrescription(const string& prescriptionId,
                             const string& timestamp);
    bool updatePrescriptionDosage(const string& prescriptionId,
                                   const string& newDosage,
                                   const string& timestamp);

    // ── Hospitalizations ─────────────────────────────────────────────────────
    bool insertHospitalization(const Hospitalization& h);
    bool dischargeHospitalization(const string& hospitalizationId,
                                    const string& date,
                                    const string& timestamp);
    bool updateHospitalizationTransfer(const string& hospitalizationId,
                                        const string& department,
                                        const string& hospital,
                                        const string& bedId,
                                        const string& timestamp);
    bool insertTransferLog(const string& logId,
                            const string& hospitalizationId,
                            const string& fromDept, const string& fromHosp,
                            const string& fromBed,
                            const string& toDept,   const string& toHosp,
                            const string& toBed,
                            const string& transferredAt);

    // ── Hospital region lookup (for transfer constraint) ─────────────────────
    string getHospitalRegion(const string& hospitalName);
};

#endif
