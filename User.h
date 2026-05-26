#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User {
private:
    string id;
    string name;
    string surname;
    string email;
    string phoneNumber;
    string passwordHash;

public:

    //constructor
    User(string& id, string& name, string& surname, string& email);
    
    // destructor 
    virtual ~User() = default;
    
    //UC 11
    void logout();

    //UC 3
    void ERStatusCheck();

    //UC
    void CustomerService();

    // getters
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    
    
    // setters
    void setEmail(const std::string& newEmail) { email = newEmail; }
};

#endif
