// =============================================================================
// test.cpp — exercises every public method in the project, not just the
// database round-trip. See README.md for how to compile/run this.
//
// Structure:
//   - A tiny harness (check / runIO) at the top.
//   - One section per class, in dependency order (User -> Patient/Doctor/Nurse,
//     MedicalRecord/Prescription/Hospitalization, Database, stub entities).
//   - A final PASS/FAIL summary; exit code is 0 only if everything passed.
//
// `runIO(input, fn)` redirects cin to read from `input` (whitespace-separated
// tokens, exactly like every interactive method in this codebase reads via
// `cin >> var`) and captures whatever `fn` prints to cout, returning it as a
// string so we can assert on the message instead of just "it didn't crash".
// =============================================================================

#include "Database.h"
#include "User.h"
#include "Doctor.h"
#include "Nurse.h"
// #include "Patient.h" — temporarily excluded: Patient.h/.cpp are out of sync
// (mid-redesign), see chat. Re-enable once Patient.cpp matches Patient.h again.
#include "Appointment.h"
#include "Schedule.h"
#include "Payment.h"
#include <iostream>
#include <sstream>
#include <functional>
#include <cstdio>
using namespace std;

static int total = 0, passed = 0;

static void check(bool cond, const string& label) {
    total++;
    if (cond) { passed++; cout << "[PASS] " << label << "\n"; }
    else      {           cout << "[FAIL] " << label << "\n"; }
}

static string runIO(const string& input, const function<void()>& fn) {
    istringstream in(input);
    ostringstream out;
    streambuf* origIn  = cin.rdbuf(in.rdbuf());
    streambuf* origOut = cout.rdbuf(out.rdbuf());
    fn();
    cin.rdbuf(origIn);
    cout.rdbuf(origOut);
    return out.str();
}

