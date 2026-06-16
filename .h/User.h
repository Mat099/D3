#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User {
protected:
    string id;
    string name;
    string surname;
    string email;
    string phoneNumber;
    string password;

public:
    //constructor
    User(const string& id, const string& name, const string& surname, 
        const string& email, const string& phoneNumber,const string& password);
    
    // destructor 
    virtual ~User() = default;

    
    //UC 11
    void logout();

    //UC 3
    void checkER();

    //UC 5
    void customerService();
    
    // getters
      string getId() const { return id; }
      string getName() const { return name; }
      string getEmail() const { return email; }
    
    
    // setters
    void setEmail(const   string& newEmail) { email = newEmail; }
};

#endif
