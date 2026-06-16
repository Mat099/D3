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

// Prints the real SQLite error instead of letting a write method fail
// silently — without this, any constraint violation or I/O error just
// looks like "returned false" with no way to tell what actually happened.
static inline void logSqlError(sqlite3* db, const char* context) {
    cerr << "[Database] " << context << " failed: " << sqlite3_errmsg(db) << endl;
}

// All sqlite3_bind_text() calls below use SQLITE_TRANSIENT, not
// SQLITE_STATIC. Most bound values come from `someEntity.getX().c_str()`,
// where getX() returns std::string *by value* — that temporary is
// destroyed at the end of the full expression, before sqlite3_step() ever
// runs. SQLITE_STATIC promises the pointer stays valid for the life of the
// statement, which is false here, so it reads freed memory — a real,
// silent, intermittent dangling-pointer bug (it happened to look like a
// "FOREIGN KEY constraint failed" on a perfectly valid value). TRANSIENT
// makes SQLite copy the bytes immediately during the bind call, which is
// always safe and is the correct default unless you've specifically
// verified the source outlives the statement.

// ── Construction / teardown ───────────────────────────────────────────────────

Database::Database() : db(nullptr) {}

Database::~Database() { close(); }

bool Database::open(const string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    // Retry for up to 5s on SQLITE_BUSY instead of failing immediately —
    // avoids spurious failures from transient file-lock contention (e.g.
    // antivirus or cloud-sync briefly touching the .db file).
    sqlite3_busy_timeout(db, 5000);
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

    sqlite3_bind_text(stmt, 1, patientId.c_str(), -1, SQLITE_TRANSIENT);

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
        sqlite3_bind_text(stmt, 1, recordId.c_str(), -1, SQLITE_TRANSIENT);
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
        sqlite3_bind_text(stmt, 1, recordId.c_str(), -1, SQLITE_TRANSIENT);
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateMedicalRecordTimestamp (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, recordId.c_str(),  -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateMedicalRecordTimestamp (step)");
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateMedicalRecordFields (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, allergies.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, medicalHistory.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timestamp.c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, recordId.c_str(),       -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateMedicalRecordFields (step)");
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "insertPrescription (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, p.getPrescriptionId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, p.getRecordId().c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, p.getMedicineName().c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, p.getDosage().c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, p.getDescription().c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 6, p.isActive() ? 1 : 0);
    sqlite3_bind_text(stmt, 7, p.getDoctorId().c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, p.getCreatedAt().c_str(),      -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "insertPrescription (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::cancelPrescription(const string& prescriptionId,
                                    const string& /*timestamp*/) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE prescriptions SET active = 0 WHERE prescription_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "cancelPrescription (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, prescriptionId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "cancelPrescription (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updatePrescriptionDosage(const string& prescriptionId,
                                         const string& newDosage,
                                         const string& /*timestamp*/) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE prescriptions SET dosage = ? WHERE prescription_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updatePrescriptionDosage (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, newDosage.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, prescriptionId.c_str(),  -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updatePrescriptionDosage (step)");
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "insertHospitalization (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt,  1, h.getHospitalizationId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  2, h.getRecordId().c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  3, h.getPatientId().c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  4, h.getDepartment().c_str(),        -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  5, h.getHospitalName().c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  6, h.getBedId().c_str(),             -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  7, h.getAdmissionDate().c_str(),     -1, SQLITE_TRANSIENT);
    if (h.getDischargeDate().empty())
        sqlite3_bind_null(stmt, 8);
    else
        sqlite3_bind_text(stmt, 8, h.getDischargeDate().c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt,  9, h.isDischarged() ? 1 : 0);
    sqlite3_bind_text(stmt, 10, h.getDoctorId().c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, h.getLastModified().c_str(),      -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "insertHospitalization (step)");
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "dischargeHospitalization (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, date.c_str(),              -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hospitalizationId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "dischargeHospitalization (step)");
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateHospitalizationTransfer (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, department.c_str(),        -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hospital.c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, bedId.c_str(),             -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, hospitalizationId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateHospitalizationTransfer (step)");
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
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "insertTransferLog (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, logId.c_str(),             -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hospitalizationId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, fromDept.c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, fromHosp.c_str(),          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, fromBed.c_str(),           -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, toDept.c_str(),            -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, toHosp.c_str(),            -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, toBed.c_str(),             -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, transferredAt.c_str(),     -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "insertTransferLog (step)");
    sqlite3_finalize(stmt);
    return ok;
}

// ── Hospital region lookup ────────────────────────────────────────────────────

string Database::getHospitalRegion(const string& hospitalName) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT region FROM hospitals WHERE hospital_name = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return "";
    sqlite3_bind_text(stmt, 1, hospitalName.c_str(), -1, SQLITE_TRANSIENT);
    string region;
    if (sqlite3_step(stmt) == SQLITE_ROW) region = col(stmt, 0);
    sqlite3_finalize(stmt);
    return region;
}

// ── Doctor schedule / availability ────────────────────────────────────────────

vector<Schedule> Database::getDoctorSchedule(const string& doctorId) {
    vector<Schedule> slots;
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT slot_id, doctor_id, date, time_slot, available "
        "FROM schedules WHERE doctor_id = ? ORDER BY date, time_slot;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "getDoctorSchedule (prepare)");
        return slots;
    }
    sqlite3_bind_text(stmt, 1, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        slots.push_back(Schedule(col(stmt, 0), col(stmt, 1), col(stmt, 2),
                                  col(stmt, 3), sqlite3_column_int(stmt, 4) != 0));
    }
    sqlite3_finalize(stmt);
    return slots;
}

bool Database::checkScheduleConflict(const string& doctorId, const string& date,
                                      const string& timeSlot) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT 1 FROM schedules WHERE doctor_id = ? AND date = ? AND time_slot = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "checkScheduleConflict (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, date.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timeSlot.c_str(), -1, SQLITE_TRANSIENT);
    bool conflict = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return conflict;
}

bool Database::addScheduleSlot(const string& doctorId, const string& date,
                                const string& timeSlot) {
    string slotId = generateNextId("schedules", "slot_id", "SCH");
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO schedules (slot_id, doctor_id, date, time_slot, available) "
        "VALUES (?, ?, ?, ?, 1);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "addScheduleSlot (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, slotId.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, date.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timeSlot.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "addScheduleSlot (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::removeScheduleSlot(const string& doctorId, const string& date,
                                   const string& timeSlot) {
    sqlite3_stmt* stmt;
    const char* sql =
        "DELETE FROM schedules WHERE doctor_id = ? AND date = ? AND time_slot = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "removeScheduleSlot (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, date.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timeSlot.c_str(), -1, SQLITE_TRANSIENT);
    bool ranOk = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ranOk) logSqlError(db, "removeScheduleSlot (step)");
    sqlite3_finalize(stmt);
    // SQLITE_DONE just means the statement ran without error — also check
    // that a row actually matched, so removing a nonexistent slot reports
    // failure instead of a silent no-op success.
    return ranOk && sqlite3_changes(db) > 0;
}

// ── Appointments ───────────────────────────────────────────────────────────────

bool Database::insertAppointment(const string& appointmentId, const string& patientId,
                                  const string& doctorId, const string& date,
                                  const string& time, const string& status) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO appointments "
        "(appointment_id, patient_id, doctor_id, date, time, status, paid, last_modified) "
        "VALUES (?, ?, ?, ?, ?, ?, 0, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "insertAppointment (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, appointmentId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, patientId.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, doctorId.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, date.c_str(),           -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, time.c_str(),           -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, status.c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, currentTimestamp().c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "insertAppointment (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateAppointmentStatus(const string& appointmentId, const string& status,
                                        const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE appointments SET status = ?, last_modified = ? WHERE appointment_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateAppointmentStatus (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, status.c_str(),        -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, appointmentId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateAppointmentStatus (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateAppointmentSchedule(const string& appointmentId, const string& newDate,
                                          const string& newTime, const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE appointments SET date = ?, time = ?, last_modified = ? "
        "WHERE appointment_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateAppointmentSchedule (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, newDate.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, newTime.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timestamp.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, appointmentId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateAppointmentSchedule (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateAppointmentPaymentStatus(const string& appointmentId, bool paid) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE appointments SET paid = ?, last_modified = ? WHERE appointment_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateAppointmentPaymentStatus (prepare)");
        return false;
    }
    sqlite3_bind_int (stmt, 1, paid ? 1 : 0);
    sqlite3_bind_text(stmt, 2, currentTimestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, appointmentId.c_str(),      -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateAppointmentPaymentStatus (step)");
    sqlite3_finalize(stmt);
    return ok;
}

// ── Appointment booking (search + slot availability) ───────────────────────────

vector<Schedule> Database::searchAvailableSlots(const string& date, const string& hospital,
                                                  const string& doctorId) {
    vector<Schedule> slots;
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT s.slot_id, s.doctor_id, s.date, s.time_slot, s.available "
        "FROM schedules s "
        "JOIN doctors d ON d.doctor_id = s.doctor_id "
        "WHERE s.available = 1 "
        "AND (? = '' OR s.date = ?) "
        "AND (? = '' OR s.doctor_id = ?) "
        "AND (? = '' OR d.hospital_name = ?) "
        "ORDER BY s.date, s.time_slot;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "searchAvailableSlots (prepare)");
        return slots;
    }
    sqlite3_bind_text(stmt, 1, date.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, date.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, hospital.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, hospital.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        slots.push_back(Schedule(col(stmt, 0), col(stmt, 1), col(stmt, 2),
                                  col(stmt, 3), sqlite3_column_int(stmt, 4) != 0));
    }
    sqlite3_finalize(stmt);
    return slots;
}

bool Database::updateScheduleAvailability(const string& slotId, bool available) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE schedules SET available = ? WHERE slot_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateScheduleAvailability (prepare)");
        return false;
    }
    sqlite3_bind_int (stmt, 1, available ? 1 : 0);
    sqlite3_bind_text(stmt, 2, slotId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updateScheduleAvailability (step)");
    sqlite3_finalize(stmt);
    return ok;
}

// ── Payments ─────────────────────────────────────────────────────────────────

bool Database::insertPayment(const string& paymentId, const string& appointmentId,
                              const string& patientId, double amount,
                              const string& method, const string& status) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO payments "
        "(payment_id, appointment_id, patient_id, amount, method, status) "
        "VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "insertPayment (prepare)");
        return false;
    }
    sqlite3_bind_text  (stmt, 1, paymentId.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, appointmentId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, patientId.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, amount);
    sqlite3_bind_text  (stmt, 5, method.c_str(),        -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 6, status.c_str(),        -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "insertPayment (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updatePaymentStatus(const string& paymentId, const string& status) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE payments SET status = ? WHERE payment_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updatePaymentStatus (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, status.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, paymentId.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "updatePaymentStatus (step)");
    sqlite3_finalize(stmt);
    return ok;
}

// ── Triage / E.R. ────────────────────────────────────────────────────────────

bool Database::insertTriageRecord(const string& triageId, const string& patientId,
                                   const string& hospitalName, const string& code,
                                   const string& diagnosis, const string& admittedAt) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO triage "
        "(triage_id, patient_id, hospital_name, code, diagnosis, admitted_at, "
        " discharged, last_modified) "
        "VALUES (?, ?, ?, ?, ?, ?, 0, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "insertTriageRecord (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, triageId.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, patientId.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hospitalName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, code.c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, diagnosis.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, admittedAt.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, admittedAt.c_str(),   -1, SQLITE_TRANSIENT); // last_modified = admitted_at initially
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) logSqlError(db, "insertTriageRecord (step)");
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateTriageCode(const string& triageId, const string& newCode,
                                 const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE triage SET code = ?, last_modified = ? "
        "WHERE triage_id = ? AND discharged = 0;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "updateTriageCode (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, newCode.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, triageId.c_str(),  -1, SQLITE_TRANSIENT);
    bool ranOk = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ranOk) logSqlError(db, "updateTriageCode (step)");
    sqlite3_finalize(stmt);
    // Also false if the triage_id doesn't exist or is already discharged —
    // matches the "not found or already discharged" message in Triage.cpp.
    return ranOk && sqlite3_changes(db) > 0;
}

