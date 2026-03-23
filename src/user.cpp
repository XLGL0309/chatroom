#include "../include/user.h"
#include "../include/database.h"

bool UserManager::registerUser(const std::string& username, const std::string& password) {
    
    // Check if user already exists using prepared statement
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ?";
    std::vector<std::string> checkParams = {username};
    MYSQL_RES* result = g_databaseManager.executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // User already exists
            mysql_free_result(result);
            return false;
        }
        mysql_free_result(result);
    }
    
    // Insert new user using prepared statement
    std::string insertQuery = "INSERT INTO users (username, password) VALUES (?, ?)";
    std::vector<std::string> insertParams = {username, password};
    int rowsAffected = g_databaseManager.executePreparedUpdate(insertQuery, insertParams);
    return rowsAffected > 0;
}

bool UserManager::loginUser(const std::string& username, const std::string& password) {
    
    // Validate username and password using prepared statement
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ? AND password = ?";
    std::vector<std::string> checkParams = {username, password};
    MYSQL_RES* result = g_databaseManager.executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // Username and password match
            mysql_free_result(result);
            return true;
        }
        mysql_free_result(result);
    }
    // Invalid username or password
    return false;
}

bool UserManager::userExists(const std::string& username) {
    
    // Check if user exists using prepared statement
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ?";
    std::vector<std::string> checkParams = {username};
    MYSQL_RES* result = g_databaseManager.executePreparedQuery(checkQuery, checkParams);
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