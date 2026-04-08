# 聊天室项目

## 项目简介

这是一个基于C++实现的聊天室服务器项目，提供了完整的用户注册、登录、消息发送和接收功能，并通过Web界面进行交互。项目采用了线程池技术处理并发连接，使用MySQL数据库存储用户信息和消息，支持跨平台运行（Windows和Linux）。

## 项目亮点

- **跨平台高并发网络模型**：Windows 用 select + 非阻塞监听 Socket，Linux 用 epoll ET 边缘触发 + EPOLLONESHOT + 非阻塞监听 Socket，循环 accept 一次性取完所有待处理连接，解决高并发下的连接漏处理问题和多线程同时处理同一连接的问题
- **线程池解耦连接与业务**：生产者 - 消费者模式实现线程池，主线程负责接收连接，工作线程负责处理 HTTP 请求，避免频繁创建 / 销毁线程的性能开销
- **安全与鲁棒性设计**：数据库密码无回显输入、参数化查询防 SQL 注入、HTML 转义防 XSS、全链路错误处理与资源清理
- **工程化能力**：CMake 跨平台构建、配置文件动态配置、模块化代码分层（网络 / 数据库 / 业务 / 工具）
- **单例模式优化**：所有管理器类均采用懒汉式单例模式，确保线程安全和资源合理使用
- **完整的中文注释**：所有代码文件均添加了详细的中文注释，提高代码可读性和可维护性

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
- **实时消息更新**：使用长轮询技术实现消息实时更新
- **智能线程数配置**：根据CPU核心数自动调整线程数
- **支持中文**：支持中文用户名和消息内容
- **单例模式**：所有管理器类均采用懒汉式单例模式，确保线程安全
- **详细的中文注释**：所有代码文件均添加了详细的中文注释，提高代码可读性

## 技术栈

- **后端**：C++
- **数据库**：MySQL
- **前端**：HTML5, CSS3, JavaScript
- **网络**：Socket编程, HTTP协议
- **并发处理**：线程池（生产者-消费者模式）
- **实时通信**：长轮询（Long Polling）
- **设计模式**：单例模式（懒汉式）
- **数据库连接池**：管理数据库连接，提高性能
- **跨平台**：支持Windows和Linux平台

## 项目结构

```
chatroom/
├── src/                  # 源代码目录
│   ├── main.cpp          # 主入口：服务启动、网络事件监听、连接分发
│   ├── config.cpp        # 配置管理实现
│   ├── database.cpp      # 数据库实现
│   ├── dbpool.cpp        # 数据库连接池实现
│   ├── message.cpp       # 消息实现
│   ├── network.cpp       # 网络实现
│   ├── threadpool.cpp    # 线程池实现
│   ├── user.cpp          # 用户实现
│   ├── utils.cpp         # 工具函数实现
│   └── web.cpp           # Web实现
├── include/              # 头文件目录
│   ├── config.h          # 配置管理：读取config.ini，提供配置项接口
│   ├── database.h        # 数据库管理：MySQL连接、参数化查询执行
│   ├── dbpool.h          # 数据库连接池：管理数据库连接
│   ├── message.h         # 消息管理：消息存储、查询
│   ├── network.h         # 网络通信：Socket初始化、跨平台兼容、客户端连接处理
│   ├── threadpool.h      # 线程池：生产者-消费者模式，任务分发
│   ├── user.h            # 用户管理：用户注册、登录验证
│   ├── utils.h           # 工具函数：HTML转义、字符串处理
│   └── web.h             # Web处理：HTTP请求解析、响应生成、API接口
├── html/                 # 前端页面：登录/注册、聊天界面
│   ├── login.html        # 登录/注册页面
│   └── chat.html         # 聊天页面
├── build/                # 构建目录
│   └── clean.bat         # 清理脚本
├── config.ini            # 配置文件：服务器端口、数据库连接信息
├── CMakeLists.txt        # CMake配置：跨平台构建
├── init_database.sql     # 数据库初始化：建表、索引
├── make.bat              # Windows编译脚本
├── .gitignore            # Git忽略文件
└── README.md             # 项目说明
```

## 环境要求

- **C++编译器**：支持C++11或更高版本
- **MySQL**：5.7或更高版本
- **MySQL Connector/C**：用于C++连接MySQL数据库
- **Windows**：需要Winsock2库
- **Linux**：需要Socket库

## 安装与运行

### 1. 安装依赖

#### Windows
- 安装MySQL数据库
- 安装MySQL Connector/C
- 配置环境变量，确保编译器能找到MySQL头文件和库文件

#### Linux
- 安装MySQL数据库：`sudo apt-get install mysql-server`
- 安装MySQL开发库：`sudo apt-get install libmysqlclient-dev`

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

1. 安装 MySQL 开发库：`sudo apt-get install libmysqlclient-dev`
2. 确保 CMakeLists.txt 中包含 MySQL 头文件和库文件链接：

```cmake
find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
target_link_libraries(chatroom ${MYSQL_LIBRARIES})
```

3. 使用CMake编译：

