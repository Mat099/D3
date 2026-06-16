-- =============================================================================
-- seed.sql
-- Hospital Application — Sample Data
-- Giuseppine & Co.
--
-- Run this file after schema.sql to populate the database with test data.
-- Safe to re-run after a DELETE FROM on all tables (run in reverse FK order).
-- =============================================================================

PRAGMA foreign_keys = ON;


-- =============================================================================
-- USERS
-- =============================================================================

INSERT INTO users (user_id, name, surname, email, phone_number, password_hash, role) VALUES
    ('USR-0001', 'Marco',    'Bianchi',   'marco.bianchi@email.it',   '+39 333 1234567', '$2b$10$placeholder_hash_001', 'patient'),
    ('USR-0002', 'Giulia',   'Ferretti',  'giulia.ferretti@email.it', '+39 333 2345678', '$2b$10$placeholder_hash_002', 'patient'),
    ('USR-0003', 'Luca',     'Moretti',   'luca.moretti@email.it',    '+39 333 3456789', '$2b$10$placeholder_hash_003', 'patient'),
    ('USR-0004', 'Elena',    'Ricci',     'elena.ricci@hospital.it',  '+39 335 4567890', '$2b$10$placeholder_hash_004', 'doctor'),
    ('USR-0005', 'Andrea',   'Conti',     'andrea.conti@hospital.it', '+39 335 5678901', '$2b$10$placeholder_hash_005', 'doctor'),
    ('USR-0006', 'Sara',     'Lombardi',  'sara.lombardi@hospital.it','+39 336 6789012', '$2b$10$placeholder_hash_006', 'nurse');


-- =============================================================================
-- PATIENTS
-- =============================================================================

INSERT INTO patients (patient_id, insurance) VALUES
    ('USR-0001', 'Unipol — Policy #UP-8821'),
    ('USR-0002', 'Generali — Policy #GN-4453'),
    ('USR-0003', NULL);


-- =============================================================================
-- MEDICAL STAFF
-- =============================================================================

INSERT INTO medical_staff (staff_id, workpass_id) VALUES
    ('USR-0004', 'WP-DOC-0041'),
    ('USR-0005', 'WP-DOC-0042'),
    ('USR-0006', 'WP-NRS-0011');


-- =============================================================================
-- HOSPITALS
-- Seeded before DOCTORS — doctors.hospital_name now references this table.
-- =============================================================================

INSERT INTO hospitals (hospital_name, region) VALUES
    ('Ospedale Santa Chiara', 'Trentino-Alto Adige'),
    ('Ospedale Villa Igea',   'Trentino-Alto Adige'),
    ('Ospedale San Raffaele', 'Lombardia');


-- =============================================================================
-- DOCTORS
-- =============================================================================

INSERT INTO doctors (doctor_id, specialization, hospital_name) VALUES
    ('USR-0004', 'Cardiology',       'Ospedale Santa Chiara'),
    ('USR-0005', 'General Medicine', 'Ospedale Santa Chiara');


-- =============================================================================
-- NURSES
-- =============================================================================

INSERT INTO nurses (nurse_id) VALUES
    ('USR-0006');


-- =============================================================================
-- MEDICAL RECORDS
-- =============================================================================

INSERT INTO medical_records (record_id, patient_id, allergies, medical_history, last_modified) VALUES
    ('REC-0001', 'USR-0001',
        'Penicillin, Aspirin',
        'Hypertension diagnosed 2019. Appendectomy 2021.',
        '2024-11-10 09:00'),

    ('REC-0002', 'USR-0002',
        'None known',
        'Type 2 diabetes diagnosed 2022. Under regular monitoring.',
        '2024-11-14 11:30'),

    ('REC-0003', 'USR-0003',
        'Latex',
        'No significant medical history.',
        '2024-10-01 08:00');


-- =============================================================================
-- PRESCRIPTIONS
-- =============================================================================

