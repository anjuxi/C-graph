#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// ========== 邻接表实现的有向图/无向图 ==========

// 边节点结构体（邻接表中的边）
typedef struct EdgeNode {
    int dest;               // 目标顶点索引
    int weight;             // 边的权重（非带权图可忽略）
    struct EdgeNode* next;  // 下一条边
} EdgeNode;

// 顶点结构体
typedef struct Vertex {
    int data;               // 顶点数据
    EdgeNode* firstEdge;    // 第一条边
} Vertex;

// 图结构体
typedef struct {
    Vertex* vertices;       // 顶点数组
    int vertexCount;        // 顶点数量
    int capacity;           // 容量
    bool isDirected;        // 是否为有向图
} Graph;

// 队列结构体（用于BFS）
typedef struct {
    int* data;
    int front;
    int rear;
    int capacity;
} Queue;

// ========== 队列操作 ==========

Queue* queueCreate(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) return NULL;
    q->data = (int*)malloc(sizeof(int) * capacity);
    if (!q->data) {
        free(q);
        return NULL;
    }
    q->front = 0;
    q->rear = 0;
    q->capacity = capacity;
    return q;
}

void queueDestroy(Queue* q) {
    if (!q) return;
    free(q->data);
    free(q);
}

bool queueIsEmpty(Queue* q) {
    return q->front == q->rear;
}

bool queueEnqueue(Queue* q, int val) {
    if ((q->rear + 1) % q->capacity == q->front) return false;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    return true;
}

bool queueDequeue(Queue* q, int* val) {
    if (queueIsEmpty(q)) return false;
    *val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    return true;
}

// ========== 图的创建与销毁 ==========

Graph* graphCreate(int capacity, bool isDirected) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (!g) return NULL;
    
    g->vertices = (Vertex*)malloc(sizeof(Vertex) * capacity);
    if (!g->vertices) {
        free(g);
        return NULL;
    }
    
    for (int i = 0; i < capacity; i++) {
        g->vertices[i].data = 0;
        g->vertices[i].firstEdge = NULL;
    }
    
    g->vertexCount = 0;
    g->capacity = capacity;
    g->isDirected = isDirected;
    return g;
}

void graphDestroy(Graph* g) {
    if (!g) return;
    
    for (int i = 0; i < g->vertexCount; i++) {
        EdgeNode* edge = g->vertices[i].firstEdge;
        while (edge) {
            EdgeNode* temp = edge;
            edge = edge->next;
            free(temp);
        }
    }
    
    free(g->vertices);
    free(g);
}

// ========== 顶点操作 ==========

int graphAddVertex(Graph* g, int data) {
    if (!g || g->vertexCount >= g->capacity) return -1;
    
    int index = g->vertexCount;
    g->vertices[index].data = data;
    g->vertices[index].firstEdge = NULL;
    g->vertexCount++;
    return index;
}

bool graphRemoveVertex(Graph* g, int index) {
    if (!g || index < 0 || index >= g->vertexCount) return false;
    
    // 删除该顶点的所有出边
    EdgeNode* edge = g->vertices[index].firstEdge;
    while (edge) {
        EdgeNode* temp = edge;
        edge = edge->next;
        free(temp);
    }
    
    // 删除其他顶点指向该顶点的边
    for (int i = 0; i < g->vertexCount; i++) {
        if (i == index) continue;
        
        EdgeNode* prev = NULL;
        EdgeNode* curr = g->vertices[i].firstEdge;
        
        while (curr) {
            if (curr->dest == index) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    g->vertices[i].firstEdge = curr->next;
                }
                EdgeNode* temp = curr;
                curr = curr->next;
                free(temp);
            } else {
                // 调整目标索引（因为删除了一个顶点）
                if (curr->dest > index) {
                    curr->dest--;
                }
                prev = curr;
                curr = curr->next;
            }
        }
    }
    
    // 移动顶点数组
    for (int i = index; i < g->vertexCount - 1; i++) {
        g->vertices[i] = g->vertices[i + 1];
    }
    
    g->vertexCount--;
    return true;
}

int graphGetVertexIndex(Graph* g, int data) {
    if (!g) return -1;
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->vertices[i].data == data) return i;
    }
    return -1;
}

int graphGetVertexData(Graph* g, int index) {
    if (!g || index < 0 || index >= g->vertexCount) return INT_MIN;
    return g->vertices[index].data;
}

int graphVertexCount(Graph* g) {
    return g ? g->vertexCount : 0;
}

// ========== 边操作 ==========

bool graphAddEdge(Graph* g, int from, int to, int weight) {
    if (!g || from < 0 || from >= g->vertexCount || to < 0 || to >= g->vertexCount) {
        return false;
    }
    
    // 检查边是否已存在
    EdgeNode* edge = g->vertices[from].firstEdge;
    while (edge) {
        if (edge->dest == to) return false;
        edge = edge->next;
    }
    
    // 创建新边
    EdgeNode* newEdge = (EdgeNode*)malloc(sizeof(EdgeNode));
    if (!newEdge) return false;
    
    newEdge->dest = to;
    newEdge->weight = weight;
    newEdge->next = g->vertices[from].firstEdge;
    g->vertices[from].firstEdge = newEdge;
    
    // 无向图需要添加反向边
    if (!g->isDirected) {
        EdgeNode* reverseEdge = (EdgeNode*)malloc(sizeof(EdgeNode));
        if (!reverseEdge) return false;
        
        reverseEdge->dest = from;
        reverseEdge->weight = weight;
        reverseEdge->next = g->vertices[to].firstEdge;
        g->vertices[to].firstEdge = reverseEdge;
    }
    
    return true;
}

