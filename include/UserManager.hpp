#pragma once

#include <string>

class UserManager {
private:
    static const int MAX_USERS = 100;
   

    int m_userIDs[MAX_USERS];
    std::string m_usernames[MAX_USERS];
    std::string m_dates[MAX_USERS];
     std::string m_passwords[MAX_USERS];

    int m_count;

public:
    UserManager();

    int findOrCreateUser(const std::string& name);
    int loginUser(const std::string& name, const std::string& password);
int registerUser(const std::string& name, const std::string& password);

private:
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename);

    int findUser(const std::string& name);
    int generateNewID();
};