```bash
mkdir build
cd build
cmake ..
make
```

### 4. 运行项目

1. 启动服务器：

```bash
chatroom.exe  # Windows
./chatroom    # Linux
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
| created_at | TIMESTAMP | 创建时间 |

### messages表

| 字段名 | 数据类型 | 描述 |
|-------|---------|------|
| id | INT | 消息ID（自增） |
| from_user | VARCHAR(50) | 发送者用户名 |
| to_user | VARCHAR(50) | 接收者用户名 |
| content | TEXT | 消息内容 |
| timestamp | TIMESTAMP | 消息时间戳 |

## 技术实现细节

### 1. 网络模型

- **多线程并发处理**：使用线程池技术，充分利用多核CPU性能
- **生产者-消费者模式**：主线程负责接受新连接，工作线程负责处理请求
- **跨平台Socket**：通过条件编译实现Windows和Linux平台的Socket兼容
- **HTTP服务器**：自定义实现简单的HTTP服务器，支持GET和POST请求
- **非阻塞监听Socket**：Windows/Linux 的 serverSocket 均设为非阻塞，配合循环 accept 一次性取完所有待处理连接，避免高并发下的连接漏处理
- **客户端Socket适配**：Windows 下 clientSocket 改回阻塞模式，兼容现有线程池的 recv/send 逻辑；Linux 下 clientSocket 设为非阻塞，并使用 EPOLLONESHOT 确保每个事件只被一个线程处理
- **循环Accept**：实现循环accept处理所有待处理连接
- **优雅关闭机制**：控制台指令（ exit/quit/stop ）触发服务退出，全链路资源清理（套接字、epoll 实例、线程池、数据库连接）
- **双重检查服务状态**： accept 前二次检查服务状态，避免对已关闭的 Socket 操作
- **超时机制**： select/epoll_wait 设置 1 秒超时，保证服务能及时响应退出指令

### 2. 单例模式实现

所有管理器类均采用懒汉式单例模式，确保线程安全和资源合理使用：

```cpp
// NetworkManager单例实现
NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

// ConfigManager单例实现
ConfigManager& ConfigManager::getInstance(const std::string& file) {
    static ConfigManager instance(file);
    return instance;
}

// DatabaseManager单例实现
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

// ThreadPool单例实现
ThreadPool& ThreadPool::getInstance(int numThreads) {
    static ThreadPool instance(numThreads);
    return instance;
}

// MessageManager单例实现
MessageManager& MessageManager::getInstance() {
    static MessageManager instance;
    return instance;
}

// UserManager单例实现
UserManager& UserManager::getInstance() {
    static UserManager instance;
    return instance;
}
```

### 3. 线程池实现

- **智能线程数配置**：根据CPU核心数自动调整线程数（核心数×2）
- **任务队列**：使用线程安全的队列存储客户端连接
- **条件变量**：实现工作线程的高效唤醒机制
- **线程安全**：使用互斥锁和原子变量确保线程安全

### 4. 消息更新机制

- **长轮询**：前端JavaScript发送请求后等待服务器响应，收到响应后立即发送新请求，实现实时消息更新
- **API接口**：`/api/messages`接口返回JSON格式的消息数据，支持长轮询
- **消息过滤**：只返回当前用户的消息，确保消息的私密性
- **超时机制**：服务器端设置30秒超时，避免连接无限期等待

### 5. 安全措施

- **HTML转义**：对用户输入进行HTML转义，防止XSS攻击
- **输入验证**：验证用户名和密码的合法性
- **SQL注入防护**：使用字符串转义处理参数，防止SQL注入攻击

### 6. 性能优化

- **线程池**：使用线程池处理并发连接，提高性能
- **数据库索引**：在用户名字段上创建索引，提高查询速度
- **消息清理**：定期清理过期消息，减少数据库负担
- **非阻塞Socket**：提高网络处理效率
- **循环发送**：确保所有数据都能发送出去

## 核心代码解析

### 1. 主循环（main.cpp）

核心逻辑：非阻塞监听 Socket + 循环 accept  + 线程池分发，使用 NetworkManager 单例管理网络相关全局变量

```cpp
// 主循环 - 处理新连接和客户端通信
#ifdef _WIN32
// Windows平台使用select
while (NetworkManager::getInstance().getRunning()) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);
    
    // 将所有客户端套接字添加到readSet
    {  
        std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
        for (SOCKET clientSocket : NetworkManager::getInstance().getClientSocketSet()) {
            FD_SET(clientSocket, &readSet);
        }
    }
    
    // 设置超时时间为1秒
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    // 调用select检查可读事件
    int result = select(0, &readSet, nullptr, nullptr, &timeout);
    // 处理结果...
}
#endif
```

### 2. 长轮询实现（chat.html）

核心逻辑：发送请求后等待服务器响应，收到响应后立即发送新请求，实现实时消息更新

```javascript
// 长轮询函数
function longPollMessages() {
    let xhr = new XMLHttpRequest();
    // 发送GET请求到API接口
    xhr.open('GET', '/api/messages?username=' + encodeURIComponent(username) + '&lastCount=' + currentMessageCount, true);
    // 处理响应
    xhr.onload = function() {
        if (xhr.status === 200) {
            const data = JSON.parse(xhr.responseText);
            updateMessages(data);
        }
        // 立即发送下一次请求，实现长轮询
        longPollMessages();
    };
    xhr.send();
}
```

### 3. 线程池实现（threadpool.cpp）

核心逻辑：生产者-消费者模式，任务队列 + 条件变量唤醒

```cpp
// 工作线程循环
void ThreadPool::workerLoop() {
    while (m_running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            // 等待任务或退出信号
            m_cv.wait(lock, [this] { return !m_taskQueue.empty() || !m_running; });
            if (!m_running) return;
            // 取出任务
            task = m_taskQueue.front();
            m_taskQueue.pop();
        }
        handleClientConnection(task.socket);
    }
}

