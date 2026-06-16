// =============================================================================
// main.cpp — interactive console entry point for the hospital application.
//
// Ties together the classes exercised individually in test.cpp into real
// menus: top-level login + anonymous E.R. crowding check, then a role-
// specific menu (Doctor / Nurse / Patient) built entirely from methods that
// already exist on those classes. No new business logic lives here — this
// file only does menu plumbing, input prompts, and routing.
//
// Login looks up the typed user ID in the database (real seeded users, not
// a hardcoded mock), then calls that role's existing workLogin()/login()
// method using the looked-up password_hash/workpass as the value to check
// the user's typed credentials against. Since nothing in this project does
// real password hashing, "the password" is literally the placeholder
// strings in data/seed.sql (e.g. "$2b$10$placeholder_hash_001" for
// USR-0001) — type that to log in. See README.md.
// =============================================================================

#include "Database.h"
#include "User.h"
#include "Doctor.h"
#include "Nurse.h"
#include "Patient.h"
#include "Triage.h"
#include "Appointment.h"
#include "Payment.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
using namespace std;

// ── Small input helpers ──────────────────────────────────────────────────────

static void pressEnter() {
    cout << "\nPress Enter to continue...";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Reads a menu choice; returns -1 (never a valid option) on bad input instead
// of leaving cin in a failed state that would loop forever.
static int readChoice() {
    int choice;
    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    return choice;
}

static double readAmount() {
    double amount;
    if (!(cin >> amount)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1.0;
    }
    return amount;
}

// ── E.R. / Triage management — shared by Doctor and Nurse menus ────────────────

static void triageMenu(Database& db) {
    while (true) {
        cout << "\n╔══════════════════════════════════════════════════╗\n";
        cout << "║  E.R. / TRIAGE MANAGEMENT                        ║\n";
        cout << "╠══════════════════════════════════════════════════╣\n";
        cout << "║  1. Admit patient to E.R.                        ║\n";
        cout << "║  2. Update triage code                           ║\n";
        cout << "║  3. Discharge from E.R.                          ║\n";
        cout << "║  4. View triage status                           ║\n";
        cout << "║  5. Check E.R. crowding                          ║\n";
        cout << "║  0. Back                                         ║\n";
        cout << "╚══════════════════════════════════════════════════╝\n";
        cout << "Choice: ";
        int choice = readChoice();
        if (cin.eof()) { cout << "\nInput closed.\n"; return; }

        Triage t;   // each action looks records up by ID, so a fresh object is fine
        switch (choice) {
            case 1: t.admitPatient(db);     pressEnter(); break;
            case 2: t.updateTriageCode(db); pressEnter(); break;
            case 3: t.dischargeFromER(db);  pressEnter(); break;
            case 4: t.viewStatus(db);       pressEnter(); break;
            case 5: t.checkERCrowding(db);  pressEnter(); break;
            case 0: return;
            default: cout << "Invalid choice.\n";
        }
    }
}

// ── Doctor menu ──────────────────────────────────────────────────────────────

static void doctorMenu(Doctor& doc, Database& db) {
    MedicalRecord currentRecord;   // empty sentinel until "Load patient record"

    auto needRecord = [&]() {
        if (currentRecord.getRecordId().empty()) {
            cout << "No record loaded — use option 1 first.\n";
            return false;
        }
        return true;
    };

    while (true) {
        cout << "\n╔══════════════════════════════════════════════════╗\n";
        cout << "║  DOCTOR MENU — Dr. " << doc.getName() << "\n";
        cout << "║  Specialization: " << doc.getSpecialization() << "\n";
        if (!currentRecord.getRecordId().empty())
            cout << "║  Loaded record: " << currentRecord.getRecordId()
                 << " (patient " << currentRecord.getPatientId() << ")\n";
        cout << "╠══════════════════════════════════════════════════╣\n";
        cout << "║   1. Load patient record                         ║\n";
        cout << "║   2. View loaded record                          ║\n";
        cout << "║   3. View a prescription from the loaded record  ║\n";
        cout << "║   4. Update diagnosis & notes                    ║\n";
        cout << "║   5. Issue prescription                          ║\n";
        cout << "║   6. Cancel prescription                         ║\n";
        cout << "║   7. Update prescription dosage                  ║\n";
        cout << "║   8. Admit patient                               ║\n";
        cout << "║   9. Transfer patient                            ║\n";
        cout << "║  10. Discharge patient                           ║\n";
        cout << "║  11. Manage schedule availability                ║\n";
        cout << "║  12. E.R. / Triage management                    ║\n";
        cout << "║  13. Change password                             ║\n";
        cout << "║   0. Logout                                      ║\n";
        cout << "╚══════════════════════════════════════════════════╝\n";
        cout << "Choice: ";
        int choice = readChoice();
        if (cin.eof()) { cout << "\nInput closed.\n"; return; }

        switch (choice) {
            case 1:
                currentRecord = doc.loadPatientRecord(db);
                break;
            case 2:
                if (needRecord()) doc.viewMedicalRecord(currentRecord);
                break;
            case 3:
                if (needRecord()) {
                    const vector<Prescription>& rx = currentRecord.getPrescriptions();
                    if (rx.empty()) {
                        cout << "No prescriptions on this record.\n";
                        break;
                    }
                    for (size_t i = 0; i < rx.size(); i++)
                        cout << "  " << (i + 1) << ". " << rx[i].getPrescriptionId()
                             << " — " << rx[i].getMedicineName() << "\n";
                    cout << "Select: ";
                    int sel = readChoice();
                    if (sel >= 1 && sel <= (int)rx.size()) {
                        Prescription chosen = rx[sel - 1];
                        doc.viewPrescription(chosen);
                    } else {
                        cout << "Invalid selection.\n";
                    }
                }
                break;
            case 4:  if (needRecord()) doc.updateMedicalRecord(currentRecord, db);        break;
            case 5:  if (needRecord()) doc.issuePrescription(currentRecord, db);          break;
            case 6:  if (needRecord()) doc.cancelPrescription(currentRecord, db);         break;
            case 7:  if (needRecord()) doc.updatePrescriptionDosage(currentRecord, db);   break;
            case 8:  if (needRecord()) doc.admitPatient(currentRecord, db);               break;
            case 9:  if (needRecord()) doc.transferPatient(currentRecord, db);            break;
            case 10: if (needRecord()) doc.dischargePatient(currentRecord, db);           break;
            case 11: doc.manageAvailability(db); break;
            case 12: triageMenu(db); continue;   // own loop/pressEnter, skip the one below
            case 13: {
                string pw;   // changePassword has no DB persistence — see README
                doc.changePassword(pw);
                break;
            }
            case 0:
                doc.logout();
                return;
            default:
                cout << "Invalid choice.\n";
        }
        pressEnter();
    }
}

// ── Nurse menu ───────────────────────────────────────────────────────────────

static void nurseMenu(Nurse& nurse, Database& db) {
    while (true) {
        cout << "\n╔══════════════════════════════════════════════════╗\n";
        cout << "║  NURSE MENU — " << nurse.getName() << "\n";
        cout << "╠══════════════════════════════════════════════════╣\n";
        cout << "║  1. View a patient's medical record              ║\n";
        cout << "║  2. View a patient's hospitalization              ║\n";
        cout << "║  3. E.R. / Triage management                     ║\n";
        cout << "║  0. Logout                                       ║\n";
        cout << "╚══════════════════════════════════════════════════╝\n";
        cout << "Choice: ";
        int choice = readChoice();
        if (cin.eof()) { cout << "\nInput closed.\n"; return; }

        switch (choice) {
            case 1: {
                cout << "Patient ID: ";
                string pid;
                cin >> pid;
                MedicalRecord rec = db.loadMedicalRecord(pid);
                if (rec.getRecordId().empty()) cout << "No record found.\n";
                else nurse.viewMedicalRecord(rec);
                break;
            }
            case 2: {
                cout << "Patient ID: ";
                string pid;
                cin >> pid;
                MedicalRecord rec = db.loadMedicalRecord(pid);
                if (rec.getRecordId().empty()) {
                    cout << "No record found.\n";
                    break;
                }
                const vector<Hospitalization>& hs = rec.getHospitalizations();
                if (hs.empty()) {
                    cout << "No hospitalizations on this record.\n";
                    break;
                }
                for (size_t i = 0; i < hs.size(); i++)
                    cout << "  " << (i + 1) << ". " << hs[i].getHospitalizationId()
                         << " — " << hs[i].getHospitalName() << "\n";
                cout << "Select: ";
                int sel = readChoice();
                if (sel >= 1 && sel <= (int)hs.size()) {
                    Hospitalization chosen = hs[sel - 1];
                    nurse.viewHospitalization(chosen);
                } else {
                    cout << "Invalid selection.\n";
                }
                break;
            }
            case 3:
                triageMenu(db);
                continue;
            case 0:
                nurse.logout();
                return;
            default:
                cout << "Invalid choice.\n";
        }
        pressEnter();
    }
}

// ── Patient menu ─────────────────────────────────────────────────────────────

static void patientMenu(Patient& patient, Database& db) {
    while (true) {
        cout << "\n╔══════════════════════════════════════════════════╗\n";
        cout << "║  PATIENT MENU — " << patient.getName() << "\n";
        cout << "╠══════════════════════════════════════════════════╣\n";
        cout << "║  1. View my medical record                       ║\n";
        cout << "║  2. View a prescription                          ║\n";
        cout << "║  3. Book an appointment                          ║\n";
        cout << "║  4. Make a payment                               ║\n";
        cout << "║  0. Logout                                       ║\n";
        cout << "╚══════════════════════════════════════════════════╝\n";
        cout << "Choice: ";
        int choice = readChoice();
        if (cin.eof()) { cout << "\nInput closed.\n"; return; }

        switch (choice) {
            case 1: {
                MedicalRecord rec = db.loadMedicalRecord(patient.getId());
                if (rec.getRecordId().empty()) cout << "No medical record on file.\n";
                else patient.viewMedicalRecord(rec);
                break;
            }
            case 2: {
                MedicalRecord rec = db.loadMedicalRecord(patient.getId());
                if (rec.getRecordId().empty()) {
                    cout << "No medical record on file.\n";
                    break;
                }
                const vector<Prescription>& rx = rec.getPrescriptions();
                if (rx.empty()) {
                    cout << "No prescriptions on this record.\n";
                    break;
                }
                for (size_t i = 0; i < rx.size(); i++)
                    cout << "  " << (i + 1) << ". " << rx[i].getPrescriptionId()
                         << " — " << rx[i].getMedicineName() << "\n";
                cout << "Select: ";
                int sel = readChoice();
                if (sel >= 1 && sel <= (int)rx.size()) {
                    Prescription chosen = rx[sel - 1];
                    patient.viewPrescription(chosen);
                } else {
                    cout << "Invalid selection.\n";
                }
                break;
            }
            case 3:
                patient.bookAppointment(db);
                break;
            case 4: {
                cout << "Appointment ID: ";
                string aid;
                cin >> aid;
                string pid, did, date, time, status;
                if (!db.loadAppointmentBasic(aid, pid, did, date, time, status)) {
                    cout << "Appointment not found.\n";
                    break;
                }
                Appointment apt(aid, pid, did, date, time, status);

                cout << "Amount: ";
                double amount = readAmount();
                if (amount < 0) {
                    cout << "Invalid amount.\n";
                    break;
                }
                cout << "Method (e.g. card/cash): ";
                string method;
                cin >> method;

                string paymentId = db.generateNextId("payments", "payment_id", "PAY");
                Payment pay(paymentId, "", "", amount, method);
                patient.makePayment(apt, pay, db);
                break;
            }
            case 0:
                patient.logout();
                return;
            default:
                cout << "Invalid choice.\n";
        }
        pressEnter();
    }
}

// ── Login routing ────────────────────────────────────────────────────────────

static void loginFlow(Database& db) {
    cout << "User ID: ";
    string id;
    cin >> id;

    string name, surname, email, phone, passwordHash, role;
    if (!db.getUserBasicInfo(id, name, surname, email, phone, passwordHash, role)) {
        cout << "No user found with that ID.\n";
        return;
    }

    if (role == "doctor") {
        string spec     = db.getDoctorSpecialization(id);
        string workpass = db.getStaffWorkpass(id);
        Doctor doc(id, name, surname, email, phone, spec, passwordHash);
        string wp = workpass, pw = passwordHash, nm = name;
        if (doc.workLogin(wp, pw, nm)) doctorMenu(doc, db);
    } else if (role == "nurse") {
        string workpass = db.getStaffWorkpass(id);
        Nurse nurse(id, name, surname, email, phone, passwordHash);
        string wp = workpass, pw = passwordHash, nm = name;
        if (nurse.workLogin(wp, pw, nm)) nurseMenu(nurse, db);
    } else if (role == "patient") {
        string insurance = db.getPatientInsurance(id);
        Patient patient(id, name, surname, email, phone, insurance, passwordHash);
        string em = email, pw = passwordHash, nm = name;
        if (patient.login(em, pw, nm)) patientMenu(patient, db);
    } else {
        cout << "Unknown role for this user.\n";
    }
}

// ── Top-level menu ───────────────────────────────────────────────────────────

int main() {
    // Use is_open(), not good() — on this toolchain a freshly-constructed
    // ifstream on a nonexistent file reports good()==true even though
    // is_open()==false, so good() can't be used to detect "file missing".
    bool isNewDb = !ifstream("data/hospital.db").is_open();

    Database db;
    if (!db.open("data/hospital.db")) {
        cerr << "Cannot open database.\n";
        return 1;
    }
    if (isNewDb) {
        if (!db.execSchema("data/schema.sql") || !db.execSeed("data/seed.sql")) {
            cerr << "Failed to initialize a fresh database.\n";
            return 1;
        }
        cout << "Initialized a fresh database with sample data.\n";
    }

    while (true) {
        cout << "\n╔══════════════════════════════════════════════════╗\n";
        cout << "║                 HOSPITAL APP                     ║\n";
        cout << "╠══════════════════════════════════════════════════╣\n";
        cout << "║  1. Login                                        ║\n";
        cout << "║  2. Check E.R. Crowding Status                   ║\n";
        cout << "║  3. View FAQ & Assistance                        ║\n";
        cout << "║  0. Exit                                         ║\n";
        cout << "╚══════════════════════════════════════════════════╝\n";
        cout << "Choice: ";
        int choice = readChoice();
        if (cin.eof()) { cout << "\nInput closed.\n"; return 0; }

        switch (choice) {
            case 1:
                loginFlow(db);
                break;
            case 2: {
                Triage anon;   // UC 3 — available to any user, including anonymous
                anon.checkERCrowding(db);
                break;
            }
            case 3: {
                User anon("", "", "", "", "", "");   // UC 5 — generic, no identity needed
                anon.customerService();
                break;
            }
            case 0:
                cout << "Goodbye!\n";
                db.close();
                return 0;
            default:
                cout << "Invalid choice.\n";
        }
        pressEnter();
    }
}
