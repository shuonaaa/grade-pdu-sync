# grade-pdu-sync

基于自定义应用层 PDU 的 UDP 端到端成绩管理系统。仿照学校成绩管理系统，客户端从终端或本地数据库（week_7_client）读取数据，通过 UDP 发送给服务端，服务端解包后同步写入远端数据库（week_7_server）。

## 项目结构

```
homework/7/
├── udp_client.c          # 客户端：输入/读库 → 打包 → 发送
├── udp_server.c          # 服务端：接收 → 解包 → 打印/写库
└── PDULib/
    ├── controlPDU.h/c    # ControlPDU：学号、课程号、学期、状态
    └── dataPDU.h/c       # DataPDU：ControlPDU 基础上加教工号、成绩
```

PDU 设计遵循：**数据包（二进制）↔ PDU 结构体 ↔ 结构化数据（可读）** 三层互转。

---

## 编译

```bash
# 不带 debug
gcc udp_client.c PDULib/controlPDU.c PDULib/dataPDU.c -o client
gcc udp_server.c PDULib/controlPDU.c PDULib/dataPDU.c -o server

# 带 debug（打印原始包、字段内容）
gcc -DDEBUG udp_client.c PDULib/controlPDU.c PDULib/dataPDU.c -o client -lmysqlclient
gcc -DDEBUG udp_server.c PDULib/controlPDU.c PDULib/dataPDU.c -o server -lmysqlclient
```

初步测试（debug_v 目录）：

```bash
cd ~/homework/7/debug_v && gcc udp_client.c -o client && ./client
cd ~/homework/7/debug_v && gcc udp_server.c -o server && ./server
```

---

## 数据库初始化

### 创建数据库

```sql
CREATE DATABASE IF NOT EXISTS week_7_server
CHARACTER SET utf8mb4
COLLATE utf8mb4_general_ci;

CREATE DATABASE IF NOT EXISTS week_7_client
CHARACTER SET utf8mb4
COLLATE utf8mb4_general_ci;
```

### 定义数据表

```sql
use week_7_server;
-- ddl语句

use week_7_client;
-- ddl语句
```

### 测试样例

```
2
220101101
10678040
1
1
19600108
2
5
```
---

## 开发记录

### PDU 结构优化

- 创建 PDULib 头文件，将原始二进制包与结构化数据解耦
- 多字节字段（sid、CourseNumber、tid）统一使用大端序（网络字节序）

### PDU 类型区分

- 发现原始 PDU 无法区分 ControlPDU 与 DataPDU
- 在所有 PDU 最前方加入 1 字节 `type` 标志位（0x01=Control，0x02=Data）
