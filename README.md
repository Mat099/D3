# D3

## Running the test

The test (`test.cpp`) exercises every public method in the project — not just
the database round-trip. It covers `Database` utilities, `User`/`Doctor`/`Nurse`/
`Patient` login and password flows, all of `Doctor`'s DB-aware mutators
(issuing/cancelling prescriptions, admitting/transferring/discharging
patients, managing schedule availability), `Patient`'s appointment-booking
and payment flows, `Triage`'s full E.R. admit/update/discharge/status/crowding
flow, and `MedicalRecord`, `Prescription`, `Hospitalization`, `Schedule`,
`Appointment`, and `Payment` directly.

Interactive methods that read from `cin` are exercised by redirecting `cin`
to a fixed string of input and capturing whatever they print to `cout`, so
each one gets a real `[PASS]`/`[FAIL]` assertion instead of just "didn't
crash". The test prints one line per check plus a final `N/M checks passed`
summary, and exits `0` only if every check passed — currently 111/111.

Real issues were found and fixed this way:
- `Doctor::admitPatient`/`Doctor::transferPatient` read free-text
  department/hospital-name fields with `cin >> field`, which stops at the
  first space — multi-word values like `"Ospedale Santa Chiara"` or
  `"General Medicine"` couldn't be entered correctly. Fixed by switching
  those two fields to `getline`.
- `MedicalRecord::addHospitalization` ignored the result of the SQL insert,
  so a hospitalization that failed the `hospital_name` foreign key still got
  added to the in-memory record — silently desyncing memory from the
  database. It now returns `bool`, and `Doctor::admitPatient` reports
  failure instead of claiming success.
- `MedicalRecord::dischargePatient` never checked whether a hospitalization
  was already discharged, so calling it twice just re-discharged with the
  new date both times. It now rejects the second call.
- **`Database.cpp` bound every `sqlite3_bind_text` with `SQLITE_STATIC` on
  values like `someEntity.getX().c_str()`, where `getX()` returns
  `std::string` *by value*.** The temporary is destroyed at the end of that
  expression, before `sqlite3_step()` ever runs — `SQLITE_STATIC` promises
  the pointer stays valid until then, which was false, so SQLite sometimes
  read freed memory. This showed up as a flaky, hard-to-reproduce
  `"FOREIGN KEY constraint failed"` on perfectly valid data (traced by
  adding the `logSqlError` calls below — once the real SQLite error message
  was visible instead of a bare `false`, it pointed straight at this). Fixed
  by switching every bind in `Database.cpp` to `SQLITE_TRANSIENT`, which
  tells SQLite to copy the bytes immediately. Also added
  `sqlite3_busy_timeout(db, 5000)` in `Database::open` as a defensive
  measure against transient file-lock contention (not the cause here, but
  cheap insurance), and `logSqlError` so any future write failure prints the
  real `sqlite3_errmsg()` instead of just returning `false`.
- `Patient`/`Appointment`/`Payment` (the booking + payment feature) needed
  several pieces to compile and actually work: `Patient.h` was missing
  `#include`s for `Appointment`/`Payment` and had a stale `makePayment`
  signature (`int&, int&` instead of `Appointment&, Payment&`); `Payment.cpp`
  had been overwritten with a stale copy of `Payment.h`'s text instead of an
  implementation; and `Database` was missing `insertAppointment`,
  `updateAppointmentStatus`, `updateAppointmentSchedule`,
  `updateAppointmentPaymentStatus`, `searchAvailableSlots`,
  `updateScheduleAvailability`, `insertPayment`, and `updatePaymentStatus`,
  backed by new `appointments.paid`/`last_modified` columns and a new
  `payments` table in `data/schema.sql`. `searchAvailableSlots`'s `hospital`
  filter is backed by a new `doctors.hospital_name` column (`REFERENCES
  hospitals(hospital_name)`, joined on `doctor_id`) — `hospitals` is now
  seeded before `doctors` in `data/seed.sql` since `doctors` references it.
