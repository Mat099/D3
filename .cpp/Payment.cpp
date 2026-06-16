#include "Payment.h"
#include "Database.h"
#include <iostream>
using namespace std;

Payment::Payment(const string& paymentId,
                  const string& appointmentId,
                  const string& patientId,
                  double amount,
                  const string& method,
                  const string& status)
    : paymentId(paymentId), appointmentId(appointmentId), patientId(patientId),
      amount(amount), paymentMethod(method), status(status) {
}

bool Payment::processOnlinePayment(Database& db) {
    status = "paid";
    bool ok = db.insertPayment(paymentId, appointmentId, patientId, amount,
                                paymentMethod, status);
    if (!ok) status = "failed";
    return ok;
}

void Payment::registerPendingPayment(Database& db) {
    status = "pending";
    db.insertPayment(paymentId, appointmentId, patientId, amount, paymentMethod, status);
}

void Payment::cancel(Database& db) {
    status = "cancelled";
    db.updatePaymentStatus(paymentId, status);
}
