#include "UserManager.hpp"
#include <fstream>
#include <iostream>
#include <ctime>

UserManager::UserManager() : m_count(0) {
    loadFromFile("users.txt");
    
}

// 🔹 Get current date
std::string getCurrentDateUser() {
    time_t now = time(0);
    tm* ltm = localtime(&now);

    char buffer[11];
    sprintf(buffer, "%04d-%02d-%02d",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon,
            ltm->tm_mday);

    return std::string(buffer);
}


// 🔹 Load users
void UserManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    

    if (!file.is_open()) return;

    char line[200];
    while (file.getline(line, 200))
{
    if (m_count >= MAX_USERS) break;

    int id;
    char name[100];
    char password[100];
    char date[20];

    int result = sscanf(line, "%d,%99[^,],%99[^,],%19s", &id, name, password, date);

    if (result == 4)

    {
        std::cout << "Loaded user: " << name << "\n"; 
        m_userIDs[m_count] = id;
        m_usernames[m_count] = name;
        m_passwords[m_count] = password;
        m_dates[m_count] = date;

        m_count++;
    }
    else
    {
        std::cout << "⚠ Skipping bad line: " << line << "\n";
    }
}
std::cout << "Total loaded: " << m_count << "\n";

    
}


// 🔹 Save users
void UserManager::saveToFile(const std::string& filename) {
    std::ofstream file(filename);

    for (int i = 0; i < m_count; i++) {
        file << m_userIDs[i] << ","
     << m_usernames[i] << ","
     << m_passwords[i] << ","
     << m_dates[i] << "\n";
    }

    file.close();
}


// 🔹 Find user
int UserManager::findUser(const std::string& name) {
    for (int i = 0; i < m_count; i++) {
        if (m_usernames[i] == name) {
            return m_userIDs[i];
        }
    }
    return -1;
}


// 🔹 Generate new ID
int UserManager::generateNewID() {
    int maxID = 0;

    for (int i = 0; i < m_count; i++) {
        if (m_userIDs[i] > maxID) {
            maxID = m_userIDs[i];
        }
    }

    return maxID + 1;
}


// 🔹 Main function
int UserManager::findOrCreateUser(const std::string& name) {
    int id = findUser(name);

    if (id != -1) {
        std::cout << "User found! ID: " << id << "\n";
        return id;
    }

    // Create new user
    int newID = generateNewID();

    if (m_count < MAX_USERS) {
        m_userIDs[m_count] = newID;
        m_usernames[m_count] = name;
        m_dates[m_count] = getCurrentDateUser();

        m_count++;

        saveToFile("users.txt");

        std::cout << "New user created! ID: " << newID << "\n";

        return newID;
    }

    return -1;
}
int UserManager::loginUser(const std::string& name, const std::string& password) {
    for (int i = 0; i < m_count; i++) {
        if (m_usernames[i] == name && m_passwords[i] == password) {
            return m_userIDs[i];
        }
    }
    return -1;
}
int UserManager::registerUser(const std::string& name, const std::string& password) {
if (name.empty() || password.empty())
{
    std::cout << "Invalid input!\n";
    return -1;
}
    // check if user already exists
    for (int i = 0; i < m_count; i++) {
        if (m_usernames[i] == name) {
            std::cout << "User already exists!\n";
            return -1;
        }
    }

    int newID = generateNewID();

    if (m_count < MAX_USERS) {
        m_userIDs[m_count] = newID;
        m_usernames[m_count] = name;
        m_passwords[m_count] = password;
        m_dates[m_count] = getCurrentDateUser();

        m_count++;

        saveToFile("users.txt");

        std::cout << "User registered! ID: " << newID << "\n";
        return newID;
    }

    return -1;
}