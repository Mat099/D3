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

void Patient::makePayment(Appointment& appointment, Payment& payment, Database& db) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Payment                                         ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Select payment option:                          ║" << endl;
    cout << "║  1. Online Payment                               ║" << endl;
    cout << "║  2. Pay in Person                                ║" << endl;
    cout << "║  0. Cancel                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    int option;
    cin >> option;

    if (option == 0)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Payment cancelled                               ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }

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
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Enter payment information:                      ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
       
        cin >> paymentInfo;

        if(paymentInfo.empty())
        {
            cout << "╔══════════════════════════════════════════════════╗" << endl;
            cout << "║  Invalid payment information.                    ║" << endl;
            cout << "║  Please try again.                               ║" << endl;
            cout << "╚══════════════════════════════════════════════════╝" << endl;
            return;
        }

        payment.setStatus("paid");
      
        db.updatePaymentStatus(payment.getPaymentId(), "paid");

        appointment.setPaid(true);
        appointment.setStatus("confirmed");
        appointment.confirm(db);

        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Payment successful!                             ║" << endl;
        cout << "║  Appointment confirmed.                          ║" << endl;
        cout << "║  Confirmation e-mail sent.                       ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
    }
    else if(option == 2)
    {  
        payment.setStatus("pending");

        db.insertPayment(
            payment.getPaymentId(),
            appointment.getAppointmentId(),
            payment.getPatientId(),
            payment.getAmount(),
            "cash",
            "pending"
        );

        appointment.setPaid(false);
        appointment.setStatus("confirmed");
      
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

void Patient::bookAppointment(Appointment& appointment){
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Appointment Booking                             ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  Search appointment by:                          ║" << endl;
    cout << "║  1. Date and Time                                ║" << endl;
    cout << "║  2. Facility                                     ║" << endl;
    cout << "║  3. Doctor                                       ║" << endl;
    cout << "║  0. Cancel                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    int choice;
    cin>> choice;

    if(choice == 0)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Appointment booking cancelled.                  ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }

    string date, hospital, doctorId;
    cin.ignore();
  
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Enter search criteria:                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    if (choice == 1)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Enter date (YYYY-MM-DD):                        ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        getline(cin, date);
    }
    else if (choice == 2)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Enter hospital name:                            ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl; 
        getline(cin, hospital);
    }
    else if (choice == 3)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Enter doctor ID:                                ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        getline(cin, doctorId);
    }
    else
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid selection.                              ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }

   vector<Schedule> slots = db.searchAvailableSlots(date, hospital, doctorId);

    if (slots.empty())
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  No available slots found.                       ║" << endl;
        cout << "║  Please try different criteria.                  ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }


    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║              AVAILABLE SLOTS                     ║" << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;

    for (size_t i = 0; i < slots.size(); i++)
    {
        cout << "║  " << (i + 1)
             << ". Doctor: " << slots[i].getDoctorId()
             << " | " << slots[i].getDate()
             << " " << slots[i].getTimeSlot();

        cout << string(8, ' ') << "║" << endl;
    }

    cout << "║  0. Cancel                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
  
    int slotChoice;
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Select slot:                                    ║" << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;

    cin >> slotChoice;

    if(slotChoice == 0)
    {
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Appointment booking cancelled.                  ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }

    if (slotChoice < 1 || slotChoice > (int)slots.size()){
        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid slot selection.                         ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        return;
    }

    Schedule selected = slots[slotChoice - 1];

    Appointment appointment(
        db.generateNextId("appointments", "appointment_id", "APT"),
        this->getId(),
        selected.getDoctorId(),
        selected.getDate(),
        selected.getTimeSlot()
    );

    appointment.setStatus("pending");

    db.insertAppointment(
        appointment.getAppointmentId(),
        appointment.getPatientId(),
        appointment.getDoctorId(),
        appointment.getDate(),
        appointment.getTime()
    );

    db.updateScheduleAvailability(selected.getSlotId(), false);
  
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║             APPOINTMENT CONFIRMED                ║" << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  Date:   " << selected.getDate() << endl;
    cout << "║  Time:   " << selected.getTimeSlot() << endl;
    cout << "║  Doctor: " << selected.getDoctorId() << endl;
    cout << "║  Status: CONFIRMED                               ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

}

