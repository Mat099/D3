-- =============================================================================
-- seed.sql
-- Hospital Application — Sample Data
-- Giuseppine & Co.
--
-- Run this file after schema.sql to populate the database with test data.
-- Safe to re-run after a DELETE FROM on all tables (run in reverse FK order).
--
-- Covers minimum data needed to test all use cases:
--   UC 1  Login                  -> 5 users with hashed passwords
--   UC 2  Appointment Booking    -> 3 appointments
--   UC 3  ER Line Check          -> 4 triage entries
--   UC 4  Payment                -> linked to appointments
--   UC 6  Manage Availability    -> 4 schedule slots
--   UC 7  Hospitalization        -> 2 active, 1 discharged
--   UC 8  Triage Update          -> 4 triage entries
--   UC 9  Prescriptions          -> 6 prescriptions across 3 records
--   UC 10 View Medical Records   -> 3 records
--   UC 11 Logout                 -> any logged-in user
--   UC 12 Password Change        -> any logged-in user
--
-- Note: password_hash values below represent plaintext "password123"
-- hashed with a placeholder. Replace with real bcrypt hashes in production.
-- =============================================================================

PRAGMA foreign_keys = ON;


-- =============================================================================
-- USERS (base table — insert all roles here first)
-- =============================================================================

INSERT INTO users (user_id, name, surname, email, phone_number, password_hash, role) VALUES
    -- Patients
    ('USR-0001', 'Marco',    'Bianchi',   'marco.bianchi@email.it',   '+39 333 1234567', '$2b$10$placeholder_hash_001', 'patient'),
    ('USR-0002', 'Giulia',   'Ferretti',  'giulia.ferretti@email.it', '+39 333 2345678', '$2b$10$placeholder_hash_002', 'patient'),
    ('USR-0003', 'Luca',     'Moretti',   'luca.moretti@email.it',    '+39 333 3456789', '$2b$10$placeholder_hash_003', 'patient'),
    -- Doctors
    ('USR-0004', 'Elena',    'Ricci',     'elena.ricci@hospital.it',  '+39 335 4567890', '$2b$10$placeholder_hash_004', 'doctor'),
    ('USR-0005', 'Andrea',   'Conti',     'andrea.conti@hospital.it', '+39 335 5678901', '$2b$10$placeholder_hash_005', 'doctor'),
    -- Nurse
    ('USR-0006', 'Sara',     'Lombardi',  'sara.lombardi@hospital.it','+39 336 6789012', '$2b$10$placeholder_hash_006', 'nurse');


-- =============================================================================
-- PATIENTS
-- =============================================================================

INSERT INTO patients (patient_id, insurance) VALUES
    ('USR-0001', 'Unipol — Policy #UP-8821'),
    ('USR-0002', 'Generali — Policy #GN-4453'),
    ('USR-0003', NULL);                         -- no insurance registered


-- =============================================================================
-- MEDICAL STAFF
-- =============================================================================

INSERT INTO medical_staff (staff_id, workpass_id) VALUES
    ('USR-0004', 'WP-DOC-0041'),
    ('USR-0005', 'WP-DOC-0042'),
    ('USR-0006', 'WP-NRS-0011');


-- =============================================================================
-- DOCTORS
-- =============================================================================

INSERT INTO doctors (doctor_id, specialization) VALUES
    ('USR-0004', 'Cardiology'),
    ('USR-0005', 'General Medicine');


-- =============================================================================
-- NURSES
-- =============================================================================

INSERT INTO nurses (nurse_id) VALUES
    ('USR-0006');


-- =============================================================================
-- MEDICAL RECORDS
-- One per patient
-- =============================================================================

INSERT INTO medical_records (record_id, patient_id, allergies, medical_history) VALUES
    ('REC-0001', 'USR-0001',
        'Penicillin, Aspirin',
        'Hypertension diagnosed 2019. Appendectomy 2021.'),

    ('REC-0002', 'USR-0002',
        'None known',
        'Type 2 diabetes diagnosed 2022. Under regular monitoring.'),

    ('REC-0003', 'USR-0003',
        'Latex',
        'No significant medical history.');


-- =============================================================================
-- PRESCRIPTIONS
-- Two per record (one active, one cancelled) to test UC 9 flows
-- =============================================================================