INSERT INTO prescriptions (prescription_id, record_id, medicine_name, dosage, description, active, doctor_id, created_at) VALUES
    ('PRX-0001', 'REC-0001', 'Ramipril',    '5mg once daily',        'For hypertension management',              1, 'USR-0004', '2024-11-10 09:15'),
    ('PRX-0002', 'REC-0001', 'Aspirin',     '100mg once daily',      'Cancelled due to allergy confirmation',    0, 'USR-0004', '2024-11-10 09:20'),
    ('PRX-0003', 'REC-0002', 'Metformin',   '500mg twice daily',     'First-line treatment for type 2 diabetes', 1, 'USR-0005', '2024-11-14 11:45'),
    ('PRX-0004', 'REC-0002', 'Atorvastatin','20mg once at night',    'Cholesterol management',                   1, 'USR-0005', '2024-11-14 12:00'),
    ('PRX-0005', 'REC-0003', 'Amoxicillin', '500mg three times daily','10-day course for bacterial infection',   1, 'USR-0004', '2024-10-01 08:30'),
    ('PRX-0006', 'REC-0003', 'Ibuprofen',   '400mg as needed',       'Pain relief — max 3 times daily',          0, 'USR-0004', '2024-10-01 08:35');


-- =============================================================================
-- HOSPITALIZATIONS
-- =============================================================================

INSERT INTO hospitalizations
    (hospitalization_id, record_id, patient_id, department, hospital_name,
     bed_id, admission_date, discharge_date, discharged, doctor_id, last_modified)
VALUES
    ('HSP-0001', 'REC-0001', 'USR-0001',
        'Cardiology', 'Ospedale Santa Chiara', 'BED-204',
        '2024-11-10', NULL, 0, 'USR-0004', '2024-11-10 10:00'),

    ('HSP-0002', 'REC-0002', 'USR-0002',
        'Internal Medicine', 'Ospedale Santa Chiara', 'BED-118',
        '2024-11-14', NULL, 0, 'USR-0005', '2024-11-14 13:00'),

    ('HSP-0003', 'REC-0003', 'USR-0003',
        'General Medicine', 'Ospedale Villa Igea', 'BED-305',
        '2024-10-01', '2024-10-05', 1, 'USR-0005', '2024-10-05 16:00');


-- =============================================================================
-- APPOINTMENTS
-- =============================================================================

INSERT INTO appointments (appointment_id, patient_id, doctor_id, date, time, status) VALUES
    ('APT-0001', 'USR-0001', 'USR-0004', '2024-12-01', '09:00', 'confirmed'),
    ('APT-0002', 'USR-0002', 'USR-0005', '2024-12-03', '10:30', 'pending'),
    ('APT-0003', 'USR-0003', 'USR-0004', '2024-12-05', '14:00', 'cancelled');


-- =============================================================================
-- SCHEDULES
-- =============================================================================

INSERT INTO schedules (slot_id, doctor_id, date, time_slot, available) VALUES
    ('SCH-0001', 'USR-0004', '2024-12-01', '09:00', 0),
    ('SCH-0002', 'USR-0004', '2024-12-01', '10:00', 1),
    ('SCH-0003', 'USR-0005', '2024-12-03', '10:30', 0),
    ('SCH-0004', 'USR-0005', '2024-12-03', '11:30', 1);


-- =============================================================================
-- TRIAGE
-- =============================================================================

INSERT INTO triage (triage_id, patient_id, hospital_name, code, diagnosis, admitted_at, discharged, discharged_at) VALUES
    ('TRG-0001', 'USR-0003', 'Ospedale Santa Chiara', 'Red',    'Severe chest pain, suspected MI',    '2024-11-15 02:30', 0, NULL),
    ('TRG-0002', 'USR-0001', 'Ospedale Santa Chiara', 'Orange', 'High fever, breathing difficulties', '2024-11-15 08:45', 0, NULL),
    ('TRG-0003', 'USR-0002', 'Ospedale Villa Igea',   'Green',  'Minor laceration on left hand',      '2024-11-15 11:00', 1, '2024-11-15 12:30'),
    ('TRG-0004', 'USR-0003', 'Ospedale Santa Chiara', 'White',  'Routine check-up request',           '2024-11-14 16:20', 1, '2024-11-14 17:00');
