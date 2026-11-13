# C-graph：邻接表图结构（C 语言实现）

[![C](https://img.shields.io/badge/C-11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[English](README.md)

## 项目介绍

这个项目用 C11 实现了一个基于邻接表的图结构，支持有向图和无向图，边权是整数。除了建图、增删顶点和边，还带了 DFS、BFS、Dijkstra 最短路径和路径还原。

最早只有一个 `图.cpp`，代码全挤在一起，顶点容量也写死。后来拆成 `graph.h` / `graph.c` / `main.c`，顶点数组改成自动扩容，删顶点时统一调整边下标，遍历改成回调，测试和 Makefile 也补上了。

核心流程：

```mermaid
flowchart LR
    A[graphCreate 建图] --> B[graphAddVertex 加顶点]
    B --> C[graphAddEdge 加边]
    C --> D[graphDFS / graphBFS 遍历]
    C --> E[graphDijkstra 最短路]
    E --> F[graphBuildPath 还原路径]
    D --> G[graphDestroy 销毁]
    F --> G
```

功能大概这些：

- 有向图、无向图，边权为整数。
- 顶点数组满了按两倍扩容，创建时的容量只是初始值。
- 无向边自动补反向边；自环只存一条，出度不会翻倍。
- DFS 用显式栈，BFS 用数组队列，遍历结果通过回调交给调用方。
- Dijkstra 求最短路，另外可以用 `prev` 还原完整路径。

## 依赖说明

[![C Standard](https://img.shields.io/badge/C-Standard%20Library-orange.svg)](https://en.wikipedia.org/wiki/C_standard_library)

- C 标准库：`stdio.h`、`stdlib.h`、`stdbool.h`、`stdint.h`、`limits.h`。
- 编译器：GCC 或 Clang，需要支持 C11。
- 构建工具：GNU Make 可选，直接用 gcc 命令也能编译。

## 项目结构

```
.
├── graph.h            类型定义和公开接口
├── graph.c            具体实现
├── main.c             示例程序
├── tests/
│   └── test_graph.c   断言测试
├── Makefile           构建和测试入口
├── README.md          英文说明
├── README_CN.md       中文说明
└── LICENSE
```

顶点放在连续数组里，每个顶点挂一条出边链表：

```
Graph
├── vertices[]
│   ├── data
│   └── firstEdge ──► EdgeNode(dest, weight, next) ──► ... ──► NULL
├── vertexCount
├── capacity
└── isDirected
```

## 使用说明

### 编译运行

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -o graph_demo main.c graph.c
./graph_demo
```

或者：

```bash
make
./graph_demo
```

Windows 下生成 `graph_demo.exe`，用 `.\graph_demo.exe` 运行。

### 跑测试

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -I. -o test_graph tests/test_graph.c graph.c
./test_graph
```

也可以直接 `make test`。测试覆盖扩容、增删边、自环、删顶点后的下标迁移、遍历顺序和最短路径。

### 接口

| 函数 | 说明 |
| --- | --- |
| `graphCreate(capacity, isDirected)` | 建图，`capacity` 是初始容量 |
| `graphDestroy(g)` | 释放顶点数组和所有边节点 |
| `graphReserve(g, capacity)` | 预留至少 `capacity` 个顶点的空间 |
| `graphAddVertex(g, data)` | 添加顶点，返回下标，失败返回 `-1` |
| `graphRemoveVertex(g, index)` | 删除顶点，后面的下标前移 |
| `graphGetVertexIndex(g, data)` | 按数据找下标，找不到返回 `-1` |
| `graphGetVertexData(g, index)` | 读顶点数据，非法下标返回 `INT_MIN` |
| `graphVertexCount(g)` | 顶点数量 |
| `graphAddEdge(g, from, to, weight)` | 添加边，无向图自动补反向边 |
| `graphRemoveEdge(g, from, to)` | 删除边，无向图成对删除 |
| `graphHasEdge(g, from, to)` | 判断边是否存在 |
| `graphGetEdgeWeight(g, from, to)` | 读边权，没有这条边返回 `INT_MAX` |
| `graphDFS(g, start, visit, userData)` | 深度优先遍历 |
| `graphBFS(g, start, visit, userData)` | 广度优先遍历 |
| `graphDijkstra(g, start, dist, prev)` | 非负权最短路 |
| `graphBuildPath(prev, vertexCount, end, path, capacity)` | 根据 `prev` 还原路径 |
| `graphInDegree(g, index)` | 入度，无向图中等于顶点的度 |
| `graphOutDegree(g, index)` | 出度，无向图中等于顶点的度 |
| `graphPrint(g)` | 打印邻接表 |

遍历回调的类型是 `void (*GraphVisitFn)(int index, int data, void* userData)`，`userData` 会原样传回去。

### 示例

```c
#include "graph.h"
#include <stdio.h>

static void printVertex(int index, int data, void* userData) {
    (void)index;
    (void)userData;
    printf("%d ", data);
}

int main(void) {
    Graph* g = graphCreate(10, true);

    for (int i = 0; i < 3; i++) {
        graphAddVertex(g, i);
    }
    graphAddEdge(g, 0, 1, 10);
    graphAddEdge(g, 0, 2, 5);
    graphAddEdge(g, 1, 2, 2);

    graphDFS(g, 0, printVertex, NULL);
    printf("\n");

    int dist[3];
    int prev[3];
    graphDijkstra(g, 0, dist, prev);
    printf("到顶点 2 的距离: %d\n", dist[2]);

    graphDestroy(g);
    return 0;
}
```

### 输出摘录

```
[Undirected graph]
Graph (undirected, 5 vertices):
Vertex[0](data=0): -> [2](weight=1) -> [1](weight=1)
...
DFS: 0 2 3 4 1
BFS: 0 2 1 3 4

Shortest paths from vertex 0:
To vertex[0]: distance=0, path=0
To vertex[1]: distance=8, path=0 2 1
To vertex[2]: distance=5, path=0 2
To vertex[3]: distance=9, path=0 2 1 3
To vertex[4]: distance=7, path=0 2 4
```

完整输出运行 `./graph_demo` 就能看到。

### 注意事项

- Dijkstra 只支持非负边权，遇到负权边会跳过。
- 删顶点后数组会压紧，之前保存的下标可能失效，需要重新查。
- `graphDijkstra` 的 `dist` 和 `prev` 由调用方分配，长度至少为顶点数。
- DFS/BFS 只走起点所在的连通分量，非连通图要自己遍历剩下的顶点。

## 贡献指南

发现问题可以直接开 issue，也欢迎提 PR。改动尽量保持 C11，行为有变化就补一条测试，提交前跑一下 `make test`。

## 联系作者

**谙弆悕博士（Ailan Anjuxi）**

📫邮箱：[anjuxi.ME@outlook.com](mailto:anjuxi.ME@outlook.com)

📞SIP电话：[sip:anjuxi@sip.linphone.org](sip:anjuxi@sip.linphone.org)

## 许可证

MIT，详见 [LICENSE](LICENSE)。