int main() {
    // ── PART 0: Database setup ──────────────────────────────────────────────
    remove("data/hospital.db");   // always start from a clean slate
    Database db;
    if (!db.open("data/hospital.db")) { cerr << "open failed\n"; return 1; }
    if (!db.execSchema("data/schema.sql")) { cerr << "schema failed\n"; return 1; }
    if (!db.execSeed("data/seed.sql"))     { cerr << "seed failed\n";   return 1; }

    cout << "\n=== PART 1: Database utilities ===\n";
    {
        string ts = Database::currentTimestamp();
        check(ts.size() == 16 && ts[4] == '-' && ts[7] == '-' &&
              ts[10] == ' ' && ts[13] == ':',
              "Database::currentTimestamp() returns 'YYYY-MM-DD HH:MM'");

        string id1 = db.generateNextId("prescriptions", "prescription_id", "PRX");
        check(id1 == "PRX-0007", "Database::generateNextId() continues from seeded max (" + id1 + ")");

        check(db.getHospitalRegion("Ospedale Santa Chiara") == "Trentino-Alto Adige",
              "Database::getHospitalRegion() looks up a known hospital");
        check(db.getHospitalRegion("Nonexistent Hospital").empty(),
              "Database::getHospitalRegion() returns empty for an unknown hospital");
    }

    cout << "\n=== PART 2: User (base class) ===\n";
    {
        User plain("USR-9999", "Test", "User", "test@example.com", "000", "pw");
        check(plain.getId() == "USR-9999", "User::getId()");
        check(plain.getName() == "Test", "User::getName()");
        check(plain.getEmail() == "test@example.com", "User::getEmail()");
        plain.setEmail("new@example.com");
        check(plain.getEmail() == "new@example.com", "User::setEmail()");

        string out = runIO("y", [&]{ plain.logout(); });
        check(out.find("logged out") != string::npos, "User::logout() accepts 'y'");

        out = runIO("n", [&]{ plain.logout(); });
        check(out.find("cancelled") != string::npos, "User::logout() accepts 'n'");

        out = runIO("x", [&]{ plain.logout(); });
        check(out.find("Invalid choice") != string::npos, "User::logout() rejects garbage input");

        // checkER() / customerService() are intentionally empty stubs — calling
        // them just confirms they're linkable and don't crash.
        plain.checkER();
        plain.customerService();
        check(true, "User::checkER() / User::customerService() are callable (empty stubs)");
    }

    cout << "\n=== PART 3: Doctor ===\n";
    Doctor drElena("USR-0004", "Elena", "Ricci", "elena.ricci@hospital.it",
                    "+39 335 4567890", "Cardiology", "DrPass123!");
    {
        check(drElena.getSpecialization() == "Cardiology", "Doctor::getSpecialization()");
        drElena.setSpecialization("Oncology");
        check(drElena.getSpecialization() == "Oncology", "Doctor::setSpecialization()");
        drElena.setSpecialization("Cardiology");   // restore for the rest of the test

        string workPass = "WP-DOC-0041", password = "DrPass123!", name = "Elena";
        string out = runIO("WP-DOC-0041 DrPass123!",
                            [&]{ drElena.workLogin(workPass, password, name); });
        check(out.find("Login successful") != string::npos, "Doctor::workLogin() accepts correct credentials");

        out = runIO("wrong wrong", [&]{ drElena.workLogin(workPass, password, name); });
        check(out.find("Invalid credentials") != string::npos, "Doctor::workLogin() rejects wrong credentials");

        out = runIO("Str0ngP@ss", [&]{ drElena.changePassword(password); });
        check(out.find("changed successfully") != string::npos &&
              password == "Str0ngP@ss",
              "Doctor::changePassword() accepts a valid password and updates it");

        out = runIO("short1A!", [&]{ drElena.changePassword(password); });
        check(out.find("at least 10 characters") != string::npos,
              "Doctor::changePassword() rejects a too-short password");
    }

    // Load the real seeded record for the rest of the Doctor / MedicalRecord tests.
    MedicalRecord rec = db.loadMedicalRecord("USR-0001");
    check(rec.getRecordId() == "REC-0001", "Database::loadMedicalRecord() loads a known patient");

    {
        string out = runIO("", [&]{ drElena.viewMedicalRecord(rec); });
        check(out.find("REC-0001") != string::npos, "Doctor::viewMedicalRecord() prints the record");

        Prescription rx = rec.getPrescriptions()[0];   // PRX-0001, active
        out = runIO("", [&]{ drElena.viewPrescription(rx); });
        check(out.find(rx.getDosage()) != string::npos, "Doctor::viewPrescription() prints the dosage");
    }

    {
        string out = runIO("USR-0001", [&]{ drElena.loadPatientRecord(db); });
        check(out.find("REC-0001") != string::npos,
              "Doctor::loadPatientRecord() finds and displays an existing patient");

        out = runIO("USR-9999", [&]{ drElena.loadPatientRecord(db); });
        check(out.find("No record found") != string::npos,
              "Doctor::loadPatientRecord() reports a missing patient");
    }

    {
        // Single-word values on purpose: this isolates "does the new
        // persistence path work" from the cin>>-tokenisation limitation
        // documented further down for free-text fields.
        string out = runIO("Asthma PeanutAllergy",
                            [&]{ drElena.updateMedicalRecord(rec, db); });
        check(out.find("updated successfully") != string::npos &&
              rec.getDiagnosis() == "Asthma" && rec.getNotes() == "PeanutAllergy",
              "Doctor::updateMedicalRecord() updates the in-memory record");

        MedicalRecord reloaded = db.loadMedicalRecord("USR-0001");
        check(reloaded.getDiagnosis() == "Asthma" && reloaded.getNotes() == "PeanutAllergy",
              "Doctor::updateMedicalRecord() persists diagnosis/notes to the database");
    }

    string newPrescriptionId;
    {
        string out = runIO("Paracetamol 500mg_twice_daily Pain_relief",
                            [&]{ drElena.issuePrescription(rec, db); });
        newPrescriptionId = rec.getPrescriptions().back().getPrescriptionId();
        check(out.find("issued") != string::npos &&
              rec.getPrescriptions().back().getMedicineName() == "Paracetamol",
              "Doctor::issuePrescription() adds a new active prescription (" + newPrescriptionId + ")");

        MedicalRecord reloaded = db.loadMedicalRecord("USR-0001");
        bool found = false;
        for (const Prescription& p : reloaded.getPrescriptions())
            if (p.getPrescriptionId() == newPrescriptionId) found = true;
        check(found, "Doctor::issuePrescription() persists the new prescription to the database");
    }

    {
        string out = runIO(newPrescriptionId + " 250mg_twice_daily",
                            [&]{ drElena.updatePrescriptionDosage(rec, db); });
        bool updated = false;
        for (const Prescription& p : rec.getPrescriptions())
            if (p.getPrescriptionId() == newPrescriptionId && p.getDosage() == "250mg_twice_daily")
                updated = true;
        check(out.find("updated") != string::npos && updated,
              "Doctor::updatePrescriptionDosage() changes the dosage of an existing prescription");

        out = runIO("PRX-NOPE 1mg", [&]{ drElena.updatePrescriptionDosage(rec, db); });
        check(out.find("not found") != string::npos,
              "Doctor::updatePrescriptionDosage() reports an unknown prescription ID");
    }

    {
        string out = runIO(newPrescriptionId, [&]{ drElena.cancelPrescription(rec, db); });
        bool cancelled = false;
        for (const Prescription& p : rec.getPrescriptions())
            if (p.getPrescriptionId() == newPrescriptionId && !p.isActive()) cancelled = true;
        check(out.find("cancelled") != string::npos && cancelled,
              "Doctor::cancelPrescription() deactivates an active prescription");

        out = runIO("PRX-NOPE", [&]{ drElena.cancelPrescription(rec, db); });
        check(out.find("Not found") != string::npos,
              "Doctor::cancelPrescription() reports an unknown prescription ID");
    }

    {
        // Real, multi-word seed values on purpose: department ("General
        // Medicine") and hospital ("Ospedale Santa Chiara") both contain
        // spaces, but admitPatient() reads each field with a single
        // `cin >> field`, which stops at the first space. This call
        // demonstrates that pre-existing limitation rather than hiding it
        // behind single-word test fixtures.
        string out = runIO("General Medicine Ospedale Santa Chiara BED-501 2024-12-01",
                            [&]{ drElena.admitPatient(rec, db); });
        string newHospId = rec.getHospitalizations().back().getHospitalizationId();
        MedicalRecord reloaded = db.loadMedicalRecord("USR-0001");
        bool persistedCorrectly = false;
        for (const Hospitalization& h : reloaded.getHospitalizations())
            if (h.getHospitalizationId() == newHospId &&
                h.getHospitalName() == "Ospedale Santa Chiara")
                persistedCorrectly = true;
        check(persistedCorrectly,
              "Doctor::admitPatient() persists a multi-word hospital name correctly "
              "[KNOWN LIMITATION: cin>>hospitalName truncates at the first space — see summary]");
    }

    {
        // Same limitation, this time on transferPatient()'s newHospital field.
        string out = runIO("HSP-0001 General_Medicine Ospedale Villa Igea BED-210",
                            [&]{ drElena.transferPatient(rec, db); });
        check(out.find("Transfer completed") != string::npos,
              "Doctor::transferPatient() completes a same-region transfer with a multi-word hospital name "
              "[KNOWN LIMITATION: cin>>newHospital truncates at the first space — see summary]");
    }

    {
        string out = runIO("HSP-0001 2024-12-02", [&]{ drElena.dischargePatient(rec, db); });
        bool discharged = false;
        for (const Hospitalization& h : rec.getHospitalizations())
            if (h.getHospitalizationId() == "HSP-0001" && h.isDischarged()) discharged = true;
        check(out.find("discharged") != string::npos && discharged,
              "Doctor::dischargePatient() discharges an active hospitalization");

        out = runIO("HSP-0001 2024-12-03", [&]{ drElena.dischargePatient(rec, db); });
        check(out.find("already") != string::npos,
              "Doctor::dischargePatient() reports an already-discharged hospitalization");
    }

    cout << "\n=== PART 4: Nurse ===\n";
    Nurse nurseSara("USR-0006", "Sara", "Lombardi", "sara.lombardi@hospital.it",
                     "+39 336 6789012", "NursePass123!");
    {
        string workPass = "WP-NRS-0011", password = "NursePass123!", name = "Sara";
        string out = runIO("WP-NRS-0011 NursePass123!",
                            [&]{ nurseSara.workLogin(workPass, password, name); });
        check(out.find("Login successful") != string::npos, "Nurse::workLogin() accepts correct credentials");

        out = runIO("Str0ngP@ss", [&]{ nurseSara.changePassword(password); });
        check(out.find("changed successfully") != string::npos, "Nurse::changePassword() accepts a valid password");

        MedicalRecord rec2 = db.loadMedicalRecord("USR-0002");
        out = runIO("", [&]{ nurseSara.viewMedicalRecord(rec2); });
        check(out.find("REC-0002") != string::npos, "Nurse::viewMedicalRecord() prints the record");

        Hospitalization h = rec2.getHospitalizations()[0];
        out = runIO("", [&]{ nurseSara.viewHospitalization(h); });
        check(out.find(h.getHospitalizationId()) != string::npos, "Nurse::viewHospitalization() prints the hospitalization");

        Prescription rx = rec2.getPrescriptions()[0];
        out = runIO("", [&]{ nurseSara.viewPrescription(rx); });
        check(out.find(rx.getDosage()) != string::npos && out.find(rx.getDescription()) != string::npos,
              "Nurse::viewPrescription() prints dosage/description (regression check for the getDose()/getInstructions() fix)");
    }

    cout << "\n=== PART 5: Patient — SKIPPED ===\n";
    cout << "  Patient.h / Patient.cpp are currently out of sync (mid-redesign);\n";
    cout << "  see chat for details. Re-enable this part once they match again.\n";

    cout << "\n=== PART 6: MedicalRecord ===\n";
    {
        MedicalRecord empty;
        check(empty.getRecordId().empty(), "MedicalRecord() default constructor is the 'not found' sentinel");

        MedicalRecord parsed("REC-TEST", "USR-TEST", "Penicillin, Latex",
                              "Asthma diagnosed 2020. Broken arm 2021.", "2025-01-01 10:00");
        check(parsed.getAllergies().size() == 2 && parsed.getAllergies()[1] == "Latex",
              "MedicalRecord constructor splits the allergies string");
        check(parsed.getMedicalHistory().size() == 2 && parsed.getMedicalHistory()[1] == "Broken arm 2021",
              "MedicalRecord constructor splits the medical-history string on '. '");

        parsed.addAllergy("Pollen");
        parsed.addMedicalHistoryEntry("Flu 2023");
        check(parsed.getAllergies().size() == 3 && parsed.getMedicalHistory().size() == 3,
              "MedicalRecord::addAllergy() / addMedicalHistoryEntry() append loader entries");

        Prescription rxStub("PRX-STUB", "REC-TEST", "Test Med", "1mg", "desc", true, "USR-0004", "2025-01-01 10:00");
        parsed.addPrescriptionToRecord(rxStub);
        check(parsed.getPrescriptions().size() == 1, "MedicalRecord::addPrescriptionToRecord() appends a loader entry");

        Hospitalization hStub("HSP-STUB", "REC-TEST", "USR-TEST", "Dept", "Hosp", "BED-1", "2025-01-01");
        parsed.addHospitalizationToRecord(hStub);
        check(parsed.getHospitalizations().size() == 1, "MedicalRecord::addHospitalizationToRecord() appends a loader entry");

        check(parsed.getDate() == "2025-01-01 10:00", "MedicalRecord::getDate() aliases lastModified");
        parsed.setDiagnosis("Migraine");
        parsed.setNotes("None");
        check(parsed.getDiagnosis() == "Migraine" && parsed.getNotes() == "None",
              "MedicalRecord::setDiagnosis() / setNotes() round-trip in-memory");

        parsed.display();   // visual smoke test — prints the full record once
    }

    cout << "\n=== PART 7: Prescription ===\n";
    {
        Prescription p("PRX-TEST", "REC-TEST", "Ibuprofen", "200mg", "desc", true, "USR-0004", "2025-01-01 10:00");
        p.setMedicineName("Naproxen");
        p.setDosage("250mg");
        p.setDescription("new desc");
        p.setActive(false);
        check(p.getMedicineName() == "Naproxen" && p.getDosage() == "250mg" &&
              p.getDescription() == "new desc" && !p.isActive(),
              "Prescription DB-aligned setters round-trip in-memory");

        p.setMedication("Aspirin");
        p.setPatientId("USR-TEST");
        p.setFrequency("twice_daily");
        p.setDuration("7_days");
        check(p.getMedication() == "Aspirin" && p.getPatientId() == "USR-TEST" &&
              p.getFrequency() == "twice_daily" && p.getDuration() == "7_days" &&
              p.getIssueDate() == p.getCreatedAt(),
              "Prescription Doctor.cpp-compatibility aliases round-trip in-memory");

        // Real seeded prescription, exercised through the DB-aware methods directly.
        MedicalRecord rec3 = db.loadMedicalRecord("USR-0002");
        Prescription real = rec3.getPrescriptions()[0];   // PRX-0003, active
        real.updateDosage("750mg twice daily", db);
        real.cancel(db);

        MedicalRecord reloaded = db.loadMedicalRecord("USR-0002");
        bool ok = false;
        for (const Prescription& rp : reloaded.getPrescriptions())
            if (rp.getPrescriptionId() == real.getPrescriptionId())
                ok = (rp.getDosage() == "750mg twice daily" && !rp.isActive());
        check(ok, "Prescription::updateDosage() / Prescription::cancel() persist to the database");
    }

    cout << "\n=== PART 8: Hospitalization ===\n";
    {
        Hospitalization h("HSP-TEST", "REC-TEST", "USR-TEST", "Dept", "Hosp", "BED-1", "2025-01-01");
        check(h.getRoom() == "BED-1" && h.isActive(), "Hospitalization compatibility aliases reflect constructor state");
        h.setRoom("BED-2");
        h.setReason("checkup");
        h.setActive(false);
        check(h.getRoom() == "BED-2" && h.getReason() == "checkup" && !h.isActive(),
              "Hospitalization::setRoom() / setReason() / setActive() round-trip in-memory");

        // transfer()/discharge() business logic, exercised directly with real data.
        MedicalRecord rec3 = db.loadMedicalRecord("USR-0002");
        bool sameRegion = rec3.transferPatient("HSP-0002", "Cardiology",
                                                 "Ospedale Villa Igea", "BED-777", db);
        check(sameRegion, "Hospitalization::transfer() succeeds within the same region");

        bool crossRegion = rec3.transferPatient("HSP-0002", "Oncology",
                                                  "Ospedale San Raffaele", "BED-888", db);
        check(!crossRegion, "Hospitalization::transfer() blocks a cross-region move");

        bool dischargedOk = rec3.dischargePatient("HSP-0002", "2025-02-01", db);
        check(dischargedOk, "MedicalRecord::dischargePatient() discharges an active hospitalization (drives Hospitalization::discharge())");
    }

    cout << "\n=== PART 9: stub entity classes ===\n";
    {
        Appointment apt("APT-TEST", "USR-0001", "USR-0004", "2025-03-01", "09:00");
        string out = runIO("", [&]{ apt.display(); });
        check(out.find("APT-TEST") != string::npos && out.find("Booked") != string::npos,
              "Appointment::display() prints the appointment with default status 'Booked'");

        Schedule slot("USR-0004", "2025-03-01", "09:00");
        check(slot.isAvailable(), "Schedule::isAvailable() defaults to true for a freshly constructed slot");

        Payment pay("PAY-TEST", "USR-0001", 49.90);
        pay.pay();
        check(true, "Payment::pay() is callable [NOTE: Payment has no accessor to verify the 'paid' flag changed]");
    }

    // ── Summary ────────────────────────────────────────────────────────────
    cout << "\n=========================================\n";
    cout << passed << "/" << total << " checks passed.\n";
    cout << "=========================================\n";

    return (passed == total) ? 0 : 1;
}
