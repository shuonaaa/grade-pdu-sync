## 编译客户端和服务端

### 初步测试

```bash
cd ~/homework/7/debug_v && gcc udp_client.c -o client && ./client
cd ~/homework/7/debug_v && gcc udp_server.c -o server && ./server
```

### 创建数据库

```bash
CREATE DATABASE IF NOT EXISTS week_7_server
CHARACTER SET utf8mb4
COLLATE utf8mb4_general_ci;

CREATE DATABASE IF NOT EXISTS week_7_client
CHARACTER SET utf8mb4
COLLATE utf8mb4_general_ci;
```

### 使用数据库，定义数据

```
use week_7_server;
# ddl语句

use week_7_client;
# ddl语句
```

### 优化PDU结构

### 创建头文件

数据包(无结构) <-> PDU <-> 结构化数据(方便人读)
实现相互转化
编译更改为
```bash
# 不 debug
gcc udp_client.c PDULib/controlPDU.c PDULib/dataPDU.c -o client               
gcc udp_server.c PDULib/controlPDU.c PDULib/dataPDU.c -o server     
# 带 debug      
gcc -DDEBUG udp_client.c PDULib/controlPDU.c PDULib/dataPDU.c -o client       
gcc -DDEBUG udp_server.c PDULib/controlPDU.c PDULib/dataPDU.c -o server
```

### 发现PDU无法区分为data还是control
在PDU最前方加了flag