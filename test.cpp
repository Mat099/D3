#include ".h\Database.h"
#include <iostream>
using namespace std;

int main() {
    // ── Step 1: Init DB ──────────────────────────────────────────────────────
    Database db;
    if (!db.open("hospital.db")) { cerr << "open failed\n"; return 1; }
    if (!db.execSchema("data/schema.sql")) { cerr << "schema failed\n"; return 1; }
    if (!db.execSeed("data/seed.sql"))     { cerr << "seed failed\n";   return 1; }

    // ── Step 2: Load REC-0001 and display ────────────────────────────────────
    MedicalRecord rec = db.loadMedicalRecord("USR-0001");
    if (rec.getRecordId().empty()) {
        cerr << "Record not found for USR-0001\n";
        return 1;
    }
    cout << "Loaded: " << rec.getRecordId()
         << "  patient: " << rec.getPatientId() << "\n";
    rec.display();

    // ── Step 3: Add a new prescription ───────────────────────────────────────
    string newId = db.generateNextId("prescriptions", "prescription_id", "PRX");
    Prescription p(newId, rec.getRecordId(),
                   "Lisinopril", "10mg once daily",
                   "ACE inhibitor — replacing Ramipril trial",
                   true, "USR-0004", Database::currentTimestamp());
    rec.addPrescription(p, db);
    cout << "Inserted prescription: " << newId << "\n";

    // ── Step 4: Round-trip — fresh connection ─────────────────────────────────
    Database db2;
    db2.open("hospital.db");
    MedicalRecord rec2 = db2.loadMedicalRecord("USR-0001");

    bool found = false;
    for (const Prescription& px : rec2.getPrescriptions())
        if (px.getPrescriptionId() == newId) { found = true; break; }

    cout << "Round-trip check: " << (found ? "PASS" : "FAIL") << "\n";

    // ── Step 5: Same-region transfer (should succeed) ─────────────────────────
    // HSP-0001 is at Ospedale Santa Chiara (Trentino) — transfer to Villa Igea (also Trentino)
    bool transferred = rec2.transferPatient("HSP-0001",
                                             "General Medicine",
                                             "Ospedale Villa Igea",
                                             "BED-210", db2);
    cout << "Same-region transfer: " << (transferred ? "PASS" : "FAIL") << "\n";

    // ── Step 6: Cross-region transfer (should fail) ───────────────────────────
    bool blocked = !rec2.transferPatient("HSP-0001",
                                          "Oncology",
                                          "Ospedale San Raffaele",   // Lombardia
                                          "BED-999", db2);
    cout << "Cross-region block:   " << (blocked ? "PASS" : "FAIL") << "\n";

    return (found && transferred && blocked) ? 0 : 1;
}
