#ifndef USER_H
#define USER_H

#include <string>
#include <chrono>

class User {
private:
    std::string id;
    std::string name;
    std::string surname;
    std::string email;
    std::string phoneNumber;
    std::string passwordHash;

public:

    //constructor
    User(const std::string& id, const std::string& name, 
         const std::string& surname, const std::string& email);
    
    // destructor 
    virtual ~User() = default;
    
    //UC1
    virtual bool login(const std::string& credentials);
    
    //UC 11
    void logout();
    
    // getters
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    
    
    // setters
    void setEmail(const std::string& newEmail) { email = newEmail; }
};

#endif