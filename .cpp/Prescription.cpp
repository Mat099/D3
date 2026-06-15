#include "Prescription.h"
#include "Database.h"
#include <string>
using namespace std;

Prescription::Prescription(const string& prescriptionId,
                            const string& recordId,
                            const string& medicineName,
                            const string& dosage,
                            const string& description,
                            bool active,
                            const string& doctorId,
                            const string& createdAt)
    : prescriptionId(prescriptionId), recordId(recordId),
      medicineName(medicineName), dosage(dosage), description(description),
      active(active), doctorId(doctorId), createdAt(createdAt),
      patientId(""), frequency(""), duration("") {
}

void Prescription::updateDosage(const string& newDosage, Database& db) {
    dosage = newDosage;
    db.updatePrescriptionDosage(prescriptionId, newDosage,
                                 Database::currentTimestamp());
}

void Prescription::cancel(Database& db) {
    active = false;
    db.cancelPrescription(prescriptionId, Database::currentTimestamp());
}