bool Database::dischargeTriageRecord(const string& triageId, const string& timestamp) {
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE triage SET discharged = 1, discharged_at = ?, last_modified = ? "
        "WHERE triage_id = ? AND discharged = 0;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "dischargeTriageRecord (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, triageId.c_str(),  -1, SQLITE_TRANSIENT);
    bool ranOk = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ranOk) logSqlError(db, "dischargeTriageRecord (step)");
    sqlite3_finalize(stmt);
    return ranOk && sqlite3_changes(db) > 0;
}

Triage Database::loadTriageRecord(const string& triageId) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT triage_id, patient_id, hospital_name, code, diagnosis, admitted_at, "
        "       discharged, discharged_at "
        "FROM triage WHERE triage_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "loadTriageRecord (prepare)");
        return Triage();
    }
    sqlite3_bind_text(stmt, 1, triageId.c_str(), -1, SQLITE_TRANSIENT);
    Triage record;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        record = Triage(col(stmt, 0), col(stmt, 1), col(stmt, 2), col(stmt, 3),
                         col(stmt, 4), col(stmt, 5),
                         sqlite3_column_int(stmt, 6) != 0, col(stmt, 7));
    }
    sqlite3_finalize(stmt);
    return record;
}

vector<Triage> Database::loadActiveTriageByHospital(const string& hospitalName) {
    vector<Triage> patients;
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT triage_id, patient_id, hospital_name, code, diagnosis, admitted_at, "
        "       discharged, discharged_at "
        "FROM triage WHERE hospital_name = ? AND discharged = 0;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "loadActiveTriageByHospital (prepare)");
        return patients;
    }
    sqlite3_bind_text(stmt, 1, hospitalName.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        patients.push_back(Triage(col(stmt, 0), col(stmt, 1), col(stmt, 2), col(stmt, 3),
                                   col(stmt, 4), col(stmt, 5),
                                   sqlite3_column_int(stmt, 6) != 0, col(stmt, 7)));
    }
    sqlite3_finalize(stmt);
    return patients;
}

