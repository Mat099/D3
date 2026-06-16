# D3 — Giuseppine & Co.
 
## Setup and running
 
### Step 1 — Get the SQLite amalgamation
 
The project uses SQLite via the [amalgamation](https://www.sqlite.org/amalgamation.html): two files (`sqlite3.c` and `sqlite3.h`) that you place in the project root. They are not included in the repository.
 
1. Go to https://www.sqlite.org/download.html
2. Under *Source Code*, download `sqlite-amalgamation-XXXXXXXX.zip` (the number is the version)
3. Extract `sqlite3.c` and `sqlite3.h` into the project root (same folder as `test.cpp`)
---
 
### Step 2 — Configure terminal encoding (Windows only)
 
The UI uses box-drawing characters (╔ ║ ╚ etc.). Without explicit UTF-8 encoding these render as garbage in the Windows terminal. Run these two commands **once per PowerShell session before compiling or running**:
 
```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
chcp 65001
```
 
Both are required — either alone is not enough.
 
---
 
### Step 3 — Compile
 
SQLite must be compiled as C, not C++. Compile it separately first:
 
```bash
gcc -c data/sqlite3.c -o data/sqlite3.o
```
 
Then compile the project with G++. Use `test.cpp` for the test suite or `main.cpp` for the interactive app — the list of `.cpp` files is the same either way:
 
```powershell
g++ -I.h -Idata test.cpp .cpp/Doctor.cpp .cpp/Nurse.cpp .cpp/User.cpp `
    .cpp/Prescription.cpp .cpp/Hospitalization.cpp .cpp/MedicalRecord.cpp `
    .cpp/Database.cpp .cpp/Schedule.cpp .cpp/Patient.cpp .cpp/Payment.cpp `
    .cpp/Appointment.cpp .cpp/Triage.cpp .cpp/Assistance.cpp  data/sqlite3.o -o test
```
 
```powershell
g++ -I.h -Idata main.cpp .cpp/Doctor.cpp .cpp/Nurse.cpp .cpp/User.cpp `
    .cpp/Prescription.cpp .cpp/Hospitalization.cpp .cpp/MedicalRecord.cpp `
    .cpp/Database.cpp .cpp/Schedule.cpp .cpp/Patient.cpp .cpp/Payment.cpp `
    .cpp/Appointment.cpp .cpp/Triage.cpp .cpp/Assistance.cpp data/sqlite3.o -o app
```
 
> `-I.h` tells G++ to search the `.h/` directory for project headers.  
> `-Idata` adds the `data/` folder so `sqlite3.h` is found.  
> The backtick `` ` `` is the PowerShell line-continuation character (`\` on bash).
 
---
 
### Step 4 — Run
 
**Test suite:**
 
```powershell
./test
```
 
Or with a pass/fail summary:
 
```powershell
./test; if ($?) { "ALL TESTS PASSED" } else { "SOME TESTS FAILED" }
```
 
Expected output ends with:
 
```
=========================================
111/111 checks passed.
=========================================
```
 
**Interactive app:**
 
```powershell
./app
```
 
On first run (`data/hospital.db` does not exist yet), the app automatically creates and seeds the database. On later runs it opens the existing file as-is, so data entered through the app persists across sessions.
 
To log in, use the user IDs and credentials from `data/seed.sql`. Passwords are the placeholder strings stored there — for example, for `USR-0004` (Dr. Elena Ricci) the workpass is `WP-DOC-0041` and the password is `$2b$10$placeholder_hash_004`.