bool graphRemoveEdge(Graph* g, int from, int to) {
    if (!g || from < 0 || from >= g->vertexCount || to < 0 || to >= g->vertexCount) {
        return false;
    }
    
    // 删除从from到to的边
    EdgeNode* prev = NULL;
    EdgeNode* curr = g->vertices[from].firstEdge;
    bool found = false;
    
    while (curr) {
        if (curr->dest == to) {
            if (prev) {
                prev->next = curr->next;
            } else {
                g->vertices[from].firstEdge = curr->next;
            }
            free(curr);
            found = true;
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    
    if (!found) return false;
    
    // 无向图需要删除反向边
    if (!g->isDirected) {
        prev = NULL;
        curr = g->vertices[to].firstEdge;
        
        while (curr) {
            if (curr->dest == from) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    g->vertices[to].firstEdge = curr->next;
                }
                free(curr);
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    
    return true;
}

bool graphHasEdge(Graph* g, int from, int to) {
    if (!g || from < 0 || from >= g->vertexCount || to < 0 || to >= g->vertexCount) {
        return false;
    }
    
    EdgeNode* edge = g->vertices[from].firstEdge;
    while (edge) {
        if (edge->dest == to) return true;
        edge = edge->next;
    }
    return false;
}

int graphGetEdgeWeight(Graph* g, int from, int to) {
    if (!g || from < 0 || from >= g->vertexCount || to < 0 || to >= g->vertexCount) {
        return INT_MAX;
    }
    
    EdgeNode* edge = g->vertices[from].firstEdge;
    while (edge) {
        if (edge->dest == to) return edge->weight;
        edge = edge->next;
    }
    return INT_MAX;
}

// ========== 图的遍历 ==========

// 深度优先搜索（递归辅助函数）
void graphDFSHelper(Graph* g, int index, bool* visited) {
    visited[index] = true;
    printf("%d ", g->vertices[index].data);
    
    EdgeNode* edge = g->vertices[index].firstEdge;
    while (edge) {
        if (!visited[edge->dest]) {
            graphDFSHelper(g, edge->dest, visited);
        }
        edge = edge->next;
    }
}

void graphDFS(Graph* g, int startIndex) {
    if (!g || startIndex < 0 || startIndex >= g->vertexCount) return;
    
    bool* visited = (bool*)calloc(g->vertexCount, sizeof(bool));
    if (!visited) return;
    
    printf("DFS遍历: ");
    graphDFSHelper(g, startIndex, visited);
    printf("\n");
    
    free(visited);
}

// 广度优先搜索
void graphBFS(Graph* g, int startIndex) {
    if (!g || startIndex < 0 || startIndex >= g->vertexCount) return;
    
    bool* visited = (bool*)calloc(g->vertexCount, sizeof(bool));
    if (!visited) return;
    
    Queue* q = queueCreate(g->vertexCount + 1);
    if (!q) {
        free(visited);
        return;
    }
    
    printf("BFS遍历: ");
    
    visited[startIndex] = true;
    queueEnqueue(q, startIndex);
    
    while (!queueIsEmpty(q)) {
        int index;
        queueDequeue(q, &index);
        printf("%d ", g->vertices[index].data);
        
        EdgeNode* edge = g->vertices[index].firstEdge;
        while (edge) {
            if (!visited[edge->dest]) {
                visited[edge->dest] = true;
                queueEnqueue(q, edge->dest);
            }
            edge = edge->next;
        }
    }
    
    printf("\n");
    free(visited);
    queueDestroy(q);
}

// ========== 图的其他操作 ==========

// 计算顶点的入度
int graphInDegree(Graph* g, int index) {
    if (!g || index < 0 || index >= g->vertexCount) return -1;
    
    int inDegree = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        EdgeNode* edge = g->vertices[i].firstEdge;
        while (edge) {
            if (edge->dest == index) inDegree++;
            edge = edge->next;
        }
    }
    return inDegree;
}

// 计算顶点的出度
int graphOutDegree(Graph* g, int index) {
    if (!g || index < 0 || index >= g->vertexCount) return -1;
    
    int outDegree = 0;
    EdgeNode* edge = g->vertices[index].firstEdge;
    while (edge) {
        outDegree++;
        edge = edge->next;
    }
    return outDegree;
}

// Dijkstra最短路径算法
void graphDijkstra(Graph* g, int startIndex, int* dist, int* prev) {
    if (!g || startIndex < 0 || startIndex >= g->vertexCount) return;
    
    bool* visited = (bool*)calloc(g->vertexCount, sizeof(bool));
    if (!visited) return;
    
    // 初始化
    for (int i = 0; i < g->vertexCount; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[startIndex] = 0;
    
    for (int i = 0; i < g->vertexCount; i++) {
        // 找到未访问的最小距离顶点
        int minDist = INT_MAX;
        int u = -1;
        for (int j = 0; j < g->vertexCount; j++) {
            if (!visited[j] && dist[j] < minDist) {
                minDist = dist[j];
                u = j;
            }
        }
        
        if (u == -1) break;
        visited[u] = true;
        
        // 更新邻接顶点的距离
        EdgeNode* edge = g->vertices[u].firstEdge;
        while (edge) {
            int v = edge->dest;
            if (!visited[v] && dist[u] != INT_MAX && 
                dist[u] + edge->weight < dist[v]) {
                dist[v] = dist[u] + edge->weight;
                prev[v] = u;
            }
            edge = edge->next;
        }
    }
    
    free(visited);
}

// 打印图的结构
void graphPrint(Graph* g) {
    if (!g) {
        printf("NULL\n");
        return;
    }
    
    printf("\n图结构 (%s, %d个顶点):\n", 
           g->isDirected ? "有向图" : "无向图", 
           g->vertexCount);
    
    for (int i = 0; i < g->vertexCount; i++) {
        printf("顶点[%d](数据=%d): ", i, g->vertices[i].data);
        
        EdgeNode* edge = g->vertices[i].firstEdge;
        if (!edge) {
            printf("无出边");
        }
        while (edge) {
            printf("-> [%d](权重=%d) ", edge->dest, edge->weight);
            edge = edge->next;
        }
        printf("\n");
    }
    printf("\n");
}

// ========== 测试代码 ==========

int main() {
    printf("=== 图结构测试 ===\n\n");
    
    // 创建无向图
    printf("【无向图测试】\n");
    Graph* undirected = graphCreate(10, false);
    
    // 添加顶点
    graphAddVertex(undirected, 0);
    graphAddVertex(undirected, 1);
    graphAddVertex(undirected, 2);
    graphAddVertex(undirected, 3);
    graphAddVertex(undirected, 4);
    
    // 添加边
    graphAddEdge(undirected, 0, 1, 1);
    graphAddEdge(undirected, 0, 2, 1);
    graphAddEdge(undirected, 1, 3, 1);
    graphAddEdge(undirected, 2, 3, 1);
    graphAddEdge(undirected, 3, 4, 1);
    
    graphPrint(undirected);
    
    // 遍历测试
    graphDFS(undirected, 0);
    graphBFS(undirected, 0);
    
    printf("顶点0的度: %d\n", graphOutDegree(undirected, 0));
    
    graphDestroy(undirected);
    printf("无向图已销毁\n\n");
    
    // 创建有向带权图
    printf("【有向带权图测试】\n");
    Graph* directed = graphCreate(10, true);
    
    // 添加顶点
    graphAddVertex(directed, 0);
    graphAddVertex(directed, 1);
    graphAddVertex(directed, 2);
    graphAddVertex(directed, 3);
    graphAddVertex(directed, 4);
    
    // 添加带权边
    graphAddEdge(directed, 0, 1, 10);
    graphAddEdge(directed, 0, 2, 5);
    graphAddEdge(directed, 1, 2, 2);
    graphAddEdge(directed, 1, 3, 1);
    graphAddEdge(directed, 2, 1, 3);
    graphAddEdge(directed, 2, 3, 9);
    graphAddEdge(directed, 2, 4, 2);
    graphAddEdge(directed, 3, 4, 4);
    graphAddEdge(directed, 4, 3, 6);
    
    graphPrint(directed);
    
    // 遍历测试
    graphDFS(directed, 0);
    graphBFS(directed, 0);
    
    // 入度出度测试
    printf("顶点1的入度: %d, 出度: %d\n", 
           graphInDegree(directed, 1), 
           graphOutDegree(directed, 1));
    
    // Dijkstra最短路径测试
    printf("\n从顶点0到各顶点的最短路径:\n");
    int* dist = (int*)malloc(sizeof(int) * directed->vertexCount);
    int* prev = (int*)malloc(sizeof(int) * directed->vertexCount);
    
    graphDijkstra(directed, 0, dist, prev);
    
    for (int i = 0; i < directed->vertexCount; i++) {
        printf("到顶点[%d]: ", i);
        if (dist[i] == INT_MAX) {
            printf("不可达\n");
        } else {
            printf("距离=%d, 路径=", dist[i]);
            // 反向追踪路径
            int path[10];
            int pathLen = 0;
            int curr = i;
            while (curr != -1) {
                path[pathLen++] = curr;
                curr = prev[curr];
            }
            for (int j = pathLen - 1; j >= 0; j--) {
                printf("%d ", path[j]);
            }
            printf("\n");
        }
    }
    
    free(dist);
    free(prev);
    
    // 删除边测试
    printf("\n删除边 0->1 后:\n");
    graphRemoveEdge(directed, 0, 1);
    graphPrint(directed);
    
    graphDestroy(directed);
    printf("有向图已销毁\n");
    
    return 0;
}
