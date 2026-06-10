-- =============================================================================
-- schema.sql
-- Hospital Application — Database Schema
-- Giuseppine & Co.
--
-- Run this file once to initialise the database structure.
-- To populate with sample data, run seed.sql afterwards.
--
-- Convention:
--   - All primary keys are TEXT in the format PREFIX-NNNN (e.g. USR-0001)
--   - Foreign keys are enforced via PRAGMA foreign_keys = ON (set at runtime)
--   - Timestamps are stored as TEXT in ISO 8601 format: YYYY-MM-DD
--   - Booleans are stored as INTEGER: 0 = false, 1 = true
-- =============================================================================

PRAGMA foreign_keys = ON;


-- =============================================================================
-- LAYER 1: BASE USER TABLE
-- Covers: UC 1 (Login), UC 11 (Logout), UC 12 (Password Change)
-- All human actors in the system inherit from this table.
-- Role determines which child table holds the extended attributes:
--   'patient'       -> patients
--   'doctor'        -> medical_staff + doctors
--   'nurse'         -> medical_staff + nurses
-- =============================================================================

CREATE TABLE IF NOT EXISTS users (
    user_id         TEXT PRIMARY KEY,           -- e.g. USR-0001
    name            TEXT NOT NULL,
    surname         TEXT NOT NULL,
    email           TEXT NOT NULL UNIQUE,
    phone_number    TEXT,
    password_hash   TEXT NOT NULL,
    role            TEXT NOT NULL               -- 'patient' | 'doctor' | 'nurse'
        CHECK (role IN ('patient', 'doctor', 'nurse'))
);


-- =============================================================================
-- LAYER 2: PATIENT
-- Covers: UC 2 (Appointment Booking), UC 4 (Payment),
--         UC 10 (View Medical Records)
-- One row per patient, references users.
-- =============================================================================

CREATE TABLE IF NOT EXISTS patients (
    patient_id      TEXT PRIMARY KEY
        REFERENCES users(user_id) ON DELETE CASCADE,
    insurance       TEXT                        -- insurance provider / policy number
);


-- =============================================================================
-- LAYER 2: MEDICAL STAFF (shared base for doctors and nurses)
-- Covers: UC 6 (Manage Availability), UC 7 (Hospitalization),
--         UC 8 (Triage), UC 9 (Prescriptions), UC 10 (View Medical Records)
-- =============================================================================

CREATE TABLE IF NOT EXISTS medical_staff (
    staff_id        TEXT PRIMARY KEY
        REFERENCES users(user_id) ON DELETE CASCADE,
    workpass_id     TEXT NOT NULL UNIQUE
);


-- =============================================================================
-- LAYER 3: DOCTOR (extends medical_staff)
-- Covers: UC 7, UC 9
-- =============================================================================

CREATE TABLE IF NOT EXISTS doctors (
    doctor_id       TEXT PRIMARY KEY
        REFERENCES medical_staff(staff_id) ON DELETE CASCADE,
    specialization  TEXT NOT NULL
);


-- =============================================================================
-- LAYER 3: NURSE (extends medical_staff)
-- No additional attributes beyond medical_staff.
-- Included as a table for schema completeness and future extensibility.
-- =============================================================================

CREATE TABLE IF NOT EXISTS nurses (
    nurse_id        TEXT PRIMARY KEY
        REFERENCES medical_staff(staff_id) ON DELETE CASCADE
);


-- =============================================================================
-- MEDICAL RECORDS
-- Covers: UC 10 (View Medical Records)
-- One record per patient. Aggregates prescriptions and hospitalizations.
-- =============================================================================

CREATE TABLE IF NOT EXISTS medical_records (
    record_id       TEXT PRIMARY KEY,           -- e.g. REC-0001
    patient_id      TEXT NOT NULL UNIQUE
        REFERENCES patients(patient_id) ON DELETE CASCADE,
    allergies       TEXT,                       -- comma-separated list
    medical_history TEXT                        -- free-text summary
);


-- =============================================================================
-- PRESCRIPTIONS
-- Covers: UC 9 (Handling of Prescriptions)
-- Many prescriptions per medical record.
-- Managed by doctors; readable by medical staff and the patient.
-- =============================================================================