INSERT INTO prescriptions (prescription_id, record_id, medicine_name, dosage, description, active) VALUES
    -- Record 1 (Marco Bianchi)
    ('PRX-0001', 'REC-0001', 'Ramipril',      '5mg once daily',        'For hypertension management',           1),
    ('PRX-0002', 'REC-0001', 'Aspirin',        '100mg once daily',      'Cancelled due to allergy confirmation', 0),

    -- Record 2 (Giulia Ferretti)
    ('PRX-0003', 'REC-0002', 'Metformin',      '500mg twice daily',     'First-line treatment for type 2 diabetes', 1),
    ('PRX-0004', 'REC-0002', 'Atorvastatin',   '20mg once at night',    'Cholesterol management',                1),

    -- Record 3 (Luca Moretti)
    ('PRX-0005', 'REC-0003', 'Amoxicillin',    '500mg three times daily','10-day course for bacterial infection', 1),
    ('PRX-0006', 'REC-0003', 'Ibuprofen',      '400mg as needed',       'Pain relief — max 3 times daily',       0);


-- =============================================================================
-- HOSPITALIZATIONS
-- Two active, one discharged — covers admit/transfer/discharge flows (UC 7)
-- =============================================================================

INSERT INTO hospitalizations
    (hospitalization_id, record_id, patient_id, department, hospital_name, bed_id, admission_date, discharge_date, discharged)
VALUES
    -- Active: Marco Bianchi in Cardiology
    ('HSP-0001', 'REC-0001', 'USR-0001',
        'Cardiology', 'Ospedale Santa Chiara', 'BED-204',
        '2024-11-10', NULL, 0),

    -- Active: Giulia Ferretti in Internal Medicine (transferred from General)
    ('HSP-0002', 'REC-0002', 'USR-0002',
        'Internal Medicine', 'Ospedale Santa Chiara', 'BED-118',
        '2024-11-14', NULL, 0),

    -- Discharged: Luca Moretti — past episode, for history display
    ('HSP-0003', 'REC-0003', 'USR-0003',
        'General Medicine', 'Ospedale Villa Igea', 'BED-305',
        '2024-10-01', '2024-10-05', 1);


-- =============================================================================
-- APPOINTMENTS (stub data — supports UC 2 and UC 4 testing)
-- =============================================================================

INSERT INTO appointments (appointment_id, patient_id, doctor_id, date, time, status) VALUES
    ('APT-0001', 'USR-0001', 'USR-0004', '2024-12-01', '09:00', 'confirmed'),
    ('APT-0002', 'USR-0002', 'USR-0005', '2024-12-03', '10:30', 'pending'),
    ('APT-0003', 'USR-0003', 'USR-0004', '2024-12-05', '14:00', 'cancelled');


-- =============================================================================
-- SCHEDULES (stub data — supports UC 6 testing)
-- =============================================================================

INSERT INTO schedules (slot_id, doctor_id, date, time_slot, available) VALUES
    ('SCH-0001', 'USR-0004', '2024-12-01', '09:00', 0),  -- booked (APT-0001)
    ('SCH-0002', 'USR-0004', '2024-12-01', '10:00', 1),
    ('SCH-0003', 'USR-0005', '2024-12-03', '10:30', 0),  -- booked (APT-0002)
    ('SCH-0004', 'USR-0005', '2024-12-03', '11:30', 1);


-- =============================================================================
-- TRIAGE (stub data — supports UC 3 and UC 8 testing)
-- Covers all five triage codes
-- =============================================================================

INSERT INTO triage (triage_id, patient_id, code, diagnosis, admitted_at, discharged) VALUES
    ('TRG-0001', 'USR-0003', 'red',    'Severe chest pain, suspected MI',    '2024-11-15 02:30', 0),
    ('TRG-0002', 'USR-0001', 'orange', 'High fever, breathing difficulties', '2024-11-15 08:45', 0),
    ('TRG-0003', 'USR-0002', 'green',  'Minor laceration on left hand',      '2024-11-15 11:00', 1),
    ('TRG-0004', 'USR-0003', 'white',  'Routine check-up request',           '2024-11-14 16:20', 1);
