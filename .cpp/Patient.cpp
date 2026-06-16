#include "User.h"
#include "Patient.h"
#include "Payment.h"
#include "Database.h"
#include <iostream>
#include <string>
using namespace std;


Patient::Patient(const string& id, 
                const string& name, 
                const string& surname, 
                const string& email, 
                const string& phoneNumber, 
                const string& insurance, 
                const string& password)
    : User(id, name, surname, email, phoneNumber, password), insurance(insurance) {
    //appointments(nullptr), payments(nullptr), medicalRecord(nullptr)
}

void Patient::login(string& email, string &password, string& name) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  1. Login with CIE                               ║" << endl;
    cout << "║  2. Login with SPID                              ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    cout << "\nEnter your choice (1-2): ";
    int choice = 0;
    cin >> choice;   

    if (choice == 1) {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  You chose to login with CIE                     ║" << endl;
        cout << "║  Please enter your CIE credentials:              ║" << endl;
        cout << "║  E-mail:                                         ║" << endl;
        cout << "║  Password:                                       ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
    }else if (choice == 2) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  You chose to login with SPID                   ║" << endl;
        cout << "║  Please enter your SPID credentials:            ║" << endl; 
        cout << "║  E-mail:                                        ║" << endl;
        cout << "║  Password:                                      ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }else {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Your choice is invalid. Please enter 1 or 2.   ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }

    string credential1, credential2;
    cin >> credential1;
    cin >> credential2;

    if (credential1 != email || credential2 != password) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid credentials. Please try again.         ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }else{
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Login successful! Welcome, " << name << "!     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }

    
}

void Patient::viewMedicalRecord(MedicalRecord& record) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Medical Record                                  ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Record ID:  " << record.getRecordId()  << endl;
    cout << "║  Patient ID: " << record.getPatientId() << endl;
    cout << "║  Date:       " << record.getDate()      << endl;
    cout << "║  Diagnosis:  " << record.getDiagnosis() << endl;
    cout << "║  Notes:      " << record.getNotes()     << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Patient::viewPrescription(Prescription& prescription) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Prescription                                    ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  ID:         " << prescription.getPrescriptionId() << endl;
    cout << "║  Patient ID: " << prescription.getPatientId()      << endl;
    cout << "║  Medication: " << prescription.getMedication()     << endl;
    cout << "║  Dosage:     " << prescription.getDosage()         << endl;
    cout << "║  Frequency:  " << prescription.getFrequency()      << endl;
    cout << "║  Duration:   " << prescription.getDuration()       << endl;
    cout << "║  Issued:     " << prescription.getIssueDate()      << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void Patient::makePayment(Payment& payment) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Payment                                         ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Select payment option:                          ║" << endl;
    cout << "║  1. Online Payment                               ║" << endl;
    cout << "║  2. Pay in Person                                ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    int option;
    cin >> option; 

     if(option == 1)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Select Payment Method                           ║" << endl;
        cout << "║                                                  ║" << endl;
        cout << "║  1. Credit Card                                  ║" << endl;
        cout << "║  2. Debit Card                                   ║" << endl;
        cout << "║  3. PayPal                                       ║" << endl;
        cout << "║  4. pagoPA                                       ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;

        int method;
        cin >> method;

        string paymentInfo;

        cout << "Enter payment information: ";
        cin >> paymentInfo;

        if(paymentInfo.empty())
        {
            cout << "╔══════════════════════════════════════════════════╗" << endl;
            cout << "║  Invalid payment information.                    ║" << endl;
            cout << "║  Please try again.                               ║" << endl;
            cout << "╚══════════════════════════════════════════════════╝" << endl;
            return;
        }

        payment.pay();

        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Payment successful!                             ║" << endl;
        cout << "║  Appointment confirmed.                          ║" << endl;
        cout << "║  Confirmation e-mail sent.                       ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
    }
    else if(option == 2)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Appointment confirmed.                          ║" << endl;
        cout << "║  Payment registered as pending.                  ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
    }
    else
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid option selected.                        ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
    }
}
     
