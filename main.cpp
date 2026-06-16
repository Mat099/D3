#include <iostream>
#include <string>
#include ".h\User.h"

int main(){

    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║                 HOSPITAL APP                     ║" << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;
    cout << "║                                                  ║" << endl;
    cout << "║  1. Login                                        ║" << endl;
    cout << "║  3. Check ER Crowding Status                     ║" << endl;
    cout << "║  4. View FAQ & Assistance                        ║" << endl;
    cout << "║  5. Exit                                         ║" << endl;
    cout << "║                                                  ║" << endl;
    cout << "╚══════════════════════════════════════════════════╝" << endl;
    cout << "\nEnter your choice (1-5): ";
    cin >> choice;

  switch (choice) {
    case 1: {
      login();
      patientMenu();
    }
    case 2 : {
      login();
    }
    
}
