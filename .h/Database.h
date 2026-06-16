#ifndef DATABASE_H
#define DATABASE_H

#include "MedicalRecord.h"
#include "Schedule.h"
#include "Triage.h"
#include <string>
#include <vector>
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
    bool updateMedicalRecordFields(const string& recordId,
                                    const string& allergies,
                                    const string& medicalHistory,
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

    // ── Doctor schedule / availability ────────────────────────────────────────
    vector<Schedule> getDoctorSchedule(const string& doctorId);
    bool checkScheduleConflict(const string& doctorId, const string& date,
                                const string& timeSlot);
    bool addScheduleSlot(const string& doctorId, const string& date,
                          const string& timeSlot);
    bool removeScheduleSlot(const string& doctorId, const string& date,
                             const string& timeSlot);

    // ── Appointments ─────────────────────────────────────────────────────────
    bool insertAppointment(const string& appointmentId, const string& patientId,
                            const string& doctorId, const string& date,
                            const string& time, const string& status);
    bool updateAppointmentStatus(const string& appointmentId, const string& status,
                                  const string& timestamp);
    bool updateAppointmentSchedule(const string& appointmentId, const string& newDate,
                                    const string& newTime, const string& timestamp);
    bool updateAppointmentPaymentStatus(const string& appointmentId, bool paid);

    // ── Appointment booking (search + slot availability) ───────────────────────
    // date/hospital/doctorId are each optional — pass "" to skip that filter.
    // hospital is matched via doctors.hospital_name (joined on doctor_id).
    vector<Schedule> searchAvailableSlots(const string& date, const string& hospital,
                                           const string& doctorId);
    bool updateScheduleAvailability(const string& slotId, bool available);

    // ── Payments ─────────────────────────────────────────────────────────────
    bool insertPayment(const string& paymentId, const string& appointmentId,
                        const string& patientId, double amount,
                        const string& method, const string& status);
    bool updatePaymentStatus(const string& paymentId, const string& status);

    // ── Triage / E.R. ────────────────────────────────────────────────────────
    bool insertTriageRecord(const string& triageId, const string& patientId,
                             const string& hospitalName, const string& code,
                             const string& diagnosis, const string& admittedAt);
    bool updateTriageCode(const string& triageId, const string& newCode,
                           const string& timestamp);
    bool dischargeTriageRecord(const string& triageId, const string& timestamp);
    Triage loadTriageRecord(const string& triageId);
    vector<Triage> loadActiveTriageByHospital(const string& hospitalName);

    // ── Login support ────────────────────────────────────────────────────────
    // Looks up a user by ID across users/medical_staff/doctors/patients/nurses.
    // Returns false (out-params left untouched) if no such user exists.
    bool getUserBasicInfo(const string& userId, string& name, string& surname,
                           string& email, string& phone, string& passwordHash,
                           string& role);
    string getDoctorSpecialization(const string& doctorId);
    string getStaffWorkpass(const string& staffId);   // doctors and nurses share this
    string getPatientInsurance(const string& patientId);

    // Minimal fields needed to reconstruct an Appointment for Patient::makePayment
    // after the appointment was created in an earlier menu action.
    bool loadAppointmentBasic(const string& appointmentId, string& patientId,
                               string& doctorId, string& date, string& time,
                               string& status);
};

#endif
