#include "User.h"
#include "Nurse.h"
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;


Nurse::Nurse(const string& id, 
            const string& name, 
            const string& surname, 
            const string& email, 
            const string& phoneNumber, 
            const string& password)
    : User(id, name, surname, email, phoneNumber, password) {
}

void Nurse::workLogin(string& workPass, string& password, string& name) {
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Please enter your work credentials:             ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  workPass:                                       ║" << endl;
    cout << "║  Password:                                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string credential1, credential2;
    cin >> credential1;
    cin >> credential2;

    if (credential1 != workPass || credential2 != password) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Invalid credentials. Please try again.         ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }
              
    else{
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Login successful! Welcome, " << name << "!     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }
}

void Nurse::changePassword(string& password){
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║  Please enter your new password. It must be at   ║" << endl;
    cout << "║  least 10 characters long, using  at  least      ║" << endl;
    cout << "║  one uppercase character, one lowercase          ║" << endl;
    cout << "║  character, one digit, and one special character.║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  New password:                                   ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;

    string newPassword;
    cin >> newPassword;

    if (newPassword.length() < 10) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must be at least 10 characters long.  ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::isupper)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one uppercase   ║" << endl;
        cout << "║  character.                                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::islower)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one lowercase   ║" << endl;
        cout << "║  character.                                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::isdigit)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one digit.      ║" << endl;
        cout << "║                                                 ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else if (!any_of(newPassword.begin(), newPassword.end(), ::ispunct)) {
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password must contain at least one special     ║" << endl;
        cout << "║  character.                                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
        return;
    }
    else {
        password = newPassword;
        cout << "╔═════════════════════════════════════════════════╗" << endl;
        cout << "║  Password changed successfully!                 ║" << endl;
        cout << "╚═════════════════════════════════════════════════╝" << endl;
    }

}

