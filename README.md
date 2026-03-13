# 聊天室项目

这是一个基于C++和MySQL的聊天室项目，使用HTTP协议实现前后端通信，采用单线程selectIO多路复用技术处理并发连接。

## 功能特点

- 用户注册和登录
- 发送和接收消息
- 实时消息更新（短轮询技术）
- 跨平台支持（Windows和Linux）
- 数据库存储消息和用户信息
- 消息24小时自动过期清理
- 单线程selectIO多路复用处理并发连接
- 支持中文用户名和消息内容

## 技术栈

- **后端**：C++
- **数据库**：MySQL
- **前端**：HTML5, CSS3, JavaScript
- **网络**：Socket编程, HTTP协议, selectIO多路复用
- **实时通信**：短轮询（Short Polling）

## 技术实现细节

### 1. 网络模型

- **单线程selectIO多路复用**：使用`select`函数实现I/O多路复用，在单个线程中处理多个客户端连接，提高服务器的并发处理能力
- **跨平台Socket**：通过条件编译实现Windows和Linux平台的Socket兼容
- **HTTP服务器**：自定义实现简单的HTTP服务器，支持GET和POST请求

### 2. 消息更新机制

- **短轮询**：前端JavaScript每1秒向服务器发送一次请求，获取最新消息
- **API接口**：`/api/messages`接口返回JSON格式的消息数据
- **消息过滤**：只返回当前用户的消息，确保消息的私密性

### 3. 数据库设计

- **用户表**：存储用户信息，包括用户名、密码和IP地址
- **消息表**：存储消息内容，包括发送者、接收者、内容和时间戳
- **自动清理**：通过数据库定时任务或应用层清理函数，自动删除24小时前的过期消息

### 4. 安全措施

- **HTML转义**：对用户输入进行HTML转义，防止XSS攻击
- **输入验证**：验证用户名和密码的合法性
- **IP绑定**：用户登录后绑定IP地址，防止账号被盗用

## 项目结构

```
chatroom/
├── html/                 # HTML页面
│   ├── chat.html         # 聊天页面
│   └── login.html        # 登录/注册页面
├── include/              # 头文件
│   ├── database.h        # 数据库管理
│   ├── message.h         # 消息管理
│   ├── network.h         # 网络管理
│   ├── user.h            # 用户管理
│   ├── utils.h           # 工具函数
│   └── web.h             # Web请求处理
├── src/                  # 源文件
│   ├── database.cpp      # 数据库实现
│   ├── message.cpp       # 消息实现
│   ├── network.cpp       # 网络实现
│   ├── user.cpp          # 用户实现
│   ├── utils.cpp         # 工具函数实现
│   └── web.cpp           # Web请求处理实现
├── main.cpp              # 主入口文件
├── CMakeLists.txt        # CMake配置文件
├── init_database.sql     # 数据库初始化脚本
├── make.bat              # Windows编译脚本
├── clean.bat             # 清理脚本
└── README.md             # 项目说明
```

## 安装与运行

### 1. 环境要求

- C++编译器（支持C++11及以上）
- MySQL数据库
- CMake（可选，用于跨平台编译）

### 2. 数据库初始化

1. 创建数据库：`CREATE DATABASE chatroom;`
2. 执行初始化脚本：`mysql -u root -p chatroom < init_database.sql`

### 3. 编译项目

#### Windows

运行 `make.bat` 脚本编译项目：

```bash
make.bat
```

#### Linux

使用CMake编译：

```bash
mkdir build
cd build
cmake ..
make
```

### 4. 运行项目

1. 启动服务器：

```bash
chatroom.exe
```

2. 输入MySQL数据库密码

3. 服务器启动后，在浏览器中访问：`http://localhost:8888`

## 使用说明

1. **注册**：在登录页面点击"注册"按钮，输入用户名和密码（至少6位）进行注册
2. **登录**：输入用户名和密码进行登录
3. **发送消息**：在聊天页面输入接收者用户名和消息内容，点击"发送"按钮
4. **查看消息**：聊天页面会自动更新接收到的消息（每1秒更新一次）

## 数据库结构

### users表

| 字段名 | 数据类型 | 描述 |
|-------|---------|------|
| id | INT | 用户ID（自增） |
| username | VARCHAR(50) | 用户名 |
| password | VARCHAR(50) | 密码 |
| ip | VARCHAR(20) | 用户IP地址 |

### messages表

| 字段名 | 数据类型 | 描述 |
|-------|---------|------|
| id | INT | 消息ID（自增） |
| from_user | VARCHAR(50) | 发送者用户名 |
| to_user | VARCHAR(50) | 接收者用户名 |
| content | TEXT | 消息内容 |
| timestamp | TIMESTAMP | 消息时间戳 |

## 核心代码解析

### 1. 主循环（main.cpp）

使用`select`函数实现I/O多路复用，处理服务器套接字和客户端套接字的事件：

```cpp
// 主循环
while (g_running) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);
    
    // 将所有客户端套接字添加到readSet
    int maxSocket = serverSocket;
    {
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (const ClientConnection& conn : clientConnections) {
            FD_SET(conn.socket, &readSet);
            if (conn.socket > maxSocket) {
                maxSocket = conn.socket;
            }
        }
    }
    
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    // 调用select函数
    int result = select(maxSocket + 1, &readSet, nullptr, nullptr, &timeout);
    
    // 处理新连接和客户端请求
    // ...
}
```

### 2. 短轮询实现（chat.html）

前端使用JavaScript实现短轮询，每1秒向服务器请求最新消息：

```javascript
// 短轮询函数
function shortPollMessages() {
    // 创建XMLHttpRequest对象
    let xhr = new XMLHttpRequest();
    
    // 发送GET请求到API接口
    xhr.open('GET', '/api/messages?username=' + encodeURIComponent(username) + '&lastCount=' + currentMessageCount, true);
    
    // 处理响应
    xhr.onload = function() {
        if (xhr.status === 200) {
            try {
                // 解析JSON响应
                const data = JSON.parse(xhr.responseText);
                // 更新消息显示
                updateMessages(data);
            } catch (e) {
                console.error('Error parsing JSON:', e);
            }
        }
        
        // 无论成功失败，等待1秒后继续下一次轮询
        setTimeout(shortPollMessages, 1000);
    };
    
    // 发送请求
    xhr.send();
}
```

### 3. 消息处理（message.cpp）

消息的存储和获取实现：

```cpp
void MessageManager::addMessage(const std::string& from, const std::string& to, const std::string& content) {
    // 插入新消息到数据库
    std::string insertQuery = "INSERT INTO messages (from_user, to_user, content) VALUES ('" + from + "', '" + to + "', '" + content + "')";
    g_databaseManager.executeUpdate(insertQuery);
}

std::vector<Message> MessageManager::getMessagesForUser(const std::string& username) {
    std::vector<Message> messages;
    // 查询用户的所有消息
    std::string query = "SELECT from_user, to_user, content, timestamp FROM messages WHERE to_user = '" + username + "' ORDER BY timestamp ASC";
    MYSQL_RES* result = g_databaseManager.executeQuery(query);
    // 处理查询结果
    // ...
    return messages;
}
```

## 安全注意事项

- 本项目使用明文存储密码，仅用于学习目的
- 生产环境中应使用密码哈希算法（如bcrypt）
- 应添加更多的输入验证和安全措施
- 应实现HTTPS加密传输

## 许可证

本项目仅供学习使用。