// ── Login support ──────────────────────────────────────────────────────────────

bool Database::getUserBasicInfo(const string& userId, string& name, string& surname,
                                 string& email, string& phone, string& passwordHash,
                                 string& role) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT name, surname, email, phone_number, password_hash, role "
        "FROM users WHERE user_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "getUserBasicInfo (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;
    if (found) {
        name         = col(stmt, 0);
        surname      = col(stmt, 1);
        email        = col(stmt, 2);
        phone        = col(stmt, 3);
        passwordHash = col(stmt, 4);
        role         = col(stmt, 5);
    }
    sqlite3_finalize(stmt);
    return found;
}

string Database::getDoctorSpecialization(const string& doctorId) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT specialization FROM doctors WHERE doctor_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "getDoctorSpecialization (prepare)");
        return "";
    }
    sqlite3_bind_text(stmt, 1, doctorId.c_str(), -1, SQLITE_TRANSIENT);
    string spec;
    if (sqlite3_step(stmt) == SQLITE_ROW) spec = col(stmt, 0);
    sqlite3_finalize(stmt);
    return spec;
}

string Database::getStaffWorkpass(const string& staffId) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT workpass_id FROM medical_staff WHERE staff_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "getStaffWorkpass (prepare)");
        return "";
    }
    sqlite3_bind_text(stmt, 1, staffId.c_str(), -1, SQLITE_TRANSIENT);
    string workpass;
    if (sqlite3_step(stmt) == SQLITE_ROW) workpass = col(stmt, 0);
    sqlite3_finalize(stmt);
    return workpass;
}

string Database::getPatientInsurance(const string& patientId) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT insurance FROM patients WHERE patient_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "getPatientInsurance (prepare)");
        return "";
    }
    sqlite3_bind_text(stmt, 1, patientId.c_str(), -1, SQLITE_TRANSIENT);
    string insurance;
    if (sqlite3_step(stmt) == SQLITE_ROW) insurance = col(stmt, 0);
    sqlite3_finalize(stmt);
    return insurance;
}

bool Database::loadAppointmentBasic(const string& appointmentId, string& patientId,
                                     string& doctorId, string& date, string& time,
                                     string& status) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT patient_id, doctor_id, date, time, status "
        "FROM appointments WHERE appointment_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logSqlError(db, "loadAppointmentBasic (prepare)");
        return false;
    }
    sqlite3_bind_text(stmt, 1, appointmentId.c_str(), -1, SQLITE_TRANSIENT);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;
    if (found) {
        patientId = col(stmt, 0);
        doctorId  = col(stmt, 1);
        date      = col(stmt, 2);
        time      = col(stmt, 3);
        status    = col(stmt, 4);
    }
    sqlite3_finalize(stmt);
    return found;
}
