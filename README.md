# 聊天室项目

这是一个基于C++和MySQL的聊天室项目，使用HTTP协议实现前后端通信。

## 功能特点

- 用户注册和登录
- 发送和接收消息
- 实时消息更新（短轮询）
- 跨平台支持（Windows和Linux）
- 数据库存储消息和用户信息
- 消息24小时自动过期清理

## 技术栈

- **后端**：C++
- **数据库**：MySQL
- **前端**：HTML5, CSS3, JavaScript
- **网络**：Socket编程, HTTP协议

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
4. **查看消息**：聊天页面会自动更新接收到的消息

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

## 安全注意事项

- 本项目使用明文存储密码，仅用于学习目的
- 生产环境中应使用密码哈希算法
- 应添加更多的输入验证和安全措施

## 许可证

本项目仅供学习使用。