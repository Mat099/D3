#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
using namespace std;

class Database;
class Payment {
private:
    string paymentId;
    string appointmentId;
    string patientId;

    double amount;
    string paymentMethod;
    string status;

public:
     Payment(const string& paymentId,
            const string& appointmentId,
            const string& patientId,
            double amount,
            const string& method,
            const string& status = "pending");

    bool processOnlinePayment(Database& db);

    void registerPendingPayment(Database& db);

    void cancel(Database& db);

    // getters

    string getPaymentId() const { return paymentId; }
    string getAppointmentId() const { return appointmentId; }
    string getPatientId() const { return patientId; }
    double getAmount() const { return amount; }
    string getMethod() const { return paymentMethod; }
    string getStatus() const { return status; }

    void setStatus(const string& s) { status = s; }
};

#endif
