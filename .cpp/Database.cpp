#include "Database.h"
#include "MedicalRecord.h"
#include "Prescription.h"
#include "Hospitalization.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>
#include <string>
using namespace std;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Safely read a TEXT column; returns "" when the column is NULL.
static inline string col(sqlite3_stmt* s, int i) {
    const unsigned char* v = sqlite3_column_text(s, i);
    return v ? reinterpret_cast<const char*>(v) : "";
}

// ── Construction / teardown ───────────────────────────────────────────────────

Database::Database() : db(nullptr) {}

Database::~Database() { close(); }

bool Database::open(const string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    return true;
}

bool Database::close() {
    if (db) { sqlite3_close(db); db = nullptr; }
    return true;
}

// ── Internal SQL helpers ──────────────────────────────────────────────────────

bool Database::execSQL(const string& sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << err << endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool Database::execFile(const string& filePath) {
    ifstream f(filePath);
    if (!f.is_open()) { cerr << "Cannot open file: " << filePath << endl; return false; }
    ostringstream ss; ss << f.rdbuf();
    return execSQL(ss.str());
}

bool Database::execSchema(const string& schemaPath) { return execFile(schemaPath); }
bool Database::execSeed(const string& seedPath)     { return execFile(seedPath);   }

// ── Utilities ─────────────────────────────────────────────────────────────────

string Database::currentTimestamp() {
    time_t now = time(nullptr);
    char buf[17];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&now));
    return string(buf);
}

string Database::generateNextId(const string& table, const string& idColumn,
                                  const string& prefix) {
    // All IDs use the format PREFIX-NNNN; SUBSTR(id, 5) extracts the number.
    string sql = "SELECT COALESCE(MAX(CAST(SUBSTR(" + idColumn +
                 ", 5) AS INTEGER)), 0) + 1 FROM " + table + ";";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return prefix + "-0001";
    int n = 1;
    if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    char buf[8];
    snprintf(buf, sizeof(buf), "%04d", n);
    return prefix + "-" + buf;
}

// ── Medical Records ───────────────────────────────────────────────────────────

MedicalRecord Database::loadMedicalRecord(const string& patientId) {
    sqlite3_stmt* stmt;

    // 1. Load the record row
    const char* recSql =
        "SELECT record_id, patient_id, allergies, medical_history, last_modified "
        "FROM medical_records WHERE patient_id = ?;";
    if (sqlite3_prepare_v2(db, recSql, -1, &stmt, nullptr) != SQLITE_OK)
        return MedicalRecord();

    sqlite3_bind_text(stmt, 1, patientId.c_str(), -1, SQLITE_STATIC);

    MedicalRecord record;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        record = MedicalRecord(col(stmt, 0), col(stmt, 1),
                               col(stmt, 2), col(stmt, 3), col(stmt, 4));
    } else {
        sqlite3_finalize(stmt);
        return MedicalRecord();
    }
    sqlite3_finalize(stmt);

    const string& recordId = record.getRecordId();

    // 2. Load prescriptions for this record
    const char* rxSql =
        "SELECT prescription_id, record_id, medicine_name, dosage, description, "
        "       active, doctor_id, created_at "
        "FROM prescriptions WHERE record_id = ?;";
    if (sqlite3_prepare_v2(db, rxSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, recordId.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            record.addPrescriptionToRecord(
                Prescription(col(stmt,0), col(stmt,1), col(stmt,2),
                             col(stmt,3), col(stmt,4),
                             sqlite3_column_int(stmt, 5) != 0,
                             col(stmt,6), col(stmt,7)));
        }
        sqlite3_finalize(stmt);
    }

    // 3. Load hospitalizations for this record
    const char* hospSql =
        "SELECT hospitalization_id, record_id, patient_id, department, hospital_name, "
        "       bed_id, admission_date, discharge_date, discharged, doctor_id, last_modified "
        "FROM hospitalizations WHERE record_id = ?;";
    if (sqlite3_prepare_v2(db, hospSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, recordId.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            record.addHospitalizationToRecord(
                Hospitalization(col(stmt,0), col(stmt,1), col(stmt,2),
                                col(stmt,3), col(stmt,4), col(stmt,5),
                                col(stmt,6), col(stmt,7),
                                sqlite3_column_int(stmt, 8) != 0,
                                col(stmt,9), col(stmt,10)));
        }
        sqlite3_finalize(stmt);
    }

    return record;
}

