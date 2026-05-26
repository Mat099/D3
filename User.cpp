#include "User.h"
#include <iostream>

User::User(const std::string& id, const std::string& name,
           const std::string& surname, const std::string& email)
    : id(id), name(name), surname(surname), email(email){
    // Empty constructor body
}

bool User::login(const std::string& credentials) {
    if (credentials == "wrong") {
        std::cerr << "Login failed: invalid credentials" << std::endl;
        return false;
    }

    std::cout << " User " << name << " logged in successfully" << std::endl;
    return true;
}

void User::logout() {
    std::cout << " User " << name << " logged out" << std::endl;
}

