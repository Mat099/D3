#include "Payment.h"

Payment::Payment(string id,
                 string patient,
                 double amount)
{
    paymentId = id;
    patientId = patient;
    this->amount = amount;
    paid = false;
}

void Payment::pay()
{
    paid = true;
}