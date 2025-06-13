

# 前端框架详细设计 
---
## 项目简介

本项目为**网页五子棋在线对战系统**前端，基于 `Vue3` + `Vite` + `Tailwind CSS` 实现，配合 RESTful API 及 WebSocket 实现实时房间、棋局与聊天。
支持注册/登录、游戏大厅、对局房间、观战、实时聊天等功能，适配通用后端服务。

---

## 技术栈

* **核心框架**：Vue3 (Composition API)
* **构建工具**：Vite
* **样式库**：Tailwind CSS 3.x
* **路由管理**：vue-router
* **数据交互**：RESTful API (fetch)，WebSocket（ws）

---

## 主要页面与功能模块

### 1. 登录/注册页面

* 路由：`/login`、`/register`
* 功能：账号注册、登录，输入校验，密码清空提示、登录状态持久化

### 2. 游戏大厅

* 路由：`/lobby`
* 功能：

  * 显示在线用户、房间列表
  * 创建/加入/观战房间
  * 人机对战入口
  * 支持房间密码
  * 实时数据刷新（定时/WS推送）

### 3. 游戏房间

* 路由：`/room/:id`
* 功能：

  * 棋盘绘制与操作（15x15）
  * 随机分配黑白方
  * 准备流程、倒计时、认输
  * 观战模式限制操作
  * 实时棋局同步与聊天
  * 对局结束弹窗与房间状态自动同步

---

## 主要技术要点

### REST API 封装

* 全部请求集中在 `src/utils/api.js`
* 提供 `get(path)`、`post(path, data)` 两个方法，自动拼接服务器地址

### WebSocket 封装

* 单独工具 `src/utils/ws.js`
* 提供 `connectWS(token)`、`sendWSMsg(type, data)`、`onWSMsg(fn)`、`closeWS()`等方法
* 所有房间内的棋盘和聊天事件实时同步

### 状态管理

* 使用 localStorage 存储登录用户/Token信息
* 页面内用 `ref` 管理响应式数据

### 样式与UI

* 使用 Tailwind 设计现代化风格，整体简洁美观
* 所有页面自适应、无图片依赖、支持动画

---

## 运行与部署

1. **依赖安装**

   ```
   npm install
   ```

2. **开发模式运行**

   ```
   npm run dev
   ```

3. **生产打包**

   ```
   npm run build
   ```

   将 `dist/` 下内容部署到任意静态服务器（Nginx/Apache）

4. **环境变量配置**

   * 如需自定义 API 地址，可在 `.env` 配置 `VITE_API_URL`

---

## 数据与交互流程说明

* 登录注册 → 进入大厅 → 获取房间/用户数据（REST） → 加入房间 → 连接 WebSocket（房间内全部交互走ws）→ 离开房间/认输/聊天 → 退出登录（REST+ws）



# 后端接口文档（REST + WebSocket）

以下为推荐/标准的后端接口及ws消息格式，请根据实际代码适配：

---

## 一、RESTful API 设计

### 1. 用户登录/注册/登出

* **POST /api/auth/register**

  * `body`：{ username, password, email }
  * `response`：{ success, message }

* **POST /api/auth/login**

  * `body`：{ username, password }
  * `response`：{ success, user, token, message }

    * token 用于ws/jwt校验

* **POST /api/auth/logout**

  * `body`：{ username }
  * `response`：{ success }

---

### 2. 用户/房间信息

* **GET /api/users/online**

  * `response`：{ users: \[用户名1, 用户名2, ...] }

* **GET /api/rooms**

  * `response`：{ rooms: \[{ id, name, playerCount, hasPassword, status }] }

    * status: "waiting" | "full" | "playing"

---

### 3. 房间管理

* **POST /api/rooms/create**

  * `body`：{ name, password? }
  * `response`：{ success, room: { id, name, ... }, message }

* **POST /api/rooms/join**

  * `body`：{ roomId, password? }
  * `response`：{ success, message }

* **POST /api/rooms/leave**

  * `body`：{ roomId }
  * `response`：{ success }

---

## 二、WebSocket 消息协议

### 1. 通信说明

