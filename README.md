# LiteChat Client
轻量级即时通讯客户端程序，基于 Qt6 Widgets 框架开发，采用 TCP Socket 通信架构，为 [LiteChatServer](https://github.com/resources-deliver/LiteChatServer) 聊天通信软件提供美观、流畅的桌面端用户交互界面。

## 项目简介
LiteChat 是一款轻量级聊天通信软件，采用客户端/服务器（C/S）架构设计。本项目为客户端程序，负责提供用户交互界面、本地输入校验、网络通信以及与服务器端进行数据交互。

- **客户端**：Windows 下基于 Qt6 Widgets 框架开发（本项目）
- **服务器端**：Linux 下基于 C++11 原生 Socket 开发
- **数据库**：MySQL 8.0+

## 功能特性
### 用户管理
- 用户注册（MD5 加密密码传输，本地用户名/密码格式校验）
- 用户登录（支持 Enter 键快捷导航，防重复点击）
- 用户信息修改（用户名/密码，需验证当前密码身份）
- 用户在线状态显示（顶部状态指示灯，绿色在线/灰色离线）

### 好友管理
- 添加/删除好友（支持搜索查询、右键菜单操作）
- 好友列表查询（含在线状态指示器，支持右键查看资料/删除好友）
- 好友状态实时通知（好友上下线自动更新状态指示灯）
- 好友列表刷新

### 消息通信
- 文本消息发送与接收（Enter 发送 / Shift+Enter 换行）
- 消息气泡显示（自己发送蓝色气泡靠右，好友消息白色气泡靠左）
- 历史消息查询（切换聊天对象时自动加载）
- 陌生人消息标记（橙色 [陌生人] 标签）
- 新消息通知（主窗口活跃时显示通知气泡，最小化时显示系统托盘通知）
- 消息时间戳显示

### 系统功能
- TCP 粘包处理（4字节大端序消息头 + 消息体）
- 断线自动重连（最多 3 次，间隔 5 秒，区分远程主动关闭与网络断开）
- 日志记录（按日期轮转，超 10MB 自动轮转，过滤敏感信息如密码）
- 防重复点击保护（1.5秒自动消失提示气泡）
- 系统托盘图标（支持托盘消息通知）
- 异常处理（网络异常、超时、连接断开等场景覆盖）

## 技术栈
| 技术领域 | 技术选型 |
|---------|---------|
| 编程语言 | C++17 |
| 构建工具 | CMake 3.19+ |
| UI 框架 | Qt 6.5+（Widgets） |
| 网络通信 | QTcpSocket（TCP/IPv4） |
| 数据加密 | MD5（QCryptographicHash） |
| JSON 解析 | QJsonDocument / QJsonObject |
| 字符编码 | UTF-8 |

## 项目结构
```
LiteChatClient/
├── CMakeLists.txt                  # CMake 构建配置
├── README.md                       # 项目说明文档
├── .gitignore                      # Git 忽略规则
├── Server.md                       # 服务器端项目说明
├── docs/                           # 设计文档
│   ├── Whole/                      # 总体设计文档
│   │   ├── SRS.md                  # 软件需求规格说明书
│   │   ├── HLD.md                  # 概要设计说明书
│   │   ├── LLD.md                  # 详细设计说明书
│   │   └── Stage.md                # 多阶段开发计划
│   └── DealStage/                  # 处理流程文档
│       ├── ClientDealStage.md      # 客户端处理逻辑
│       └── ServerDealStage.md      # 服务器端处理逻辑
├── header/                         # 头文件
│   ├── networkmanager.h            # 网络管理器（TCP 连接、数据收发）
│   ├── usermanager.h               # 用户管理器（注册/登录/信息修改）
│   ├── friendmanager.h             # 好友管理器（添加/删除/列表/查询）
│   ├── messagemanager.h            # 消息管理器（发送/接收/历史/缓存）
│   ├── clientlogger.h              # 日志记录器（单例模式）
│   ├── exceptionhandler.h          # 异常处理器（断线重连）
│   ├── connectdialog.h             # 连接对话框
│   ├── logindialog.h               # 登录对话框
│   ├── registerdialog.h            # 注册对话框
│   ├── userinfodialog.h            # 个人信息修改对话框
│   ├── mainwindow.h                # 主窗口
│   └── searchresultdialog.h        # 搜索结果对话框
├── src/                            # 源文件
│   ├── main.cpp                    # 程序入口
│   ├── networkmanager.cpp          # 网络管理器实现
│   ├── usermanager.cpp             # 用户管理器实现
│   ├── friendmanager.cpp           # 好友管理器实现
│   ├── messagemanager.cpp          # 消息管理器实现
│   ├── clientlogger.cpp            # 日志记录器实现
│   ├── exceptionhandler.cpp        # 异常处理器实现
│   ├── connectdialog.cpp           # 连接对话框实现
│   ├── logindialog.cpp             # 登录对话框实现
│   ├── registerdialog.cpp          # 注册对话框实现
│   ├── userinfodialog.cpp          # 个人信息修改对话框实现
│   ├── mainwindow.cpp              # 主窗口实现
│   └── searchresultdialog.cpp      # 搜索结果对话框实现
└── ui/                             # Qt Designer UI 文件
    ├── mainwindow.ui               # 主窗口骨架
    ├── connectdialog.ui            # 连接对话框布局
    ├── logindialog.ui              # 登录对话框布局
    ├── registerdialog.ui           # 注册对话框布局
    └── userinfodialog.ui           # 个人信息修改对话框布局
```

## 架构分层
```
┌─────────────────────────────────────┐
│          表示层（UI 层）             │
│  MainWindow、ConnectDialog、         │
│  LoginDialog、RegisterDialog、       │
│  UserInfoDialog、SearchResultDialog │
├─────────────────────────────────────┤
│          业务逻辑层                  │
│  UserManager、FriendManager、        │
│  MessageManager、ExceptionHandler   │
├─────────────────────────────────────┤
│          网络通信层                  │
│  NetworkManager（QTcpSocket）        │
│  4字节消息头封装/解析                │
├─────────────────────────────────────┤
│          基础设施层                  │
│  ClientLogger（日志记录）            │
└─────────────────────────────────────┘
```

## 环境要求
### 运行环境
| 项目 | 要求 |
|------|------|
| 操作系统 | Windows 10 及以上 |
| 运行时 | Qt 6.5+ 运行时库 |
| 网络 | 能够访问服务器端的 8886 端口 |

### 编译依赖
- **Qt 6.5+**（Core、Widgets、Network 模块）
- **CMake 3.19+**
- **支持 C++17 的编译器**（MSVC 2019+ / MinGW 8.1+ / GCC 8+）

推荐使用 Qt 官方在线安装器安装 Qt 6.8.2 + MinGW 64-bit 工具链。

## 构建与运行
### 1. 克隆项目
```bash
git clone https://github.com/resources-deliver/LiteChatClient.git
cd LiteChatClient
```

### 2. 编译项目
使用 Qt Creator 打开 `CMakeLists.txt`，或使用命令行构建：

```bash
# 创建构建目录
mkdir -p build && cd build
# 配置 CMake（请根据实际 Qt 安装路径调整）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/mingw_64"
# 编译
cmake --build . --config Release
```

### 3. 运行客户端
```bash
# 在构建目录下运行
./LiteChatClient.exe
```

程序启动后首先进入**连接服务器**对话框，输入服务器 IP 地址后点击"连接"或直接"跳过"（使用默认 IP），连接成功后进入**登录/注册**流程。

## 通信协议
### 消息格式
```
+----------------+------------------+
| 4字节消息头(大端序) | 消息体(JSON)  |
+----------------+------------------+
```

- 消息头：4字节无符号整数（大端序），表示消息体的字节长度
- 消息体：UTF-8 编码的 JSON 字符串
- 消息体最大长度：65536 字节（64KB）

### 请求类型
| Type | 说明 | 处理模块 |
|------|------|---------|
| REGISTER | 用户注册 | UserManager |
| LOGIN | 用户登录 | UserManager |
| UPDATE_USER | 修改用户信息 | UserManager |
| QUERY_STATUS | 查询用户状态 | UserManager |
| ADD_FRIEND | 添加好友 | FriendManager |
| DEL_FRIEND | 删除好友 | FriendManager |
| FRIEND_LIST | 查询好友列表 | FriendManager |
| QUERY_FRIEND | 查询好友信息 | FriendManager |
| SEND_MSG | 发送消息 | MessageManager |
| HISTORY_MSG | 查询历史消息 | MessageManager |

### 服务端推送类型
| Type | 说明 |
|------|------|
| STATUS_NOTIFY | 好友状态变更通知 |
| FORWARD_MSG | 转发消息 |
| MSG_NOTIFY | 新消息通知 |
| HEARTBEAT | 心跳检测（客户端接收并忽略） |

## 界面概览
### 连接服务器
- 输入服务器 IP 地址，点击"连接"或"跳过"使用默认 IP
- 支持 IP 地址格式的正则校验
- 5 秒超时提示

### 登录
- 用户名/密码输入，支持 Enter 键从用户名跳转到密码框
- 点击"注册"跳转到注册页面
- 注册成功后自动回填用户名到登录页

### 注册
- 用户名（3-20位字母/数字/下划线）、密码（至少6位）、确认密码
- 密码一致性校验
- 注册成功后自动返回登录页并填充用户名

### 主窗口
- **顶部栏**：头像（点击进入个人信息设置）、用户名、在线状态指示灯、搜索框
- **左侧面板**：好友列表（含在线状态指示器），支持右键菜单（查看资料/删除好友）
- **右侧内容区**：欢迎页 / 聊天页（消息气泡 + 输入框）
- **底部操作栏**：删除好友、刷新列表按钮
- **系统托盘**：最小化后托盘图标常驻，支持新消息通知

### 个人信息设置
- 修改用户名和/或密码
- 需输入当前密码验证身份

## 配置说明
主要配置项位于源码中，可根据需要修改：

| 配置项 | 位置 | 默认值 |
|--------|------|--------|
| 默认服务器 IP | `connectdialog.h` | 192.168.162.128 |
| 服务器端口 | `networkmanager.cpp` | 8886 |
| 连接超时 | `networkmanager.cpp` | 5 秒 |
| 请求超时（UI） | 各对话框 `.cpp` | 5 秒（添加好友 15秒） |
| 最大重连次数 | `exceptionhandler.cpp` | 3 次 |
| 重连间隔 | `exceptionhandler.cpp` | 5 秒 |
| 日志轮转大小 | `clientlogger.cpp` | 10MB |
| 日志目录 | `main.cpp` | 程序目录/logs/ |
| 最大消息体 | `messagemanager.cpp` | 65536 字节 |

## 开发阶段
项目按五个阶段迭代开发，当前已完成全部阶段：

1. **第一阶段**：基础通信框架与底层支撑（NetworkManager、ConnectDialog、ClientLogger）
2. **第二阶段**：用户管理模块（UserManager、LoginDialog、RegisterDialog、UserInfoDialog、ExceptionHandler）
3. **第三阶段**：好友管理模块（FriendManager、SearchResultDialog）
4. **第四阶段**：消息通信模块（MessageManager、MainWindow 聊天功能）
5. **第五阶段**：系统功能完善（主窗口界面优化、系统托盘、通知气泡、防重复点击）