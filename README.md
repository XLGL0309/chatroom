# 聊天室服务器

## 项目简介

这是一个基于C++实现的聊天室服务器项目，提供了完整的用户注册、登录、消息发送和接收功能，并通过Web界面进行交互。项目采用了线程池技术处理并发连接，使用MySQL数据库存储用户信息和消息，支持跨平台运行（Windows和Linux）。

## 功能特性

- **用户注册**：支持新用户注册
- **用户登录**：支持用户登录验证
- **消息发送**：支持用户之间发送消息
- **消息接收**：支持用户接收消息
- **Web界面**：提供友好的Web界面，支持浏览器访问
- **线程池**：使用线程池处理并发连接，提高性能
- **跨平台**：支持Windows和Linux平台
- **安全**：使用参数化查询防止SQL注入
- **可配置**：支持通过配置文件修改服务器配置

## 项目结构

```
chatroom/
├── main.cpp          # 主程序入口
├── include/          # 头文件目录
│   ├── config.h      # 配置管理
│   ├── database.h    # 数据库管理
│   ├── message.h     # 消息管理
│   ├── network.h     # 网络通信
│   ├── threadpool.h  # 线程池
│   ├── user.h        # 用户管理
│   ├── utils.h       # 工具函数
│   └── web.h         # Web处理
├── src/              # 源代码目录
│   ├── config.cpp    # 配置管理实现
│   ├── database.cpp  # 数据库实现
│   ├── message.cpp   # 消息实现
│   ├── network.cpp   # 网络实现
│   ├── threadpool.cpp# 线程池实现
│   ├── user.cpp      # 用户实现
│   ├── utils.cpp     # 工具函数实现
│   └── web.cpp       # Web实现
├── html/             # 前端页面
│   ├── login.html    # 登录页面
│   └── chat.html     # 聊天页面
└── config.ini        # 配置文件
```

## 环境要求

- **C++编译器**：支持C++11或更高版本
- **MySQL**：5.7或更高版本
- **MySQL Connector/C**：用于C++连接MySQL数据库
- **Windows**：需要Winsock2库
- **Linux**：需要Socket库

## 安装步骤

### 1. 安装依赖

#### Windows
- 安装MySQL数据库
- 安装MySQL Connector/C
- 配置环境变量，确保编译器能找到MySQL头文件和库文件

#### Linux
- 安装MySQL数据库：`sudo apt-get install mysql-server`
- 安装MySQL开发库：`sudo apt-get install libmysqlclient-dev`

### 2. 创建数据库

```sql
CREATE DATABASE chatroom;
USE chatroom;

CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(50) NOT NULL,
    ip VARCHAR(20) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE messages (
    id INT AUTO_INCREMENT PRIMARY KEY,
    from_user VARCHAR(50) NOT NULL,
    to_user VARCHAR(50) NOT NULL,
    content TEXT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建定时清理过期消息的事件
CREATE EVENT IF NOT EXISTS clean_expired_messages
ON SCHEDULE EVERY 1 HOUR
DO
    DELETE FROM messages WHERE timestamp < DATE_SUB(NOW(), INTERVAL 24 HOUR);
```

### 3. 编译项目

#### Windows
- 使用Visual Studio打开项目
- 配置包含目录和库目录，确保能找到MySQL头文件和库文件
- 编译项目

#### Linux
```bash
g++ -std=c++11 -o chatroom_server main.cpp src/*.cpp -Iinclude -lmysqlclient -lpthread
```

## 配置文件

项目使用`config.ini`文件进行配置，包含以下配置项：

```ini
# Server Configuration
server_port=8888

# Database Configuration
db_host=localhost
db_user=root
db_name=chatroom
```

- `server_port`：服务器端口，默认为8888
- `db_host`：数据库主机，默认为localhost
- `db_user`：数据库用户名，默认为root
- `db_name`：数据库名称，默认为chatroom

## 使用方法

1. **启动服务器**

```bash
./chatroom_server
```

2. **输入数据库密码**

服务器启动时会提示输入数据库密码，输入后按回车继续。

3. **访问Web界面**

在浏览器中访问：`http://localhost:8888`

4. **注册用户**

在登录页面点击"注册"按钮，填写用户名和密码进行注册。

5. **登录用户**

在登录页面填写用户名和密码进行登录。

6. **发送消息**

登录后，在聊天页面填写接收者用户名和消息内容，点击"发送"按钮发送消息。

7. **接收消息**

聊天页面会自动轮询获取新消息，显示在消息列表中。