* 客户端连接：`ws://server:5555/ws?token=xxx`
* 通信全为 JSON 格式
* 连接后，客户端必须发送 `join_room` 消息（roomId+是否观战）

---

### 2. 客户端 → 服务端 消息

| type        | 必选字段             | 说明      |
| ----------- | ---------------- | ------- |
| join\_room  | roomId, spectate | 加入房间/观战 |
| chess\_move | roomId, x, y     | 落子请求    |
| ready       | roomId           | 玩家准备    |
| giveup      | roomId           | 认输      |
| leave\_room | roomId           | 离开房间    |
| chat        | roomId, text     | 聊天消息    |
| timeout     | roomId           | 超时动作    |

---

### 3. 服务端 → 客户端 消息

| type        | 字段（示例）                                                                   | 说明           |
| ----------- | ------------------------------------------------------------------------ | ------------ |
| room\_info  | black, white, currentUser, currentColor, board, started, over, resultMsg | 房间/棋盘初始化/同步  |
| chess\_move | x, y, color, nextColor, nextUser                                         | 某玩家落子        |
| chat        | user, text, time                                                         | 聊天内容         |
| ready       | started                                                                  | 玩家准备，游戏是否已开始 |
| game\_over  | resultMsg                                                                | 游戏结束，胜负原因/说明 |
| user\_join  | user                                                                     | 用户加入房间       |
| user\_leave | user                                                                     | 用户离开房间       |
| giveup      | loser, winner                                                            | 有玩家认输        |
| timeout     | user                                                                     | 有玩家超时        |

---

### 4. 通用约定

* 连接ws时带上token，后端校验身份
* 客户端状态仅做展示，所有棋盘判定与房间/胜负权威状态由服务端广播
* 聊天、棋局、房间人员变化均有对应消息实时同步

---

## 三、典型交互流程举例

1. **用户A登录**（REST），进入大厅，拉取房间/用户列表（REST）
2. **用户A点击加入房间**（REST），跳转房间页面，连接ws，发送`join_room`
3. **房间信息初始化**（ws服务端推送`room_info`），前端渲染棋盘与状态
4. **用户A点“准备”**，发`ready`，所有人都准备服务端推送`room_info`及状态
5. **轮到用户A下棋**，点击棋盘，发`chess_move`，服务端判定后广播`chess_move`
6. **有人发言**，发`chat`，服务端推送所有人`chat`
7. **有人认输/超时/胜负/离开房间**，均发消息，服务端统一广播相关状态
---

# 数据库设计

## 一、表结构设计

---

### 1. 用户表（users）

|    字段名    |     类型     | 主键 |       约束       |         说明         |
| ----------- | ----------- | ---- | --------------- | -------------------- |
| id          | INT         | √    | AUTO\_INCREMENT | 用户唯一ID            |
| username    | VARCHAR(32) |      | UNIQUE          | 用户名               |
| password    | VARCHAR(64) |      | NOT NULL        | 密码                 |
| email       | VARCHAR(64) |      |                 | 邮箱                 |
| token       | VARCHAR(64) |      |                 | 当前登录token（可选） |
| is\_online  | TINYINT(1)  |      | DEFAULT 0       | 在线状态              |
| last\_login | DATETIME    |      |                 | 上次登录时间          |
| created\_at | DATETIME    |      |                 | 注册时间              |
   
---

### 2. 房间表（rooms）

| 字段名         | 类型                               | 主键 | 约束                | 说明           |
| ----------- | -------------------------------- | -- | ----------------- | ------------ |
| id          | INT                              | √  | AUTO\_INCREMENT   | 房间唯一ID       |
| name        | VARCHAR(32)                      |    | NOT NULL          | 房间名称         |
| password    | VARCHAR(64)                      |    |                   | 房间密码Hash（如有） |
| status      | ENUM('waiting','playing','full') |    | DEFAULT 'waiting' | 房间状态         |
| created\_by | INT                              |    |                   | 创建者用户ID      |
| created\_at | DATETIME                         |    |                   | 创建时间         |
| updated\_at | DATETIME                         |    |                   | 最后更新时间       |

