#include "User.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
 
 
struct FaqEntry {
    string category;
    string question;
    string answer;
};
 
static const vector<FaqEntry> FAQ_DB = {
    {
        "appointment booking",
        "How do I book an appointment?",
        "Log in, select 'Book an appointment', choose your preferred\n"
        "hospital, doctor and time slot, then confirm. You will\n"
        "receive a confirmation e-mail once the booking is complete."
    },
    {
        "appointment booking",
        "Can I reschedule or cancel an appointment?",
        "Yes. Go to 'My appointments', select the appointment and\n"
        "choose 'Reschedule' or 'Cancel'. Changes must be made at\n"
        "least 24 hours before the scheduled time."
    },
    {
        "appointment booking",
        "Will I get a refund if I cancel an online-paid appointment?",
        "Yes. Refunds for online payments are processed automatically\n"
        "within 24 hours of the cancellation."
    },
    {
        "payments",
        "What payment methods are accepted?",
        "Online payments can be made by credit card, debit card,\n"
        "PayPal or pagoPA. You may also choose to pay in cash\n"
        "directly at the hospital reception on the day of the visit."
    },
    {
        "payments",
        "Is my payment information stored securely?",
        "All transactions are handled by our external Payment Gateway.\n"
        "Card details are never stored on our servers."
    },
    {
        "prescriptions",
        "Where can I view my prescriptions?",
        "Your active and past prescriptions are visible inside your\n"
        "Medical Record. Log in and select 'My medical record'."
    },
    {
        "prescriptions",
        "Can I request a prescription renewal online?",
        "Prescription renewals must be authorised by your doctor.\n"
        "Book an appointment and your doctor will update your\n"
        "prescriptions during or after the visit."
    },
    {
        "login",
        "How do I log in as a patient?",
        "Patients log in using their CIE or SPID digital identity\n"
        "together with the password they set on first access."
    },
    {
        "login",
        "My session expired. What happened?",
        "For security reasons, patient sessions expire after 60 minutes\n"
        "of inactivity and staff sessions after 30 minutes.\n"
        "Simply log in again to continue."
    },
    {
        "password",
        "What are the password requirements?",
        "Passwords must be at least 10 characters long and contain\n"
        "at least one uppercase letter, one lowercase letter,\n"
        "one digit and one special character."
    },
    {
        "password",
        "How do I change my password?",
        "Go to account settings and select 'Change password'.\n"
        "You can change your password at most once every 10 days."
    },
    {
        "password",
        "I forgot my password. How can I recover it?",
        "On the login page select 'Forgot password'. The system will\n"
        "send a recovery link to your registered e-mail address."
    }
};
 
// Assistance contacts
 
static void showAssistanceContacts() {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Assistance Contacts                             ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  E-mail:    assistance@hospitalapp.it            ║" << endl;
    cout << "║  Telephone: +39 012 345 6789                     ║" << endl;
    cout << "║  Hours:     Mon-Fri 09:00 - 17:00                ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

 
void User::customerService() {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Assistance                                      ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  How can we help you?                            ║" << endl;
    cout << "║  1. FAQ                                          ║" << endl;
    cout << "║  2. Assistance contacts                          ║" << endl;
    cout << "║  3. Cancel                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    cout << "\nEnter your choice (1-3): ";
    int choice;
    cin >> choice;

    if (choice == 2) {
        showAssistanceContacts();
        return;
    }
 
    if (choice == 3) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Returning to main menu.                        ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    if (choice != 1) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid choice. Returning to main menu.        ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  FAQ — Select a category:                        ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  1. Appointment booking                          ║" << endl;
    cout << "║  2. Payments                                     ║" << endl;
    cout << "║  3. Prescriptions                                ║" << endl;
    cout << "║  4. Login                                        ║" << endl;
    cout << "║  5. Password                                     ║" << endl;
    cout << "║  6. Cancel                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    cout << "\nEnter your choice (1-6): ";
    int catChoice;
    cin >> catChoice;
 
    string selectedCategory;
    switch (catChoice) {
        case 1: selectedCategory = "appointment booking"; break;
        case 2: selectedCategory = "payments";            break;
        case 3: selectedCategory = "prescriptions";       break;
        case 4: selectedCategory = "login";               break;
        case 5: selectedCategory = "password";            break;
        case 6:
            cout << "╔═════════════════════════════════════════════════╗" << endl;
            cout << "║  Returning to main menu.                        ║" << endl;
            cout << "╚═════════════════════════════════════════════════╝" << endl;
            return;
        default:
            cout << "╔═════════════════════════════════════════════════╗" << endl;
            cout << "║  Invalid choice. Returning to main menu.        ║" << endl;
            cout << "╚═════════════════════════════════════════════════╝" << endl;
            return;
    }
 

    vector<const FaqEntry*> matches;
    for (const FaqEntry& e : FAQ_DB) {
        if (e.category == selectedCategory)
            matches.push_back(&e);
    }
 
    if (matches.empty()) {

        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  No FAQ entries found for this category.         ║" << endl;
        cout << "║  Please contact our assistance office:           ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        showAssistanceContacts();
        return;
    }
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  FAQ — " << selectedCategory                      << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;
    for (size_t i = 0; i < matches.size(); ++i) {
        cout << "║  Q" << (i + 1) << ". " << matches[i]->question << endl;
        cout << "╠══════════════════════════════════════════════════╣" << endl;
    }
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    cout << "\nSelect a question (1-" << matches.size() << "): ";
    int qChoice;
    cin >> qChoice;
 
    if (qChoice < 1 || qChoice > static_cast<int>(matches.size())) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid choice. Returning to main menu.        ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
 
    const FaqEntry* selected = matches[static_cast<size_t>(qChoice - 1)];
 
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  " << selected->question                          << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║  " << selected->answer                            << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Did this answer your question? (y/n):           ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
 
    char helpful;
    cin >> helpful;
 
    if (helpful == 'y' || helpful == 'Y') {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Glad we could help!                            ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    } else {

        cout << "╔══════════════════════════════════════════════════╗" << endl;
        cout << "║  We are sorry we could not fully help you.       ║" << endl;
        cout << "║  Please reach out to our assistance office:      ║" << endl;
        cout << "╚══════════════════════════════════════════════════╝" << endl;
        showAssistanceContacts();
    }
}
 
