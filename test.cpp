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
#include "Patient.h"
#include "Appointment.h"
#include "Schedule.h"
#include "Payment.h"
#include "Triage.h"
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
        bool loginOk = false;
        string out = runIO("WP-DOC-0041 DrPass123!",
                            [&]{ loginOk = drElena.workLogin(workPass, password, name); });
        check(out.find("Login successful") != string::npos && loginOk,
              "Doctor::workLogin() accepts correct credentials and returns true");

        loginOk = true;
        out = runIO("wrong wrong", [&]{ loginOk = drElena.workLogin(workPass, password, name); });
        check(out.find("Invalid credentials") != string::npos && !loginOk,
              "Doctor::workLogin() rejects wrong credentials and returns false");

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
        // department/hospitalName are now read with getline (see Doctor.cpp),
        // so real multi-word values need newline-separated input, not
        // space-separated tokens.
        string out = runIO("General Medicine\nOspedale Santa Chiara\nBED-501\n2024-12-01\n",
                            [&]{ drElena.admitPatient(rec, db); });
        string newHospId = rec.getHospitalizations().back().getHospitalizationId();
        MedicalRecord reloaded = db.loadMedicalRecord("USR-0001");
        bool persistedCorrectly = false;
        for (const Hospitalization& h : reloaded.getHospitalizations())
            if (h.getHospitalizationId() == newHospId &&
                h.getHospitalName() == "Ospedale Santa Chiara" &&
                h.getDepartment() == "General Medicine")
                persistedCorrectly = true;
        check(out.find("Patient admitted") != string::npos && persistedCorrectly,
              "Doctor::admitPatient() persists a multi-word department/hospital name correctly");

        out = runIO("ICU\nNonexistent Hospital\nBED-1\n2024-12-01\n",
                     [&]{ drElena.admitPatient(rec, db); });
        check(out.find("Admission failed") != string::npos,
              "Doctor::admitPatient() reports failure for an unknown hospital instead of silently desyncing memory/DB");
    }

    {
        // newDept/newHospital are now read with getline; hospitalizationId
        // above it is still read with cin >>, hence the newline-separated
        // input — see the cin.ignore() in Doctor::transferPatient.
        string out = runIO("HSP-0001\nGeneral Medicine\nOspedale Villa Igea\nBED-210\n",
                            [&]{ drElena.transferPatient(rec, db); });
        check(out.find("Transfer completed") != string::npos,
              "Doctor::transferPatient() completes a same-region transfer with a multi-word hospital name");
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
        bool nurseLoginOk = false;
        string out = runIO("WP-NRS-0011 NursePass123!",
                            [&]{ nurseLoginOk = nurseSara.workLogin(workPass, password, name); });
        check(out.find("Login successful") != string::npos && nurseLoginOk,
              "Nurse::workLogin() accepts correct credentials and returns true");

        out = runIO("Str0ngP@ss", [&]{ nurseSara.changePassword(password); });
        check(out.find("changed successfully") != string::npos, "Nurse::changePassword() accepts a valid password");

        MedicalRecord rec2 = db.loadMedicalRecord("USR-0002");
        out = runIO("", [&]{ nurseSara.viewMedicalRecord(rec2); });
        check(out.find("REC-0002") != string::npos, "Nurse::viewMedicalRecord() prints the record");

        Hospitalization h = rec2.getHospitalizations()[0];
        out = runIO("", [&]{ nurseSara.viewHospitalization(h); });
        check(out.find(h.getHospitalizationId()) != string::npos, "Nurse::viewHospitalization() prints the hospitalization");
        // Nurse::viewPrescription() was removed from Nurse.h/.cpp — no longer testable.
    }

    cout << "\n=== PART 5: Patient ===\n";
    Patient marco("USR-0001", "Marco", "Bianchi", "marco.bianchi@email.it",
                  "+39 333 1234567", "Unipol — Policy #UP-8821", "PatientPass123!");
    {
        check(marco.getInsurance() == "Unipol — Policy #UP-8821", "Patient::getInsurance()");
        string newIns = "Generali — Policy #GN-0000";
        marco.setInsurance(newIns);
        check(marco.getInsurance() == "Generali — Policy #GN-0000", "Patient::setInsurance()");

        string email = "marco.bianchi@email.it", password = "PatientPass123!", name = "Marco";
        bool patientLoginOk = false;
        string out = runIO("1 marco.bianchi@email.it PatientPass123!",
                            [&]{ patientLoginOk = marco.login(email, password, name); });
        check(out.find("Login successful") != string::npos && patientLoginOk,
              "Patient::login() accepts correct CIE credentials and returns true");

        patientLoginOk = true;
        out = runIO("9 wrong wrong", [&]{ patientLoginOk = marco.login(email, password, name); });
        check(out.find("invalid") != string::npos && out.find("Invalid credentials") != string::npos &&
              !patientLoginOk,
              "Patient::login() rejects an unknown menu choice and wrong credentials, returns false");

        out = runIO("", [&]{ marco.viewMedicalRecord(rec); });
        check(out.find("REC-0001") != string::npos, "Patient::viewMedicalRecord() prints the record");

        Prescription rx = rec.getPrescriptions()[0];
        out = runIO("", [&]{ marco.viewPrescription(rx); });
        check(out.find(rx.getDosage()) != string::npos, "Patient::viewPrescription() prints the dosage");

        // ── bookAppointment(db): search by doctor, then book the one free slot ──
        // Seed data: USR-0004 has SCH-0001 (busy) and SCH-0002 (free, 2024-12-01
        // 10:00). cin.ignore() inside bookAppointment eats the single newline
        // left after `cin >> choice`, then getline reads the doctorId line.
        out = runIO("3\nUSR-0004\n1\n", [&]{ marco.bookAppointment(db); });
        check(out.find("APPOINTMENT BOOKED") != string::npos,
              "Patient::bookAppointment() books the one available slot found by doctor search");
        {
            vector<Schedule> remaining = db.getDoctorSchedule("USR-0004");
            bool stillAvailable = false;
            for (const Schedule& s : remaining)
                if (s.getSlotId() == "SCH-0002" && s.isAvailable()) stillAvailable = true;
            check(!stillAvailable, "Patient::bookAppointment() marks the booked slot unavailable");
        }

        out = runIO("0", [&]{ marco.bookAppointment(db); });
        check(out.find("cancelled") != string::npos, "Patient::bookAppointment() honors the cancel option");

        out = runIO("3\nUSR-0004\n1\n", [&]{ marco.bookAppointment(db); });
        check(out.find("No available slots") != string::npos,
              "Patient::bookAppointment() reports no slots once the doctor's free slot is gone");

        // ── makePayment(appointment, payment, db): online and cash paths ────────
        Appointment apt("APT-0001", "USR-0001", "USR-0004", "2024-12-01", "09:00", "pending");
        Payment onlinePay("PAY-T1", "", "", 75.0, "online");
        out = runIO("1 1 4111111111111111", [&]{ marco.makePayment(apt, onlinePay, db); });
        check(out.find("Payment successful") != string::npos && apt.isPaid() &&
              apt.getStatus() == "confirmed",
              "Patient::makePayment() online path pays, confirms, and marks the appointment paid");

        Appointment apt2("APT-0002", "USR-0002", "USR-0005", "2024-12-03", "10:30", "pending");
        Payment cashPay("PAY-T2", "", "", 40.0, "cash");
        out = runIO("2", [&]{ marco.makePayment(apt2, cashPay, db); });
        check(out.find("registered as pending") != string::npos && !apt2.isPaid() &&
              apt2.getStatus() == "confirmed",
              "Patient::makePayment() cash path confirms the appointment without marking it paid");

        Appointment apt3("APT-0003", "USR-0003", "USR-0004", "2024-12-05", "14:00", "pending");
        Payment cancelPay("PAY-T3", "", "", 10.0, "online");
        out = runIO("0", [&]{ marco.makePayment(apt3, cancelPay, db); });
        check(out.find("Payment cancelled") != string::npos, "Patient::makePayment() honors the cancel option");
    }

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

    cout << "\n=== PART 9: Appointment, Payment, Schedule ===\n";
    {
        Appointment apt("APT-0002", "USR-0002", "USR-0005", "2024-12-03", "10:30", "pending");
        check(apt.getAppointmentId() == "APT-0002" && !apt.isPaid() &&
              apt.getStatus() == "pending", "Appointment constructor/getters round-trip");

        string out = runIO("", [&]{ apt.display(); });
        check(out.find("APT-0002") != string::npos && out.find("pending") != string::npos,
              "Appointment::display() prints the appointment");

        apt.confirm(db);
        check(apt.getStatus() == "confirmed", "Appointment::confirm() updates in-memory status");

        apt.reschedule("2024-12-04", "11:00", db);
        check(apt.getDate() == "2024-12-04" && apt.getTime() == "11:00",
              "Appointment::reschedule() updates in-memory date/time");

        apt.markPaid(db);
        check(apt.isPaid(), "Appointment::markPaid() updates in-memory paid flag");

        apt.cancel(db);
        check(apt.getStatus() == "cancelled", "Appointment::cancel() updates in-memory status");

        Payment p("PAY-DIRECT", "APT-0003", "USR-0003", 25.0, "online");
        check(p.getPaymentId() == "PAY-DIRECT" && p.getAmount() == 25.0 &&
              p.getMethod() == "online" && p.getStatus() == "pending",
              "Payment constructor/getters round-trip");

        bool paid = p.processOnlinePayment(db);
        check(paid && p.getStatus() == "paid",
              "Payment::processOnlinePayment() persists and updates status");

        Payment p2("PAY-DIRECT-2", "APT-0001", "USR-0001", 15.0, "cash");
        p2.registerPendingPayment(db);
        check(p2.getStatus() == "pending", "Payment::registerPendingPayment() sets pending status");

        p2.cancel(db);
        check(p2.getStatus() == "cancelled", "Payment::cancel() updates status and persists");
    }
    {
        Schedule slot("SCH-TEST", "USR-0004", "2025-03-01", "09:00", true);
        check(slot.getSlotId() == "SCH-TEST" && slot.getDoctorId() == "USR-0004" &&
              slot.isAvailable(), "Schedule constructor/getters round-trip");

        slot.setDate("2025-03-02");
        slot.setTimeSlot("10:00");
        slot.setAvailable(false);
        check(slot.getDate() == "2025-03-02" && slot.getTimeSlot() == "10:00" &&
              !slot.isAvailable(), "Schedule setters round-trip in-memory");

        string out = runIO("", [&]{ slot.display(); });
        check(out.find("SCH-TEST") != string::npos, "Schedule::display() prints the slot");
    }

    cout << "\n=== PART 10: Doctor schedule / availability ===\n";
    {
        // Seed data has SCH-0001..0004 for USR-0004/USR-0005; SCH-0002 is the
        // only available one for USR-0004 (see data/seed.sql).
        vector<Schedule> slots = db.getDoctorSchedule("USR-0004");
        check(slots.size() == 2, "Database::getDoctorSchedule() returns this doctor's slots only");

        check(db.checkScheduleConflict("USR-0004", "2024-12-01", "09:00"),
              "Database::checkScheduleConflict() detects an existing slot");
        check(!db.checkScheduleConflict("USR-0004", "2024-12-01", "23:59"),
              "Database::checkScheduleConflict() reports no conflict for a free slot");

        bool added = db.addScheduleSlot("USR-0004", "2025-04-01", "08:00");
        check(added, "Database::addScheduleSlot() adds a new slot");
        check(db.getDoctorSchedule("USR-0004").size() == 3,
              "Database::addScheduleSlot() persists — slot count goes up");

        bool removed = db.removeScheduleSlot("USR-0004", "2025-04-01", "08:00");
        check(removed, "Database::removeScheduleSlot() removes an existing slot");
        check(db.getDoctorSchedule("USR-0004").size() == 2,
              "Database::removeScheduleSlot() persists — slot count goes back down");

        bool removedAgain = db.removeScheduleSlot("USR-0004", "2025-04-01", "08:00");
        check(!removedAgain, "Database::removeScheduleSlot() reports failure for an already-removed slot");

        // Both seeded doctors (USR-0004, USR-0005) practice at Ospedale Santa
        // Chiara; no doctor is seeded at Ospedale Villa Igea.
        vector<Schedule> byHospital = db.searchAvailableSlots("", "Ospedale Santa Chiara", "");
        bool onlyExpectedDoctors = !byHospital.empty();
        for (const Schedule& s : byHospital)
            if (s.getDoctorId() != "USR-0004" && s.getDoctorId() != "USR-0005")
                onlyExpectedDoctors = false;
        check(onlyExpectedDoctors,
              "Database::searchAvailableSlots() filters by hospital via doctors.hospital_name");

        vector<Schedule> byOtherHospital = db.searchAvailableSlots("", "Ospedale Villa Igea", "");
        check(byOtherHospital.empty(),
              "Database::searchAvailableSlots() returns nothing for a hospital with no doctors");

        string out = runIO("0", [&]{ drElena.manageAvailability(db); });
        check(out.find("cancelled") != string::npos,
              "Doctor::manageAvailability() honors the cancel option without changing anything");
    }

    cout << "\n=== PART 11: Triage / E.R. ===\n";
    {
        Triage empty;
        check(empty.getTriageId().empty(), "Triage() default constructor is the 'not found' sentinel");

        Triage seeded("TRG-TEST", "USR-TEST", "Ospedale Santa Chiara", "Red",
                       "Test diagnosis", "2025-01-01 00:00", false, "");
        check(seeded.getCode() == "Red" && seeded.getDiagnosis() == "Test diagnosis" &&
              !seeded.isDischarged(), "Triage constructor/getters round-trip");

        // ── admitPatient(db): hospital name and diagnosis are read with
        // getline (multi-word, FK-checked for hospital), so they're on their
        // own lines; patient ID / code choice / confirm are single tokens.
        Triage t;
        string out = runIO("USR-0002\nOspedale Santa Chiara\n2\nHigh blood pressure\ny\n",
                            [&]{ t.admitPatient(db); });
        check(out.find("Patient admitted to E.R.") != string::npos &&
              t.getCode() == "Orange" && t.getDiagnosis() == "High blood pressure" &&
              !t.isDischarged(),
              "Triage::admitPatient() admits a new E.R. patient");

        Triage reloaded = db.loadTriageRecord(t.getTriageId());
        check(reloaded.getPatientId() == "USR-0002" && reloaded.getHospitalName() == "Ospedale Santa Chiara",
              "Triage::admitPatient() persists the new record to the database");

        out = runIO("9", [&]{ t.admitPatient(db); });
        check(out.find("Invalid code") != string::npos,
              "Triage::admitPatient() rejects an invalid code choice");

        // ── updateTriageCode(db) ────────────────────────────────────────────────
        out = runIO(t.getTriageId() + "\n1\ny", [&]{ t.updateTriageCode(db); });
        check(out.find("updated to: Red") != string::npos && t.getCode() == "Red",
              "Triage::updateTriageCode() changes the code of an active E.R. patient");

        out = runIO("TRG-NOPE\n1\ny", [&]{ t.updateTriageCode(db); });
        check(out.find("not found or already discharged") != string::npos,
              "Triage::updateTriageCode() reports an unknown triage ID");

        // ── viewStatus(db) ───────────────────────────────────────────────────────
        out = runIO("TRG-0001", [&]{ t.viewStatus(db); });
        check(out.find("Severe chest pain") != string::npos && out.find("currently in E.R.") != string::npos,
              "Triage::viewStatus() shows an active seeded patient's record");

        out = runIO("TRG-0003", [&]{ t.viewStatus(db); });
        check(out.find("Discharged:") != string::npos,
              "Triage::viewStatus() shows a discharged seeded patient's record");

        out = runIO("TRG-NOPE", [&]{ t.viewStatus(db); });
        check(out.find("No triage record found") != string::npos,
              "Triage::viewStatus() reports an unknown triage ID");

        // ── checkERCrowding(db) ──────────────────────────────────────────────────
        // Active at Ospedale Santa Chiara: seeded TRG-0001, TRG-0002, plus the
        // one we just admitted (t) = 3, until t is discharged below.
        out = runIO("Ospedale Santa Chiara", [&]{ t.checkERCrowding(db); });
        check(out.find("Total: 3") != string::npos,
              "Triage::checkERCrowding() counts active patients at a busy hospital");

        out = runIO("Ospedale San Raffaele", [&]{ t.checkERCrowding(db); });
        check(out.find("No patients currently in E.R.") != string::npos,
              "Triage::checkERCrowding() reports none for a hospital with no E.R. activity");

        // ── dischargeFromER(db) ──────────────────────────────────────────────────
        out = runIO(t.getTriageId() + "\ny", [&]{ t.dischargeFromER(db); });
        check(out.find("Patient discharged from E.R.") != string::npos && t.isDischarged(),
              "Triage::dischargeFromER() discharges an active E.R. patient");

        out = runIO(t.getTriageId() + "\ny", [&]{ t.dischargeFromER(db); });
        check(out.find("not found or already discharged") != string::npos,
              "Triage::dischargeFromER() reports an already-discharged triage ID");

        out = runIO("Ospedale Santa Chiara", [&]{ t.checkERCrowding(db); });
        check(out.find("Total: 2") != string::npos,
              "Triage::checkERCrowding() count drops back down after a discharge");
    }

    // ── Summary ────────────────────────────────────────────────────────────
    cout << "\n=========================================\n";
    cout << passed << "/" << total << " checks passed.\n";
    cout << "=========================================\n";

    return (passed == total) ? 0 : 1;
}