## 安全措施

- **SQL注入防护**：使用字符串转义防止SQL注入攻击
- **密码安全**：密码存储在数据库中（建议在生产环境中使用密码哈希）
- **用户验证**：确保用户只能在登录的IP上操作
- **HTML转义**：防止XSS攻击

## 性能优化

- **线程池**：使用线程池处理并发连接，提高性能
- **数据库索引**：在用户名字段上创建索引，提高查询速度
- **消息清理**：定期清理过期消息，减少数据库负担

## 注意事项

- 本项目仅用于学习和测试，不建议在生产环境中使用
- 在生产环境中，建议使用HTTPS加密传输
- 建议使用密码哈希算法存储密码，而不是明文存储
- 建议添加更多的错误处理和日志记录

## 许可证

本项目采用MIT许可证，详见LICENSE文件。
# 聊天室项目

这是一个基于C++和MySQL的聊天室项目，使用HTTP协议实现前后端通信，采用单线程selectIO多路复用技术处理并发连接。

## 功能特点

- 用户注册和登录
- 发送和接收消息
- 实时消息更新（短轮询技术）
- 跨平台支持（Windows和Linux）
- 数据库存储消息和用户信息
- 消息24小时自动过期清理
- 多线程并发处理（线程池技术）
- 智能线程数配置（根据CPU核心数自动调整）
- 支持中文用户名和消息内容

## 技术栈

- **后端**：C++
- **数据库**：MySQL
- **前端**：HTML5, CSS3, JavaScript
- **网络**：Socket编程, HTTP协议
- **并发处理**：线程池（生产者-消费者模式）
- **实时通信**：短轮询（Short Polling）

## 技术实现细节

### 1. 网络模型

- **多线程并发处理**：使用线程池技术，充分利用多核CPU性能
- **生产者-消费者模式**：主线程负责接受新连接，工作线程负责处理请求
- **跨平台Socket**：通过条件编译实现Windows和Linux平台的Socket兼容
- **HTTP服务器**：自定义实现简单的HTTP服务器，支持GET和POST请求

### 2. 线程池实现

- **智能线程数配置**：根据CPU核心数自动调整线程数（核心数×2）
- **任务队列**：使用线程安全的队列存储客户端连接
- **条件变量**：实现工作线程的高效唤醒机制
- **线程安全**：使用互斥锁和原子变量确保线程安全

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
│   ├── threadpool.h      # 线程池管理
│   ├── user.h            # 用户管理
│   ├── utils.h           # 工具函数
│   └── web.h             # Web请求处理
├── src/                  # 源文件
│   ├── database.cpp      # 数据库实现
│   ├── message.cpp       # 消息实现
│   ├── network.cpp       # 网络实现
│   ├── threadpool.cpp    # 线程池实现
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

主线程负责接受新连接，并将连接添加到线程池：

```cpp
// 主循环 - 只处理新连接
while (g_running) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);
    
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    int result = select(serverSocket + 1, &readSet, nullptr, nullptr, &timeout);
    
    if (result == 0) continue;
    
    // 检查服务器套接字是否有新连接
    if (FD_ISSET(serverSocket, &readSet)) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket != INVALID_SOCKET) {
            std::string clientIP = inet_ntoa(clientAddr.sin_addr);
            std::cout << "New connection from " << clientIP << std::endl;
            
            // 将新客户端添加到线程池
            g_threadPool.addTask(clientSocket, clientIP);
        }
    }
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

### 2. 线程池实现（threadpool.cpp）

使用生产者-消费者模式实现线程池：

```cpp
// 工作线程循环
void ThreadPool::workerLoop() {
    while (m_running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            // 等待，直到队列不为空 或 停止运行
            m_cv.wait(lock, [this] {  
                return !m_taskQueue.empty() || !m_running;  
            });
            
            if (!m_running && m_taskQueue.empty()) return;
            
            // 取出任务
            task = m_taskQueue.front();
            m_taskQueue.pop();
        } // 锁在这里释放，处理任务时不持有锁
        
        // 处理任务（锁外执行）
        std::cout << "Thread handling connection from " << task.ip << std::endl;
        handleClientConnection(task.socket, task.ip);
        closesocket(task.socket);
        std::cout << "Thread finished connection from " << task.ip << std::endl;
    }
}

// 添加任务到线程池
void ThreadPool::addTask(SOCKET socket, const std::string& ip) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push({socket, ip});
    }
    m_cv.notify_one(); // 唤醒一个工作线程
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