---

### 3. 房间成员表（room\_members）

| 字段名        | 类型                                | 主键 | 约束                  | 说明   |
| ---------- | --------------------------------- | -- | ------------------- | ---- |
| id         | INT                               | √  | AUTO\_INCREMENT     | 自增ID |
| room\_id   | INT                               |    | NOT NULL            | 房间ID |
| user\_id   | INT                               |    | NOT NULL            | 用户ID |
| role       | ENUM('black','white','spectator') |    | DEFAULT 'spectator' | 身份   |
| ready      | TINYINT(1)                        |    | DEFAULT 0           | 是否准备 |
| joined\_at | DATETIME                          |    |                     | 加入时间 |

> 说明：同一房间黑白玩家+观战/准备状态全部由此表维护，离开房间即删除此表记录。

---

### 4. 对局/棋谱表（games）

| 字段名         | 类型          | 主键 | 约束              | 说明               |
| ----------- | ----------- | -- | --------------- | ---------------- |
| id          | INT         | √  | AUTO\_INCREMENT | 对局ID             |
| room\_id    | INT         |    | NOT NULL        | 房间ID             |
| black\_user | INT         |    | NOT NULL        | 黑方用户ID           |
| white\_user | INT         |    | NOT NULL        | 白方用户ID           |
| winner      | INT         |    |                 | 胜者用户ID           |
| reason      | VARCHAR(32) |    |                 | 结束原因（五连/认输/超时）   |
| moves       | TEXT        |    |                 | 棋谱（JSON序列化：数组形式） |
| started\_at | DATETIME    |    |                 | 开始时间             |
| ended\_at   | DATETIME    |    |                 | 结束时间             |

---

### 5. 聊天/消息表（room\_messages）

| 字段名      | 类型           | 主键 | 约束              | 说明     |
| -------- | ------------ | -- | --------------- | ------ |
| id       | INT          | √  | AUTO\_INCREMENT | 自增ID   |
| room\_id | INT          |    | NOT NULL        | 房间ID   |
| user\_id | INT          |    | NOT NULL        | 发送用户ID |
| content  | VARCHAR(512) |    | NOT NULL        | 消息内容   |
| sent\_at | DATETIME     |    |                 | 发送时间   |

---

## 二、建表SQL示例

```sql
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(32) UNIQUE,
    password VARCHAR(64) NOT NULL,
    email VARCHAR(64),
    token VARCHAR(64),
    is_online TINYINT(1) DEFAULT 0,
    last_login DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE rooms (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(32) NOT NULL,
    password VARCHAR(64),
    status ENUM('waiting','playing','full') DEFAULT 'waiting',
    created_by INT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

CREATE TABLE room_members (
    id INT AUTO_INCREMENT PRIMARY KEY,
    room_id INT NOT NULL,
    user_id INT NOT NULL,
    role ENUM('black','white','spectator') DEFAULT 'spectator',
    ready TINYINT(1) DEFAULT 0,
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX(room_id),
    INDEX(user_id)
);

CREATE TABLE games (
    id INT AUTO_INCREMENT PRIMARY KEY,
    room_id INT NOT NULL,
    black_user INT NOT NULL,
    white_user INT NOT NULL,
    winner INT,
    reason VARCHAR(32),
    moves TEXT,
    started_at DATETIME,
    ended_at DATETIME
);

CREATE TABLE room_messages (
    id INT AUTO_INCREMENT PRIMARY KEY,
    room_id INT NOT NULL,
    user_id INT NOT NULL,
    content VARCHAR(512) NOT NULL,
    sent_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX(room_id),
    INDEX(user_id)
);
```

---

## 三、设计说明

* 用户在线状态、token仅作为简单标记，实际应配合后端session或websocket维护。
* 棋谱存储（games.moves）：建议用JSON数组，格式如：`[{"x":7,"y":7,"color":"black"},{"x":8,"y":7,"color":"white"}, ...]`。
* 对局历史与房间分离，便于日后查询和复盘。
* 聊天消息/对局/成员都可随房间id快速检索。
* 观战、准备状态全部在room\_members表维护。