bool Database::updateMedicalRecordTimestamp(const string& recordId,
                                             const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE medical_records SET last_modified = ? WHERE record_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, recordId.c_str(),  -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateMedicalRecordFields(const string& recordId,
                                          const string& allergies,
                                          const string& medicalHistory,
                                          const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE medical_records "
        "SET allergies = ?, medical_history = ?, last_modified = ? "
        "WHERE record_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, allergies.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, medicalHistory.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, timestamp.c_str(),      -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, recordId.c_str(),       -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// ── Prescriptions ─────────────────────────────────────────────────────────────

bool Database::insertPrescription(const Prescription& p) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO prescriptions "
        "(prescription_id, record_id, medicine_name, dosage, description, "
        " active, doctor_id, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, p.getPrescriptionId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, p.getRecordId().c_str(),       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, p.getMedicineName().c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, p.getDosage().c_str(),         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, p.getDescription().c_str(),    -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 6, p.isActive() ? 1 : 0);
    sqlite3_bind_text(stmt, 7, p.getDoctorId().c_str(),       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, p.getCreatedAt().c_str(),      -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::cancelPrescription(const string& prescriptionId,
                                    const string& /*timestamp*/) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE prescriptions SET active = 0 WHERE prescription_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, prescriptionId.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updatePrescriptionDosage(const string& prescriptionId,
                                         const string& newDosage,
                                         const string& /*timestamp*/) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE prescriptions SET dosage = ? WHERE prescription_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, newDosage.c_str(),       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, prescriptionId.c_str(),  -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// ── Hospitalizations ──────────────────────────────────────────────────────────

bool Database::insertHospitalization(const Hospitalization& h) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO hospitalizations "
        "(hospitalization_id, record_id, patient_id, department, hospital_name, "
        " bed_id, admission_date, discharge_date, discharged, doctor_id, last_modified) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt,  1, h.getHospitalizationId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  2, h.getRecordId().c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  3, h.getPatientId().c_str(),         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  4, h.getDepartment().c_str(),        -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  5, h.getHospitalName().c_str(),      -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  6, h.getBedId().c_str(),             -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  7, h.getAdmissionDate().c_str(),     -1, SQLITE_STATIC);
    if (h.getDischargeDate().empty())
        sqlite3_bind_null(stmt, 8);
    else
        sqlite3_bind_text(stmt, 8, h.getDischargeDate().c_str(),  -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt,  9, h.isDischarged() ? 1 : 0);
    sqlite3_bind_text(stmt, 10, h.getDoctorId().c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, h.getLastModified().c_str(),      -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::dischargeHospitalization(const string& hospitalizationId,
                                         const string& date,
                                         const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE hospitalizations "
        "SET discharge_date = ?, discharged = 1, last_modified = ? "
        "WHERE hospitalization_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, date.c_str(),              -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(),         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hospitalizationId.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateHospitalizationTransfer(const string& hospitalizationId,
                                              const string& department,
                                              const string& hospital,
                                              const string& bedId,
                                              const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE hospitalizations "
        "SET department = ?, hospital_name = ?, bed_id = ?, last_modified = ? "
        "WHERE hospitalization_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, department.c_str(),        -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hospital.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, bedId.c_str(),             -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(),         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, hospitalizationId.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::insertTransferLog(const string& logId,
                                  const string& hospitalizationId,
                                  const string& fromDept, const string& fromHosp,
                                  const string& fromBed,
                                  const string& toDept,   const string& toHosp,
                                  const string& toBed,
                                  const string& transferredAt) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO transfer_log "
        "(log_id, hospitalization_id, from_department, from_hospital, from_bed_id, "
        " to_department, to_hospital, to_bed_id, transferred_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, logId.c_str(),             -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hospitalizationId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, fromDept.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, fromHosp.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, fromBed.c_str(),           -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, toDept.c_str(),            -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, toHosp.c_str(),            -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, toBed.c_str(),             -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, transferredAt.c_str(),     -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// ── Hospital region lookup ────────────────────────────────────────────────────

string Database::getHospitalRegion(const string& hospitalName) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT region FROM hospitals WHERE hospital_name = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return "";
    sqlite3_bind_text(stmt, 1, hospitalName.c_str(), -1, SQLITE_STATIC);
    string region;
    if (sqlite3_step(stmt) == SQLITE_ROW) region = col(stmt, 0);
    sqlite3_finalize(stmt);
    return region;
}