CREATE TABLE IF NOT EXISTS prescriptions (
    prescription_id TEXT PRIMARY KEY,           -- e.g. PRX-0001
    record_id       TEXT NOT NULL
        REFERENCES medical_records(record_id) ON DELETE CASCADE,
    medicine_name   TEXT NOT NULL,
    dosage          TEXT NOT NULL,              -- e.g. "500mg twice daily"
    description     TEXT,                       -- doctor's free-text notes
    active          INTEGER NOT NULL DEFAULT 1  -- 1 = active, 0 = cancelled
        CHECK (active IN (0, 1))
);


-- =============================================================================
-- HOSPITALIZATIONS
-- Covers: UC 7 (Management of Hospitalization)
-- Many hospitalization episodes per medical record.
-- Regional constraint (same-region transfers only) is enforced at
-- the application layer in Hospitalization::transfer(), not here.
-- =============================================================================

CREATE TABLE IF NOT EXISTS hospitalizations (
    hospitalization_id  TEXT PRIMARY KEY,       -- e.g. HSP-0001
    record_id           TEXT NOT NULL
        REFERENCES medical_records(record_id) ON DELETE CASCADE,
    patient_id          TEXT NOT NULL
        REFERENCES patients(patient_id),
    department          TEXT NOT NULL,
    hospital_name       TEXT NOT NULL,
    bed_id              TEXT NOT NULL,
    admission_date      TEXT NOT NULL,          -- YYYY-MM-DD
    discharge_date      TEXT,                   -- NULL if still admitted
    discharged          INTEGER NOT NULL DEFAULT 0
        CHECK (discharged IN (0, 1))
);


-- =============================================================================
-- APPOINTMENTS
-- Covers: UC 2 (Appointment Booking)
-- Stub table — implemented by a different team member.
-- Defined here so foreign key references from other tables resolve correctly.
-- =============================================================================

CREATE TABLE IF NOT EXISTS appointments (
    appointment_id  TEXT PRIMARY KEY,           -- e.g. APT-0001
    patient_id      TEXT NOT NULL
        REFERENCES patients(patient_id) ON DELETE CASCADE,
    doctor_id       TEXT NOT NULL
        REFERENCES doctors(doctor_id),
    date            TEXT NOT NULL,              -- YYYY-MM-DD
    time            TEXT NOT NULL,              -- HH:MM
    status          TEXT NOT NULL DEFAULT 'pending'
        CHECK (status IN ('pending', 'confirmed', 'cancelled', 'completed'))
);


-- =============================================================================
-- SCHEDULES
-- Covers: UC 6 (Management of Availability)
-- Stub table — implemented by a different team member.
-- =============================================================================

CREATE TABLE IF NOT EXISTS schedules (
    slot_id         TEXT PRIMARY KEY,           -- e.g. SCH-0001
    doctor_id       TEXT NOT NULL
        REFERENCES doctors(doctor_id) ON DELETE CASCADE,
    date            TEXT NOT NULL,              -- YYYY-MM-DD
    time_slot       TEXT NOT NULL,              -- HH:MM
    available       INTEGER NOT NULL DEFAULT 1
        CHECK (available IN (0, 1))
);


-- =============================================================================
-- TRIAGE
-- Covers: UC 8 (Triage Update), UC 3 (ER Line Check)
-- Stub table — implemented by a different team member.
-- =============================================================================

CREATE TABLE IF NOT EXISTS triage (
    triage_id       TEXT PRIMARY KEY,           -- e.g. TRG-0001
    patient_id      TEXT NOT NULL
        REFERENCES patients(patient_id) ON DELETE CASCADE,
    code            TEXT NOT NULL               -- 'red'|'orange'|'azure'|'green'|'white'
        CHECK (code IN ('red', 'orange', 'azure', 'green', 'white')),
    diagnosis       TEXT,
    admitted_at     TEXT NOT NULL,              -- YYYY-MM-DD HH:MM
    discharged      INTEGER NOT NULL DEFAULT 0
        CHECK (discharged IN (0, 1))
);