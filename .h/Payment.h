#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
using namespace std;

class Payment {
private:
    string paymentId;
    string patientId;
    double amount;
    bool paid;

public:
    Payment(string id,
            string patient,
            double amount);

    void pay();
};

#endif