# D3

## Running the test

The test (`test.cpp`) exercises the full database round-trip: schema creation, seeding, loading a medical record, inserting a prescription, same-region transfer, and cross-region transfer block.

**Step 1 — Get the SQLite amalgamation**

The project uses SQLite via the [amalgamation](https://www.sqlite.org/amalgamation.html): two files (`sqlite3.c` and `sqlite3.h`) that you place in the project root. They are not included in the repository.

1. Go to https://www.sqlite.org/download.html
2. Under *Source Code*, download `sqlite-amalgamation-XXXXXXXX.zip` (the number is the version)
3. Extract `sqlite3.c` and `sqlite3.h` into the project root (same folder as `test.cpp`)

**Step 2 — Configure terminal encoding (Windows only)**

The UI uses box-drawing characters (╔ ║ ╚ etc.). Without explicit UTF-8 encoding these render as garbage in the Windows terminal. Run these two commands **once per PowerShell session before compiling or running**:

```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
chcp 65001
```

`chcp 65001` sets the console code page to UTF-8. The `[Console]::OutputEncoding` line tells PowerShell itself to write UTF-8 — both are required; either alone is not enough.

**Step 3 — Compile**

SQLite must be compiled as C, not C++. Compile it separately first:

```bash
gcc -c data/sqlite3.c -o data/sqlite3.o
```

Then compile the rest with G++:

```powershell
g++ -I.h -Idata test.cpp .cpp/Doctor.cpp .cpp/Nurse.cpp .cpp/Patient.cpp .cpp/User.cpp `
    .cpp/Prescription.cpp .cpp/Hospitalization.cpp .cpp/MedicalRecord.cpp `
    .cpp/Database.cpp .cpp/Appointment.cpp .cpp/Schedule.cpp .cpp/Triage.cpp `
    .cpp/MedicalStaff.cpp data/sqlite3.o -o test
```

- `-I.h` tells G++ to search the `.h/` directory for project headers.
- `-Idata` adds the `data/` folder so `sqlite3.h` (which lives there) is found.
- The backtick `` ` `` is the PowerShell line-continuation character (`\` is bash-only).

**Step 4 — Run**

```powershell
./test; if ($?) { "ALL TESTS PASSED" } else { "SOME TESTS FAILED" }
```

Or simply:

```powershell
./test
```

Expected output: a formatted medical record display with box-drawing borders followed by three `PASS` lines (`Round-trip check`, `Same-region transfer`, `Cross-region block`).

---

## Database integration — how it works

### Architecture

All database access goes through a single `Database` class (`Database.h` / `Database.cpp`) that wraps the SQLite C API. The key design principle is **dependency injection**: no domain object holds a database connection. Instead, every method that persists data receives a `Database&` parameter.

```
Doctor  ──calls──►  MedicalRecord::addPrescription(p, db)
                             │
                             ▼
                    Database::insertPrescription(p)
                    Database::updateMedicalRecordTimestamp(...)
```

### Domain object layers

Each domain class (`Prescription`, `Hospitalization`, `MedicalRecord`) has two layers of methods:

- **DB-aware mutating methods** — take a `Database&`, write to SQLite, then update the in-memory object. These are the ones to call from application code.
- **Loader methods** (e.g. `addPrescriptionToRecord`) — called only by `Database::loadMedicalRecord` to populate an object from a query result without triggering another write.

### Doctor's DB-aware methods

`Doctor.cpp` exposes two sets of methods. The original in-memory-only methods (`issuePrescription(Prescription&)`, `admitPatient(Hospitalization&)`, etc.) are kept for backward compatibility. The new DB-aware methods are **overloads** that take `MedicalRecord&` and `Database&` instead:

| Method | Persists via |
|---|---|
| `loadPatientRecord(db)` | `Database::loadMedicalRecord` — loads and displays a full record |
| `issuePrescription(record, db)` | `MedicalRecord::addPrescription` — generates a new PRX-NNNN ID and inserts |
| `cancelPrescription(record, db)` | `MedicalRecord::cancelPrescription` — sets `active = 0` in DB |
| `updatePrescriptionDosage(record, db)` | `MedicalRecord::updatePrescriptionDosage` — updates dosage in DB |
| `admitPatient(record, db)` | `MedicalRecord::addHospitalization` — generates a new HSP-NNNN ID and inserts |
| `transferPatient(record, db)` | `MedicalRecord::transferPatient` — logs to `transfer_log`, updates hospitalization |
| `dischargePatient(record, db)` | `MedicalRecord::dischargePatient` — sets `discharged = 1` and records discharge date |

### Regional transfer constraint

`Hospitalization::transfer()` enforces that patients can only be moved between hospitals in the same region. Before writing, it queries the `hospitals` table for the region of both the current and target hospital. If they differ, it returns `false` and no data is written. The `Doctor::transferPatient` method surfaces this as a "Transfer blocked" message in the UI.

### ID generation

All IDs follow the format `PREFIX-NNNN` (e.g. `PRX-0007`, `HSP-0004`). `Database::generateNextId` finds the current maximum numeric suffix with:

```sql
SELECT COALESCE(MAX(CAST(SUBSTR(id_column, 5) AS INTEGER)), 0) + 1 FROM table
```

and returns the next ID formatted with zero-padding.