// 添加任务到线程池
void ThreadPool::addTask(SOCKET socket) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_taskQueue.push({socket});
    m_cv.notify_one();
}
```

### 4. 消息处理（message.cpp）

核心逻辑：参数化查询存储消息，按用户查询消息列表

```cpp
// 存储消息
void MessageManager::addMessage(const std::string& from, const std::string& to, const std::string& content) {
    // 使用预处理语句插入新消息
    std::string insertQuery = "INSERT INTO messages (from_user, to_user, content) VALUES (?, ?, ?)";
    std::vector<std::string> insertParams = {from, to, content};
    DatabaseManager::getInstance().executePreparedUpdate(insertQuery, insertParams);
}

// 获取用户消息
std::vector<Message> MessageManager::getMessagesForUser(const std::string& username) {
    std::vector<Message> messages;
    // 查询用户的所有消息
    std::string query = "SELECT from_user, to_user, content, timestamp FROM messages WHERE to_user = ? ORDER BY timestamp ASC";
    std::vector<std::string> queryParams = {username};
    MYSQL_RES* result = DatabaseManager::getInstance().executePreparedQuery(query, queryParams);
    // 处理查询结果...
    return messages;
}
```

### 5. 数据库连接池（dbpool.cpp）

核心逻辑：管理数据库连接，提高数据库操作性能

```cpp
// 获取数据库连接
MYSQL* DBPool::getConnection() {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    // 等待连接可用
    m_cv.wait(lock, [this]() {
        return !m_connections.empty() || !m_running;
    });
    
    if (!m_running) {
        return nullptr;
    }
    
    MYSQL* conn = m_connections.front();
    m_connections.pop();
    return conn;
}

// 归还数据库连接
void DBPool::returnConnection(MYSQL* conn) {
    if (conn == nullptr) {
        return;
    }
    
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_running) {
        m_connections.push(conn);
        m_cv.notify_one();
    } else {
        mysql_close(conn);
    }
}
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

## 技术难点与解决方案

### 高并发下的连接漏处理
- **难点**：同一时间大量客户端连接涌入时，单次 accept 只能取一个连接，导致连接积压
- **解决方案**：将 serverSocket 设为非阻塞，循环 accept 直到返回 WSAEWOULDBLOCK/EAGAIN ，一次性取完内核全连接队列里的所有连接

### 服务退出卡死问题
- **难点**：阻塞模式下， select 返回可读但 accept 前客户端断开， accept 会一直阻塞，无法响应退出指令
- **解决方案**： serverSocket 设为非阻塞， select/epoll_wait 设置 1 秒超时， accept 前二次检查服务状态，保证服务能及时退出

### 跨平台 Socket API 差异
- **难点**：Windows 用 ioctlsocket 设置非阻塞，Linux 用 fcntl ；Windows 错误码用 WSAGetLastError ，Linux 用 errno
- **解决方案**：通过条件编译（ #ifdef _WIN32 ）封装跨平台兼容代码，统一接口

### 全局变量管理
- **难点**：全局变量容易被意外修改，导致服务器崩溃
- **解决方案**：使用单例模式封装全局变量，提供安全的访问方法，确保线程安全

## 生产环境优化方向

本项目为学习项目，生产环境可参考以下优化：

- **密码安全**：使用 bcrypt/Argon2 等密码哈希算法存储密码，而非明文
- **传输安全**：集成 OpenSSL 实现 HTTPS 加密传输，防止数据窃听
- **实时通信**：将短轮询替换为 WebSocket ，降低服务器负载，提升实时性
- **数据库优化**：封装数据库连接池，避免单连接的性能瓶颈
- **日志系统**：接入 spdlog 等日志库，实现分级日志（DEBUG/INFO/ERROR），便于问题排查
- **信号处理**：Linux 下补充 SIGINT/SIGTERM 信号处理，完善优雅关闭

## 注意事项

- 本项目仅用于学习和测试，不建议在生产环境中使用
- 在生产环境中，建议使用HTTPS加密传输
- 建议使用密码哈希算法存储密码，而不是明文存储
- 建议添加更多的错误处理和日志记录

## 许可证

本项目仅供学习使用。