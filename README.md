# ChatRoom 服务器

一个基于C++和Socket编程实现的简单聊天室服务器，支持多用户实时消息收发，具备用户认证、消息管理和Web界面交互功能。

## 项目特点

- **跨平台支持**：兼容Windows和类Unix系统
- **多线程并发**：每个客户端请求由独立线程处理
- **用户认证**：支持用户名注册和登录，IP绑定防止账户冒用
- **消息管理**：支持消息存储、查询和24小时自动过期清理
- **Web界面**：提供友好的登录和聊天界面
- **实时消息**：使用长轮询机制实现消息实时推送
- **安全防护**：HTML转义防止XSS攻击，输入验证确保数据合法性
- **中文支持**：完整支持中文用户名和消息内容

## 目录结构

```
chatroom/
├── html/              # HTML页面目录
│   ├── login.html     # 登录页面
│   └── chat.html      # 聊天页面
├── include/           # 头文件目录
│   ├── database.h     # 数据库管理
│   ├── message.h      # 消息处理
│   ├── network.h      # 网络通信
│   ├── user.h         # 用户管理
│   ├── utils.h        # 工具函数
│   └── web.h          # HTTP请求处理
├── src/               # 源文件目录
│   ├── database.cpp   # 数据库实现
│   ├── message.cpp    # 消息实现
│   ├── network.cpp    # 网络实现
│   ├── user.cpp       # 用户实现
│   ├── utils.cpp      # 工具函数实现
│   └── web.cpp        # HTTP请求处理实现
├── main.cpp           # 主服务器入口点
├── CMakeLists.txt     # CMake配置
├── run.bat            # Windows构建和运行脚本
├── init_database.sql  # 数据库初始化脚本
└── README.md          # 此文件
```

## 安装和设置

### 1. 数据库设置

1. 创建一个名为 `chatroom` 的MySQL数据库
2. 运行初始化脚本创建必要的表：
   ```bash
   mysql -u root -p chatroom < init_database.sql
   ```

### 2. 构建和运行

#### Windows系统：

1. 双击 `run.bat` 构建并启动服务器
2. 脚本会自动：
   - 清理之前的构建文件
   - 运行CMake生成构建文件
   - 编译项目
   - 启动服务器

#### Linux系统：

1. 在项目目录中打开终端
2. 运行以下命令：
   ```bash
   cmake .
   cmake --build .
   ./chatroom
   ```

### 3. MySQL连接配置

默认情况下，服务器连接MySQL的参数：
- 主机：localhost
- 用户：root
- 数据库：chatroom
- 密码：（启动时提示输入）

## 使用说明

1. **启动服务器**
   - 运行 `run.bat` (Windows) 或 `./chatroom` (Linux)
   - 输入MySQL密码

2. **访问聊天界面**
   - 打开网页浏览器，导航到 `http://localhost:8888`
   - 注册新用户或使用现有凭据登录

3. **发送消息**
   - 登录后，输入接收者用户名和消息内容
   - 点击"发送"按钮发送消息

4. **接收消息**
   - 新消息会通过长轮询自动显示
   - 消息存储24小时后会被自动清理

## API接口

