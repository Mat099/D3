#include "User.h"
#include <iostream>
using namespace std;

User::User(const string& id, 
          const string& name, 
          const string& surname, 
          const string& email, 
          const string& phoneNumber, 
          const string& password)
     : id(id), name(name), surname(surname), email(email), phoneNumber(phoneNumber), password(password){
}

//UC 11: logout 
void User:: logout() {

     cout << "╔═════════════════════════════════════════════════╗" << endl;
     cout << "║  Are you sure you want to logout? (y/n)         ║" << endl;
     cout << "╚═════════════════════════════════════════════════╝" << endl;

     char choice;
     cin >> choice;

     if (choice == 'y' || choice == 'Y') {
         cout << "╔═════════════════════════════════════════════════╗" << endl;
         cout << "║  You have been logged out. Goodbye!             ║" << endl;
         cout << "╚═════════════════════════════════════════════════╝" << endl;
     } else if (choice == 'n' || choice == 'N') {
         cout << "╔═════════════════════════════════════════════════╗" << endl;
         cout << "║  Logout cancelled. You are still logged in.     ║" << endl;
         cout << "╚═════════════════════════════════════════════════╝" << endl;
     } else {
         cout << "╔═════════════════════════════════════════════════╗" << endl;
         cout << "║  Invalid choice. Logout cancelled.              ║" << endl;
         cout << "╚═════════════════════════════════════════════════╝" << endl;
     }
}

//UC 3
void User::checkER(){

}

//UC 5
void User::customerService(){

}