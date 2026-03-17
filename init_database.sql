create database if not exists chatroom;
use chatroom;
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL, -- 建议存储哈希后的密码
    ip VARCHAR(50) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS messages (
    id INT AUTO_INCREMENT PRIMARY KEY,
    from_user VARCHAR(50) NOT NULL,
    to_user VARCHAR(50) NOT NULL,
    content MEDIUMTEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (from_user) REFERENCES users(username) ON DELETE CASCADE,
    FOREIGN KEY (to_user) REFERENCES users(username) ON DELETE CASCADE
);
-- 创建事件，每天清理 24 小时前的消息
CREATE EVENT IF NOT EXISTS clean_expired_messages
ON SCHEDULE EVERY 1 HOUR
DO
    DELETE FROM messages WHERE timestamp < DATE_SUB(NOW(), INTERVAL 24 HOUR);
CREATE INDEX idx_messages_to_user ON messages(to_user);

DELETE FROM users;
ALTER TABLE users AUTO_INCREMENT = 1;