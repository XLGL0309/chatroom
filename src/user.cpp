#include "../include/user.h"
#include "../include/database.h"

bool UserManager::registerUser(const std::string& username, const std::string& password, const std::string& ip) {
    // 使用更细粒度的锁，只在需要时加锁
    std::lock_guard<std::mutex> lock(userMutex);
    
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
    std::string insertQuery = "INSERT INTO users (username, password, ip) VALUES (?, ?, ?)";
    std::vector<std::string> insertParams = {username, password, ip};
    int rowsAffected = g_databaseManager.executePreparedUpdate(insertQuery, insertParams);
    return rowsAffected > 0;
}

bool UserManager::loginUser(const std::string& username, const std::string& password, const std::string& ip) {
    // 使用更细粒度的锁，只在需要时加锁
    std::lock_guard<std::mutex> lock(userMutex);
    
    // Validate username and password using prepared statement
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ? AND password = ?";
    std::vector<std::string> checkParams = {username, password};
    MYSQL_RES* result = g_databaseManager.executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // Username and password match, update IP using prepared statement
            std::string updateQuery = "UPDATE users SET ip = ? WHERE username = ?";
            std::vector<std::string> updateParams = {ip, username};
            g_databaseManager.executePreparedUpdate(updateQuery, updateParams);
            mysql_free_result(result);
            return true;
        }
        mysql_free_result(result);
    }
    // Invalid username or password
    return false;
}

bool UserManager::isValidUser(const std::string& username, const std::string& ip) {
    // 使用更细粒度的锁，只在需要时加锁
    std::lock_guard<std::mutex> lock(userMutex);
    
    // Validate user exists and IP matches using prepared statement
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ? AND ip = ?";
    std::vector<std::string> checkParams = {username, ip};
    MYSQL_RES* result = g_databaseManager.executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        bool isValid = row && atoi(row[0]) > 0;
        mysql_free_result(result);
        return isValid;
    }
    return false;
}

bool UserManager::userExists(const std::string& username) {
    // 使用更细粒度的锁，只在需要时加锁
    std::lock_guard<std::mutex> lock(userMutex);
    
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