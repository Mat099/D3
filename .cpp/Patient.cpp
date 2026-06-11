#include "User.h"
#include "Patient.h"
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