- **GET /**: 登录页面
- **POST /login**: 用户登录
- **POST /register**: 用户注册
- **GET /view**: 聊天界面
- **POST /send**: 发送消息
- **GET /api/messages**: 获取消息（长轮询）

## 服务器命令

服务器运行时，可在控制台输入以下命令：
- `exit`: 停止服务器
- `quit`: 停止服务器
- `stop`: 停止服务器

## 故障排除

### 常见问题

1. **MySQL连接失败**
   - 确保MySQL服务器正在运行
   - 验证MySQL密码是否正确
   - 检查 `chatroom` 数据库是否存在

2. **端口8888已被占用**
   - 在 `main.cpp` (第65行) 中更改端口为可用端口
   - 终止使用端口8888的任何进程

3. **中文字符问题**
   - 服务器现在可以正确处理中文用户名和消息
   - 确保MySQL数据库使用UTF-8编码

## 许可证

MIT许可证

## 贡献

欢迎贡献！请随时提交Pull Request。

## 致谢

- MySQL C API 用于数据库操作
- 标准C++11 用于跨平台兼容性
- HTTP长轮询 用于实时通信

---

# ChatRoom Server

A lightweight chat room server implemented in C++ with HTTP-based communication and MySQL storage.

## Features

- **User Management**
  - User registration with validation
  - User login with password verification
  - Support for Chinese usernames

- **Message System**
  - Send and receive private messages
  - Message persistence in MySQL database
  - Automatic cleanup of expired messages (24 hours old)

- **Real-time Communication**
  - Long polling for near real-time message updates
  - HTTP-based API for message retrieval

- **Cross-platform Support**
  - Windows and Linux compatibility
  - Standard C++11 implementation

- **Security**
  - Input validation and sanitization
  - HTML entity decoding for proper character handling
  - URL encoding for safe parameter passing

## Project Structure

```
chatroom/
├── html/              # HTML frontend files
│   ├── login.html     # Login and registration page
│   └── chat.html      # Chat interface
├── include/           # Header files
│   ├── database.h     # Database management
│   ├── message.h      # Message handling
│   ├── network.h      # Network communication
│   ├── user.h         # User management
│   ├── utils.h        # Utility functions
│   └── web.h          # HTTP request handling
├── src/               # Source files
│   ├── database.cpp   # Database implementation
│   ├── message.cpp    # Message implementation
│   ├── network.cpp    # Network implementation
│   ├── user.cpp       # User implementation
│   ├── utils.cpp      # Utility functions implementation
│   └── web.cpp        # HTTP request handling implementation
├── main.cpp           # Main server entry point
├── CMakeLists.txt     # CMake configuration
├── run.bat            # Windows build and run script
├── init_database.sql  # Database initialization script
└── README.md          # This file
```

## Installation and Setup

### 1. Database Setup

1. Create a MySQL database named `chatroom`
2. Run the initialization script to create necessary tables:
   ```bash
   mysql -u root -p chatroom < init_database.sql
   ```

### 2. Build and Run

#### On Windows:

1. Double-click `run.bat` to build and start the server
2. The script will automatically:
   - Clean previous build files
   - Run CMake to generate build files
   - Compile the project
   - Start the server

#### On Linux:

1. Open a terminal in the project directory
2. Run the following commands:
   ```bash
   cmake .
   cmake --build .
   ./chatroom
   ```

### 3. Configure MySQL Connection

By default, the server connects to MySQL with:
- Host: localhost
- User: root
- Database: chatroom
- Password: (prompted at startup)

## Usage

1. **Start the Server**
   - Run `run.bat` (Windows) or `./chatroom` (Linux)
   - Enter your MySQL password when prompted

2. **Access the Chat Interface**
   - Open a web browser and navigate to `http://localhost:8888`
   - Register a new user or login with existing credentials

3. **Send Messages**
   - After logging in, enter the recipient's username and message content
   - Click "Send" to deliver the message

4. **Receive Messages**
   - New messages will appear automatically via long polling
   - Messages are stored for 24 hours before being automatically cleaned

## API Endpoints

- **GET /**: Login page
- **POST /login**: User login
- **POST /register**: User registration
- **GET /view**: Chat interface
- **POST /send**: Send a message
- **GET /api/messages**: Get messages (long polling)

## Server Commands

While the server is running, you can enter the following commands in the console:
- `exit`: Stop the server
- `quit`: Stop the server
- `stop`: Stop the server

## Troubleshooting

### Common Issues

1. **MySQL Connection Failed**
   - Ensure MySQL server is running
   - Verify your MySQL password is correct
   - Check that the `chatroom` database exists

2. **Port 8888 Already in Use**
   - Change the port in `main.cpp` (line 65) to an available port
   - Kill any process using port 8888

3. **Chinese Character Issues**
   - The server now properly handles Chinese usernames and messages
   - Ensure your MySQL database is using UTF-8 encoding

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- MySQL C API for database operations
- Standard C++11 for cross-platform compatibility
- HTTP long polling for real-time communication
4. **本地访问**：在服务器所在设备的浏览器中访问 `http://localhost:8888`
5. **内网访问**：在同一局域网内的其他设备浏览器中访问 `http://[服务器IP地址]:8888`
   - 服务器IP地址可以在服务器启动时的日志中看到（Local IP address）
   - 例如：`http://192.168.1.100:8888`
6. **公网访问**：通过NAT服务器配置，使聊天室可以对互联网上的所有设备提供服务
   - **配置步骤**：
     1. 登录您的路由器管理界面
     2. 找到"端口转发"或"NAT服务器"设置
     3. 添加一条转发规则：
        - 外部端口：任意未被占用的端口（如8888）
        - 内部IP地址：服务器在局域网内的IP地址
        - 内部端口：8888
     4. 保存配置
   - **访问方式**：在公网设备浏览器中访问 `http://[您的公网IP地址]:[外部端口]`
   - **注意**：需要确保您的网络服务商没有封锁相关端口，且路由器具有公网IP地址

## 使用说明

1. **注册**：在登录页面填写用户名和密码（至少6位），点击"注册并登录"按钮
2. **登录**：在登录页面填写已注册的用户名和密码，点击"登录"按钮
3. **发送消息**：在聊天页面的"发送消息"区域填写接收者用户名和消息内容，点击"发送"按钮
4. **查看消息**：聊天页面的"消息"区域会自动更新显示收到的消息（使用长轮询机制实时推送）

## 技术栈

- **语言**：C++11
- **网络**：Socket API（跨平台兼容）
- **Web**：HTTP协议、HTML5、JavaScript
- **并发**：C++11线程库、互斥锁
- **构建工具**：CMake
- **实时通信**：长轮询机制

## 注意事项

- 服务器默认监听8888端口，请确保该端口未被占用
- 消息会在24小时后自动过期清理
- 用户名支持字母、数字、下划线和UTF-8中文，最多15个字符
- 密码长度至少6个字符
- 每个用户名只能在一个IP地址上登录
- 不允许给自己发送消息
- 发送消息时，接收者必须是已注册的用户

## 辅助函数说明

### 消息管理模块 (MessageManager)

#### `void addMessage(const std::string& from, const std::string& to, const std::string& content)`
- **功能**：添加一条新消息
- **参数**：
  - `from`：发送者用户名
  - `to`：接收者用户名
  - `content`：消息内容
- **返回值**：无
- **使用示例**：
  ```cpp
  g_messageManager.addMessage("Alice", "Bob", "Hello Bob!");
  ```

#### `std::vector<Message> getMessagesForUser(const std::string& username)`
- **功能**：获取指定用户的所有消息
- **参数**：
  - `username`：用户名
- **返回值**：包含该用户所有消息的向量
- **使用示例**：
  ```cpp
  auto messages = g_messageManager.getMessagesForUser("Bob");
  for (const auto& msg : messages) {
      std::cout << "From: " << msg.from << ", Content: " << msg.content << std::endl;
  }
  ```

#### `void cleanExpiredMessages()`
- **功能**：清理所有用户的过期消息（超过24小时）
- **参数**：无
- **返回值**：无
- **使用示例**：
  ```cpp
  g_messageManager.cleanExpiredMessages();
  ```

#### `void cleanExpiredMessagesForUser(const std::string& username)`
- **功能**：清理指定用户的过期消息（超过24小时）
- **参数**：
  - `username`：用户名
- **返回值**：无
- **使用示例**：
  ```cpp
  g_messageManager.cleanExpiredMessagesForUser("Bob");
  ```

### 用户管理模块 (UserManager)

#### `bool registerUser(const std::string& username, const std::string& password, const std::string& ip)`
- **功能**：注册新用户
- **参数**：
  - `username`：用户名
  - `password`：密码
  - `ip`：用户IP地址
- **返回值**：注册成功返回true，失败返回false（用户名已存在）
- **使用示例**：
  ```cpp
  bool success = g_userManager.registerUser("Alice", "password123", "192.168.1.100");
  if (success) {
      std::cout << "注册成功" << std::endl;
  } else {
      std::cout << "用户名已存在" << std::endl;
  }
  ```

#### `bool loginUser(const std::string& username, const std::string& password, const std::string& ip)`
- **功能**：用户登录
- **参数**：
  - `username`：用户名
  - `password`：密码
  - `ip`：用户IP地址
- **返回值**：登录成功返回true，失败返回false（用户名或密码错误）
- **使用示例**：
  ```cpp
  bool success = g_userManager.loginUser("Alice", "password123", "192.168.1.100");
  if (success) {
      std::cout << "登录成功" << std::endl;
  } else {
      std::cout << "用户名或密码错误" << std::endl;
  }
  ```

#### `bool isValidUser(const std::string& username, const std::string& ip)`
- **功能**：验证用户是否有效（用户名存在且IP匹配）
- **参数**：
  - `username`：用户名
  - `ip`：用户IP地址
- **返回值**：验证成功返回true，失败返回false
- **使用示例**：
  ```cpp
  bool valid = g_userManager.isValidUser("Alice", "192.168.1.100");
  if (valid) {
      std::cout << "用户有效" << std::endl;
  } else {
      std::cout << "用户无效" << std::endl;
  }
  ```

#### `bool userExists(const std::string& username)`
- **功能**：检查用户是否存在
- **参数**：
  - `username`：用户名
- **返回值**：用户存在返回true，不存在返回false
- **使用示例**：
  ```cpp
  bool exists = g_userManager.userExists("Alice");
  if (exists) {
      std::cout << "用户存在" << std::endl;
  } else {
      std::cout << "用户不存在" << std::endl;
  }
  ```

### 网络模块

#### `void initializeNetwork()`
- **功能**：初始化网络环境（Windows平台需要初始化WSA）
- **参数**：无
- **返回值**：无
- **使用示例**：
  ```cpp
  initializeNetwork();
  ```

#### `SOCKET createServerSocket(int port)`
- **功能**：创建并配置服务器Socket
- **参数**：
  - `port`：服务器端口
- **返回值**：创建成功返回服务器Socket，失败则退出程序
- **使用示例**：
  ```cpp
  SOCKET serverSocket = createServerSocket(8888);
  ```

#### `void handleClientConnection(SOCKET clientSocket, const std::string& clientIP)`
- **功能**：处理客户端连接
- **参数**：
  - `clientSocket`：客户端Socket
  - `clientIP`：客户端IP地址
- **返回值**：无
- **使用示例**：
  ```cpp
  std::thread(handleClientConnection, clientSocket, clientIP).detach();
  ```

### Web模块

#### `std::string handleHttpRequest(const std::string& request, const std::string& clientIP)`
- **功能**：处理HTTP请求
- **参数**：
  - `request`：HTTP请求内容
  - `clientIP`：客户端IP地址
- **返回值**：HTTP响应内容
- **使用示例**：
  ```cpp
  std::string response = handleHttpRequest(request, clientIP);
  send(clientSocket, response.c_str(), response.length(), 0);
  ```

### 工具函数模块

#### `std::string htmlEscape(const std::string& str)`
- **功能**：HTML转义，防止XSS攻击
- **参数**：
  - `str`：需要转义的字符串
- **返回值**：转义后的字符串
- **使用示例**：
  ```cpp
  std::string safeContent = htmlEscape("<script>alert('XSS')</script>");
  // 输出: &lt;script&gt;alert('XSS')&lt;/script&gt;
  ```

#### `bool isValidUsername(const std::string& username)`
- **功能**：验证用户名是否合法（允许：字母、数字、下划线、UTF-8中文）
- **参数**：
  - `username`：用户名
- **返回值**：用户名合法返回true，不合法返回false
- **使用示例**：
  ```cpp
  bool valid = isValidUsername("Alice_123");
  if (valid) {
      std::cout << "用户名合法" << std::endl;
  } else {
      std::cout << "用户名不合法" << std::endl;
  }
  ```

#### `std::string urlDecode(const std::string& str)`
- **功能**：URL解码
- **参数**：
  - `str`：需要解码的URL字符串
- **返回值**：解码后的字符串
- **使用示例**：
  ```cpp
  std::string decoded = urlDecode("Hello%20World");
  // 输出: Hello World
  ```

#### `std::string urlEncode(const std::string& str)`
- **功能**：URL编码
- **参数**：
  - `str`：需要编码的字符串
- **返回值**：编码后的URL字符串
- **使用示例**：
  ```cpp
  std::string encoded = urlEncode("Hello World");
  // 输出: Hello%20World
  ```

#### `std::string htmlEntityDecode(const std::string& str)`
- **功能**：HTML实体解码
- **参数**：
  - `str`：包含HTML实体的字符串
- **返回值**：解码后的字符串
- **使用示例**：
  ```cpp
  std::string decoded = htmlEntityDecode("&lt;script&gt;");
  // 输出: <script>
  ```

#### `std::string parseFormData(const std::string& data, const std::string& key)`
- **功能**：解析表单数据
- **参数**：
  - `data`：表单数据
  - `key`：要获取的字段名
- **返回值**：字段值
- **使用示例**：
  ```cpp
  std::string username = parseFormData("username=Alice&password=123", "username");
  // 输出: Alice
  ```

#### `std::string getContentType(const std::string& path)`
- **功能**：根据文件路径获取内容类型
- **参数**：
  - `path`：文件路径
- **返回值**：内容类型
- **使用示例**：
  ```cpp
  std::string contentType = getContentType("index.html");
  // 输出: text/html
  ```

#### `std::string parseUrlParam(const std::string& url, const std::string& key)`
- **功能**：解析URL参数
- **参数**：
  - `url`：URL字符串
  - `key`：要获取的参数名
- **返回值**：参数值
- **使用示例**：
  ```cpp
  std::string username = parseUrlParam("/view?username=Alice&status=success", "username");
  // 输出: Alice
  ```

#### `std::string createHttpResponse(int statusCode, const std::string& statusMessage, const std::string& contentType, const std::string& content, const std::string& location = "")`
- **功能**：生成HTTP响应
- **参数**：
  - `statusCode`：HTTP状态码
  - `statusMessage`：HTTP状态消息
  - `contentType`：内容类型
  - `content`：响应内容
  - `location`：重定向地址（可选）
- **返回值**：完整的HTTP响应
- **使用示例**：
  ```cpp
  std::string response = createHttpResponse(200, "OK", "text/html", "<html><body>Hello</body></html>");
  ```

## API接口说明

### 1. 登录接口
- **URL**：`/login`
- **方法**：POST
- **参数**：
  - `username`：用户名
  - `password`：密码
- **成功响应**：重定向到聊天页面
- **失败响应**：401 Unauthorized

### 2. 注册接口
- **URL**：`/register`
- **方法**：POST
- **参数**：
  - `username`：用户名
  - `password`：密码
- **成功响应**：重定向到聊天页面
- **失败响应**：409 Conflict（用户名已存在）

### 3. 发送消息接口
- **URL**：`/send`
- **方法**：POST
- **参数**：
  - `from`：发送者用户名
  - `to`：接收者用户名
  - `content`：消息内容
- **成功响应**：重定向到聊天页面，显示成功提示
- **失败响应**：200 OK，显示错误信息

### 4. 消息查询接口（长轮询）
- **URL**：`/api/messages`
- **方法**：GET
- **参数**：
  - `username`：用户名
  - `lastCount`：当前消息数量（可选）
- **响应**：JSON格式的消息列表
- **示例响应**：
  ```json
  {
    "messages": [
      {"from": "Alice", "content": "Hello Bob!"},
      {"from": "Charlie", "content": "Hi Bob!"}
    ]
  }
  ```

## 长轮询机制说明

本项目使用长轮询机制来实现实时消息推送，具体工作流程如下：

1. **客户端发起请求**：客户端发送GET请求到`/api/messages`接口，带上用户名和当前消息数量
2. **服务器处理请求**：服务器接收请求，解析参数，检查用户是否有新消息
3. **等待新消息**：如果没有新消息，服务器会等待一段时间（最多30秒）
4. **返回响应**：
   - 如果有新消息，立即返回包含所有消息的JSON数据
   - 如果超时后仍然没有新消息，返回空消息列表的JSON数据
5. **客户端处理响应**：客户端收到响应后，更新消息显示，立即发起下一次长轮询请求

这种机制相比传统的短轮询，减少了网络请求的次数，提高了消息的实时性，同时降低了服务器的负载。