- `Triage` (E.R. admission/triage) had several real bugs beyond missing
  `Database` methods: `Triage::admitPatient` referenced an undeclared `diag`
  variable (it never actually prompted for a diagnosis); `Triage.h` declared
  `getDiagnosis()` with no backing field or implementation; the constructor
  had no `diagnosis`/`discharged`/`dischargedAt` parameters, so a record
  loaded from the database couldn't be fully reconstructed; and the schema's
  `code` `CHECK` used lowercase values (`'red'`, ...) while the C++ code
  produces capitalized ones (`"Red"`, ...), which would have rejected every
  insert/update. Fixed all of it, added `insertTriageRecord`,
  `updateTriageCode`, `dischargeTriageRecord`, `loadTriageRecord`, and
  `loadActiveTriageByHospital` to `Database`, and added
  `triage.hospital_name`/`discharged_at`/`last_modified` columns — `triage`
  had no hospital column at all, but `checkERCrowding` needs to filter by
  one. Also switched the hospital-name and diagnosis prompts in
  `Triage.cpp` to `getline`, same reasoning as the `Doctor` fields above.

## Running the app

`main.cpp` is the interactive console entry point. It's pure menu
plumbing — login + an anonymous E.R.-crowding check at the top level, then
a role menu (Doctor / Nurse / Patient) built entirely out of methods that
already exist on those classes; no new business logic lives in `main.cpp`.

Login looks up the typed user ID in the database (real seeded users, not a
hardcoded mock), then calls that role's `workLogin()`/`login()` using the
looked-up `password_hash`/`workpass_id` as the value to check the typed
credentials against. Nothing in this project does real password hashing, so
"the password" is literally the placeholder string in `data/seed.sql` —
e.g. for `USR-0004` (Dr. Elena Ricci) the workpass is `WP-DOC-0041` and the
password is `$2b$10$placeholder_hash_004`.

On first run (`data/hospital.db` doesn't exist yet), it runs
`execSchema`/`execSeed` automatically. On later runs it opens the existing
file as-is, so data entered through the app actually persists across runs —
unlike `test.cpp`, which always wipes and reseeds.

`workLogin()`/`login()` used to be `void` and print success/failure without
ever telling the caller which happened — meaning nothing could actually gate
menu access on them. They now return `bool`, which `main.cpp` checks before
entering the role menu.

While wiring this up, two more real bugs surfaced:
- `Nurse.h` still declared `viewMedicalRecord`/`viewHospitalization`, but
  their implementations had dropped out of `Nurse.cpp` — restored them
  (identical to the same methods on `Doctor`).
- On this toolchain, a freshly-constructed `std::ifstream` on a *nonexistent*
  file reports `good() == true` even though `is_open() == false` — using
  `good()` to detect "does `data/hospital.db` exist yet" silently skipped
  schema/seed initialization on a fresh checkout (every query failed with
  `no such table: users`). Fixed by checking `is_open()` instead.

"View FAQ & Assistance" (UC 5) calls `User::customerService()`, implemented
in `.cpp/Assistance.cpp` as a real FAQ + assistance-contacts menu (not the
old no-op stub the name suggests) — `main.cpp` calls it on a throwaway
`User` instance, since the menu is generic and doesn't use any identity
fields. `.cpp/User.cpp` used to have its own empty `customerService()`
body, which collided with `Assistance.cpp`'s real one at link time
("multiple definition") — removed the stub now that the real implementation
exists. `Assistance.cpp` needs to be included in every build (`test` and
`app`) for the same reason.

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

Then compile the rest with G++ — `test.cpp` for the test suite, or
`main.cpp` for the interactive app (same `.cpp` file list either way, just
swap the first argument):

```powershell
g++ -I.h -Idata test.cpp .cpp/Doctor.cpp .cpp/Nurse.cpp .cpp/User.cpp `
    .cpp/Prescription.cpp .cpp/Hospitalization.cpp .cpp/MedicalRecord.cpp `
    .cpp/Database.cpp .cpp/Schedule.cpp .cpp/Patient.cpp .cpp/Payment.cpp `
    .cpp/Appointment.cpp .cpp/Triage.cpp .cpp/Assistance.cpp data/sqlite3.o -o test
```

```powershell
g++ -I.h -Idata main.cpp .cpp/Doctor.cpp .cpp/Nurse.cpp .cpp/User.cpp `
    .cpp/Prescription.cpp .cpp/Hospitalization.cpp .cpp/MedicalRecord.cpp `
    .cpp/Database.cpp .cpp/Schedule.cpp .cpp/Patient.cpp .cpp/Payment.cpp `
    .cpp/Appointment.cpp .cpp/Triage.cpp .cpp/Assistance.cpp data/sqlite3.o -o app
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

For the interactive app:

```powershell
./app
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