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
{
    this->paymentId = paymentId;
    this->appointmentId = appointmentId;
    this->patientId = patientId;
    this->amount = amount;
    this->paymentMethod = method;
    this->status = status;
}
