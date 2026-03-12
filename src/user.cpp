#include "../include/user.h"
#include "../include/database.h"

bool UserManager::registerUser(const std::string& username, const std::string& password, const std::string& ip) {
    // Check if user already exists
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = '" + username + "'";
    MYSQL_RES* result = g_databaseManager.executeQuery(checkQuery);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // User already exists
            mysql_free_result(result);
            return false;
        }
        mysql_free_result(result);
    }
    
    // Insert new user
    std::string insertQuery = "INSERT INTO users (username, password, ip) VALUES ('" + username + "', '" + password + "', '" + ip + "')";
    int rowsAffected = g_databaseManager.executeUpdate(insertQuery);
    return rowsAffected > 0;
}

bool UserManager::loginUser(const std::string& username, const std::string& password, const std::string& ip) {
    // Validate username and password
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = '" + username + "' AND password = '" + password + "'";
    MYSQL_RES* result = g_databaseManager.executeQuery(checkQuery);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // Username and password match, update IP
            std::string updateQuery = "UPDATE users SET ip = '" + ip + "' WHERE username = '" + username + "'";
            g_databaseManager.executeUpdate(updateQuery);
            mysql_free_result(result);
            return true;
        }
        mysql_free_result(result);
    }
    // Invalid username or password
    return false;
}

bool UserManager::isValidUser(const std::string& username, const std::string& ip) {
    // Validate user exists and IP matches
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = '" + username + "' AND ip = '" + ip + "'";
    MYSQL_RES* result = g_databaseManager.executeQuery(checkQuery);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        bool isValid = row && atoi(row[0]) > 0;
        mysql_free_result(result);
        return isValid;
    }
    return false;
}

bool UserManager::userExists(const std::string& username) {
    // Check if user exists
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = '" + username + "'";
    MYSQL_RES* result = g_databaseManager.executeQuery(checkQuery);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        bool exists = row && atoi(row[0]) > 0;
        mysql_free_result(result);
        return exists;
    }
    return false;
}

// Global user manager instance
UserManager g_userManager;