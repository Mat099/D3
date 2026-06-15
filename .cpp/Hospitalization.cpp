#include "Hospitalization.h"
#include "Database.h"
#include <string>
using namespace std;

Hospitalization::Hospitalization(const string& hospitalizationId,
                                  const string& recordId,
                                  const string& patientId,
                                  const string& department,
                                  const string& hospitalName,
                                  const string& bedId,
                                  const string& admissionDate,
                                  const string& dischargeDate,
                                  bool discharged,
                                  const string& doctorId,
                                  const string& lastModified)
    : hospitalizationId(hospitalizationId), recordId(recordId),
      patientId(patientId), department(department), hospitalName(hospitalName),
      bedId(bedId), admissionDate(admissionDate), dischargeDate(dischargeDate),
      discharged(discharged), doctorId(doctorId), lastModified(lastModified),
      reason("") {
}

bool Hospitalization::transfer(const string& newDepartment,
                                const string& newHospital,
                                const string& newBedId,
                                Database& db) {
    string currentRegion = db.getHospitalRegion(hospitalName);
    string targetRegion  = db.getHospitalRegion(newHospital);

    if (currentRegion.empty() || targetRegion.empty() ||
        currentRegion != targetRegion) {
        return false;   // different regions — transfer blocked
    }

    string ts    = Database::currentTimestamp();
    string logId = db.generateNextId("transfer_log", "log_id", "LOG");

    db.insertTransferLog(logId, hospitalizationId,
                         department, hospitalName, bedId,
                         newDepartment, newHospital, newBedId,
                         ts);

    db.updateHospitalizationTransfer(hospitalizationId,
                                      newDepartment, newHospital, newBedId,
                                      ts);

    department   = newDepartment;
    hospitalName = newHospital;
    bedId        = newBedId;
    lastModified = ts;
    return true;
}

void Hospitalization::discharge(const string& date, Database& db) {
    string ts = Database::currentTimestamp();
    db.dischargeHospitalization(hospitalizationId, date, ts);
    dischargeDate = date;
    discharged    = true;
    lastModified  = ts;
}
