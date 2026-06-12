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
--   - Timestamps are stored as TEXT in ISO 8601 format: YYYY-MM-DD HH:MM
--   - Booleans are stored as INTEGER: 0 = false, 1 = true
-- =============================================================================

PRAGMA foreign_keys = ON;


-- =============================================================================
-- LAYER 1: BASE USER TABLE
-- Covers: UC 1 (Login), UC 11 (Logout), UC 12 (Password Change)
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
-- =============================================================================

CREATE TABLE IF NOT EXISTS patients (
    patient_id      TEXT PRIMARY KEY
        REFERENCES users(user_id) ON DELETE CASCADE,
    insurance       TEXT
);


-- =============================================================================
-- LAYER 2: MEDICAL STAFF
-- =============================================================================

CREATE TABLE IF NOT EXISTS medical_staff (
    staff_id        TEXT PRIMARY KEY
        REFERENCES users(user_id) ON DELETE CASCADE,
    workpass_id     TEXT NOT NULL UNIQUE
);


-- =============================================================================
-- LAYER 3: DOCTOR
-- =============================================================================

CREATE TABLE IF NOT EXISTS doctors (
    doctor_id       TEXT PRIMARY KEY
        REFERENCES medical_staff(staff_id) ON DELETE CASCADE,
    specialization  TEXT NOT NULL
);


-- =============================================================================
-- LAYER 3: NURSE
-- =============================================================================

CREATE TABLE IF NOT EXISTS nurses (
    nurse_id        TEXT PRIMARY KEY
        REFERENCES medical_staff(staff_id) ON DELETE CASCADE
);


-- =============================================================================
-- MEDICAL RECORDS
-- Covers: UC 10 (View Medical Records)
-- last_modified updated on every write from C++.
-- =============================================================================

CREATE TABLE IF NOT EXISTS medical_records (
    record_id       TEXT PRIMARY KEY,
    patient_id      TEXT NOT NULL UNIQUE
        REFERENCES patients(patient_id) ON DELETE CASCADE,
    allergies       TEXT,                       -- comma-separated list
    medical_history TEXT,                       -- free-text summary
    last_modified   TEXT                        -- YYYY-MM-DD HH:MM
);


-- =============================================================================
-- PRESCRIPTIONS
-- Covers: UC 9 (Handling of Prescriptions)
-- doctor_id records accountability; created_at set once at issue time.
-- =============================================================================

CREATE TABLE IF NOT EXISTS prescriptions (
    prescription_id TEXT PRIMARY KEY,
    record_id       TEXT NOT NULL
        REFERENCES medical_records(record_id) ON DELETE CASCADE,
    medicine_name   TEXT NOT NULL,
    dosage          TEXT NOT NULL,
    description     TEXT,
    active          INTEGER NOT NULL DEFAULT 1
        CHECK (active IN (0, 1)),
    doctor_id       TEXT
        REFERENCES doctors(doctor_id),
    created_at      TEXT                        -- YYYY-MM-DD HH:MM
);


-- =============================================================================
-- HOSPITALS
-- Reference table required for the same-region transfer constraint (UC 7).
-- =============================================================================

CREATE TABLE IF NOT EXISTS hospitals (
    hospital_name   TEXT PRIMARY KEY,
    region          TEXT NOT NULL
);


-- =============================================================================
-- HOSPITALIZATIONS
-- Covers: UC 7 (Management of Hospitalization)
-- Regional transfer constraint enforced at application layer.
-- =============================================================================

CREATE TABLE IF NOT EXISTS hospitalizations (
    hospitalization_id  TEXT PRIMARY KEY,
    record_id           TEXT NOT NULL
        REFERENCES medical_records(record_id) ON DELETE CASCADE,
    patient_id          TEXT NOT NULL
        REFERENCES patients(patient_id),
    department          TEXT NOT NULL,
    hospital_name       TEXT NOT NULL
        REFERENCES hospitals(hospital_name),
    bed_id              TEXT NOT NULL,
    admission_date      TEXT NOT NULL,          -- YYYY-MM-DD
    discharge_date      TEXT,                   -- NULL if still admitted
    discharged          INTEGER NOT NULL DEFAULT 0
        CHECK (discharged IN (0, 1)),
    doctor_id           TEXT
        REFERENCES doctors(doctor_id),
    last_modified       TEXT                    -- YYYY-MM-DD HH:MM
);


-- =============================================================================
-- TRANSFER LOG
-- Covers: UC 7 (hospitalization history / audit trail)
-- One row per transfer event, capturing before/after location.
-- =============================================================================

CREATE TABLE IF NOT EXISTS transfer_log (
    log_id              TEXT PRIMARY KEY,       -- e.g. LOG-0001
    hospitalization_id  TEXT NOT NULL
        REFERENCES hospitalizations(hospitalization_id),
    from_department     TEXT,
    from_hospital       TEXT,
    from_bed_id         TEXT,
    to_department       TEXT,
    to_hospital         TEXT,
    to_bed_id           TEXT,
    transferred_at      TEXT NOT NULL           -- YYYY-MM-DD HH:MM
);


-- =============================================================================
-- APPOINTMENTS (stub — implemented by a different team member)
-- =============================================================================

CREATE TABLE IF NOT EXISTS appointments (
    appointment_id  TEXT PRIMARY KEY,
    patient_id      TEXT NOT NULL
        REFERENCES patients(patient_id) ON DELETE CASCADE,
    doctor_id       TEXT NOT NULL
        REFERENCES doctors(doctor_id),
    date            TEXT NOT NULL,
    time            TEXT NOT NULL,
    status          TEXT NOT NULL DEFAULT 'pending'
        CHECK (status IN ('pending', 'confirmed', 'cancelled', 'completed'))
);


-- =============================================================================
-- SCHEDULES (stub — implemented by a different team member)
-- =============================================================================

CREATE TABLE IF NOT EXISTS schedules (
    slot_id         TEXT PRIMARY KEY,
    doctor_id       TEXT NOT NULL
        REFERENCES doctors(doctor_id) ON DELETE CASCADE,
    date            TEXT NOT NULL,
    time_slot       TEXT NOT NULL,
    available       INTEGER NOT NULL DEFAULT 1
        CHECK (available IN (0, 1))
);


-- =============================================================================
-- TRIAGE (stub — implemented by a different team member)
-- =============================================================================

CREATE TABLE IF NOT EXISTS triage (
    triage_id       TEXT PRIMARY KEY,
    patient_id      TEXT NOT NULL
        REFERENCES patients(patient_id) ON DELETE CASCADE,
    code            TEXT NOT NULL
        CHECK (code IN ('red', 'orange', 'azure', 'green', 'white')),
    diagnosis       TEXT,
    admitted_at     TEXT NOT NULL,              -- YYYY-MM-DD HH:MM
    discharged      INTEGER NOT NULL DEFAULT 0
        CHECK (discharged IN (0, 1))
);


-- =============================================================================
-- INDEXES
-- =============================================================================

CREATE INDEX IF NOT EXISTS idx_prescriptions_record    ON prescriptions(record_id);
CREATE INDEX IF NOT EXISTS idx_hospitalizations_record ON hospitalizations(record_id);
CREATE INDEX IF NOT EXISTS idx_transfer_log_hosp       ON transfer_log(hospitalization_id);
