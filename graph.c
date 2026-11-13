#include "graph.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define GRAPH_MIN_CAPACITY 8

// BFS 用的队列，每个顶点最多入队一次，capacity 开成顶点数就够
typedef struct {
    int* items;
    int head;
    int tail;
    int capacity;
} Queue;

static Queue* queueCreate(int capacity);
static void queueDestroy(Queue* q);
static bool queueIsEmpty(const Queue* q);
static bool queuePush(Queue* q, int value);
static bool queuePop(Queue* q, int* value);
static bool isVertexIndex(const Graph* g, int index);
static bool removeDirectedEdge(Graph* g, int from, int to);

int graphVertexCount(const Graph* g) {
    return g ? g->vertexCount : 0;
}

Graph* graphCreate(int capacity, bool isDirected) {
    // 容量为负或者乘出来会溢出，直接拒绝
    if (capacity <= 0 || (size_t)capacity > SIZE_MAX / sizeof(Vertex)) {
        return NULL;
    }

    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (!g) {
        return NULL;
    }

    g->vertices = (Vertex*)malloc(sizeof(Vertex) * (size_t)capacity);
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
    if (!g) {
        return;
    }

    for (int i = 0; i < g->vertexCount; i++) {
        EdgeNode* edge = g->vertices[i].firstEdge;
        while (edge) {
            EdgeNode* next = edge->next;
            free(edge);
            edge = next;
        }
    }

    free(g->vertices);
    free(g);
}

bool graphReserve(Graph* g, int capacity) {
    if (!g || capacity < 0) {
        return false;
    }
    if (capacity <= g->capacity) {
        return true;
    }
    if ((size_t)capacity > SIZE_MAX / sizeof(Vertex)) {
        return false;
    }

    Vertex* vertices = (Vertex*)realloc(g->vertices, sizeof(Vertex) * (size_t)capacity);
    if (!vertices) {
        return false;
    }

    // 新槽位要清空，不然后面搬顶点时会碰到野指针
    for (int i = g->capacity; i < capacity; i++) {
        vertices[i].data = 0;
        vertices[i].firstEdge = NULL;
    }

    g->vertices = vertices;
    g->capacity = capacity;
    return true;
}

int graphAddVertex(Graph* g, int data) {
    if (!g) {
        return -1;
    }

    // 满了就翻倍，快贴到 int 上限时不再扩
    if (g->vertexCount == g->capacity) {
        if (g->capacity > INT_MAX / 2) {
            return -1;
        }
        int newCapacity = g->capacity > 0 ? g->capacity * 2 : GRAPH_MIN_CAPACITY;
        if (!graphReserve(g, newCapacity)) {
            return -1;
        }
    }

    int index = g->vertexCount;
    g->vertices[index].data = data;
    g->vertices[index].firstEdge = NULL;
    g->vertexCount++;
    return index;
}

bool graphRemoveVertex(Graph* g, int index) {
    if (!isVertexIndex(g, index)) {
        return false;
    }

    // 先把自己的出边放掉，自环也在这里处理
    EdgeNode* edge = g->vertices[index].firstEdge;
    while (edge) {
        EdgeNode* next = edge->next;
        free(edge);
        edge = next;
    }
    g->vertices[index].firstEdge = NULL;

    // 再断掉别人指向它的边，顺便把大于 index 的目标下标减一
    for (int i = 0; i < g->vertexCount; i++) {
        if (i == index) {
            continue;
        }

        EdgeNode* prev = NULL;
        EdgeNode* curr = g->vertices[i].firstEdge;
        while (curr) {
            if (curr->dest == index) {
                EdgeNode* next = curr->next;
                if (prev) {
                    prev->next = next;
                } else {
                    g->vertices[i].firstEdge = next;
                }
                free(curr);
                curr = next;
                continue;
            }

            if (curr->dest > index) {
                curr->dest--;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    // 顶点数组前移，最后一个位置清干净
    for (int i = index; i < g->vertexCount - 1; i++) {
        g->vertices[i] = g->vertices[i + 1];
    }
    g->vertexCount--;
    g->vertices[g->vertexCount].data = 0;
    g->vertices[g->vertexCount].firstEdge = NULL;
    return true;
}

int graphGetVertexIndex(const Graph* g, int data) {
    if (!g) {
        return -1;
    }

    for (int i = 0; i < g->vertexCount; i++) {
        if (g->vertices[i].data == data) {
            return i;
        }
    }
    return -1;
}

int graphGetVertexData(const Graph* g, int index) {
    if (!isVertexIndex(g, index)) {
        return INT_MIN;
    }
    return g->vertices[index].data;
}

bool graphAddEdge(Graph* g, int from, int to, int weight) {
    if (!isVertexIndex(g, from) || !isVertexIndex(g, to)) {
        return false;
    }
    if (graphHasEdge(g, from, to)) {
        return false;
    }

    EdgeNode* edge = (EdgeNode*)malloc(sizeof(EdgeNode));
    if (!edge) {
        return false;
    }
    edge->dest = to;
    edge->weight = weight;
    edge->next = g->vertices[from].firstEdge;
    g->vertices[from].firstEdge = edge;

    // 无向图补反向边；自环只存一条，不然出度会算成 2
    if (!g->isDirected && from != to) {
        EdgeNode* reverse = (EdgeNode*)malloc(sizeof(EdgeNode));
        if (!reverse) {
            // 反向边没分配出来就把刚加的那条撤掉
            g->vertices[from].firstEdge = edge->next;
            free(edge);
            return false;
        }
        reverse->dest = from;
        reverse->weight = weight;
        reverse->next = g->vertices[to].firstEdge;
        g->vertices[to].firstEdge = reverse;
    }

    return true;
}

bool graphRemoveEdge(Graph* g, int from, int to) {
    if (!isVertexIndex(g, from) || !isVertexIndex(g, to)) {
        return false;
    }

    if (!removeDirectedEdge(g, from, to)) {
        return false;
    }

    // 自环只有一条，不用删两次
    if (!g->isDirected && from != to) {
        removeDirectedEdge(g, to, from);
    }
    return true;
}

bool graphHasEdge(const Graph* g, int from, int to) {
    if (!isVertexIndex(g, from) || !isVertexIndex(g, to)) {
        return false;
    }

    for (const EdgeNode* edge = g->vertices[from].firstEdge; edge; edge = edge->next) {
        if (edge->dest == to) {
            return true;
        }
    }
    return false;
}

int graphGetEdgeWeight(const Graph* g, int from, int to) {
    if (!isVertexIndex(g, from) || !isVertexIndex(g, to)) {
        return INT_MAX;
    }

    for (const EdgeNode* edge = g->vertices[from].firstEdge; edge; edge = edge->next) {
        if (edge->dest == to) {
            return edge->weight;
        }
    }
    return INT_MAX;
}

bool graphDFS(const Graph* g, int startIndex, GraphVisitFn visit, void* userData) {
    if (!isVertexIndex(g, startIndex) || !visit) {
        return false;
    }

    int count = g->vertexCount;
    bool* visited = (bool*)calloc((size_t)count, sizeof(bool));
    int* stack = (int*)malloc(sizeof(int) * (size_t)count);
    int* pending = (int*)malloc(sizeof(int) * (size_t)count);
    if (!visited || !stack || !pending) {
        free(visited);
        free(stack);
        free(pending);
        return false;
    }

    // 显式栈，避免深图递归爆栈
    int top = 0;
    stack[top++] = startIndex;
    visited[startIndex] = true;

    while (top > 0) {
        int u = stack[--top];
        visit(u, g->vertices[u].data, userData);

        // 先按链表顺序记下来，再倒着压栈，访问顺序才和递归版一致
        int pendingCount = 0;
        for (const EdgeNode* edge = g->vertices[u].firstEdge; edge; edge = edge->next) {
            if (!visited[edge->dest]) {
                visited[edge->dest] = true;
                pending[pendingCount++] = edge->dest;
            }
        }
        for (int i = pendingCount - 1; i >= 0; i--) {
            stack[top++] = pending[i];
        }
    }

    free(visited);
    free(stack);
    free(pending);
    return true;
}

bool graphBFS(const Graph* g, int startIndex, GraphVisitFn visit, void* userData) {
    if (!isVertexIndex(g, startIndex) || !visit) {
        return false;
    }

    int count = g->vertexCount;
    bool* visited = (bool*)calloc((size_t)count, sizeof(bool));
    Queue* queue = queueCreate(count);
    if (!visited || !queue) {
        free(visited);
        queueDestroy(queue);
        return false;
    }

    visited[startIndex] = true;
    queuePush(queue, startIndex);

    while (!queueIsEmpty(queue)) {
        int u = 0;
        queuePop(queue, &u);
        visit(u, g->vertices[u].data, userData);

        for (const EdgeNode* edge = g->vertices[u].firstEdge; edge; edge = edge->next) {
            if (!visited[edge->dest]) {
                visited[edge->dest] = true;
                queuePush(queue, edge->dest);
            }
        }
    }

    free(visited);
    queueDestroy(queue);
    return true;
}

bool graphDijkstra(const Graph* g, int startIndex, int* dist, int* prev) {
    if (!isVertexIndex(g, startIndex) || !dist || !prev) {
        return false;
    }

    int count = g->vertexCount;
    bool* visited = (bool*)calloc((size_t)count, sizeof(bool));
    if (!visited) {
        return false;
    }

    for (int i = 0; i < count; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[startIndex] = 0;

    for (int step = 0; step < count; step++) {
        int u = -1;
        int best = INT_MAX;
        for (int i = 0; i < count; i++) {
            if (!visited[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }

        // 剩下的都不可达
        if (u == -1) {
            break;
        }
        visited[u] = true;

        for (const EdgeNode* edge = g->vertices[u].firstEdge; edge; edge = edge->next) {
            // 负权会让 Dijkstra 失效，跳过；顺便防一下加法溢出
            if (edge->weight < 0 || dist[u] > INT_MAX - edge->weight) {
                continue;
            }

            int candidate = dist[u] + edge->weight;
            if (candidate < dist[edge->dest]) {
                dist[edge->dest] = candidate;
                prev[edge->dest] = u;
            }
        }
    }

    free(visited);
    return true;
}

int graphBuildPath(const int* prev, int vertexCount, int end, int* path, int capacity) {
    if (!prev || !path || vertexCount <= 0 || capacity <= 0) {
        return -1;
    }
    if (end < 0 || end >= vertexCount) {
        return -1;
    }

    // 从终点往回走，拿到的是倒序路径
    int length = 0;
    int current = end;
    while (current != -1) {
        if (current < 0 || current >= vertexCount || length >= capacity) {
            return -1;
        }
        path[length++] = current;
        current = prev[current];
    }

    for (int i = 0, j = length - 1; i < j; i++, j--) {
        int temp = path[i];
        path[i] = path[j];
        path[j] = temp;
    }
    return length;
}

int graphInDegree(const Graph* g, int index) {
    if (!isVertexIndex(g, index)) {
        return -1;
    }

    int inDegree = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        for (const EdgeNode* edge = g->vertices[i].firstEdge; edge; edge = edge->next) {
            if (edge->dest == index) {
                inDegree++;
            }
        }
    }
    return inDegree;
}

int graphOutDegree(const Graph* g, int index) {
    if (!isVertexIndex(g, index)) {
        return -1;
    }

    int outDegree = 0;
    for (const EdgeNode* edge = g->vertices[index].firstEdge; edge; edge = edge->next) {
        outDegree++;
    }
    return outDegree;
}

void graphPrint(const Graph* g) {
    if (!g) {
        printf("(null graph)\n");
        return;
    }

    printf("Graph (%s, %d vertices):\n",
           g->isDirected ? "directed" : "undirected",
           g->vertexCount);

    for (int i = 0; i < g->vertexCount; i++) {
        printf("Vertex[%d](data=%d):", i, g->vertices[i].data);

        const EdgeNode* edge = g->vertices[i].firstEdge;
        if (!edge) {
            printf(" (no edges)");
        }
        while (edge) {
            printf(" -> [%d](weight=%d)", edge->dest, edge->weight);
            edge = edge->next;
        }
        printf("\n");
    }
}

static bool isVertexIndex(const Graph* g, int index) {
    return g && index >= 0 && index < g->vertexCount;
}

static bool removeDirectedEdge(Graph* g, int from, int to) {
    EdgeNode* prev = NULL;
    EdgeNode* curr = g->vertices[from].firstEdge;

    while (curr) {
        if (curr->dest == to) {
            if (prev) {
                prev->next = curr->next;
            } else {
                g->vertices[from].firstEdge = curr->next;
            }
            free(curr);
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

static Queue* queueCreate(int capacity) {
    if (capacity <= 0) {
        return NULL;
    }

    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        return NULL;
    }

    q->items = (int*)malloc(sizeof(int) * (size_t)capacity);
    if (!q->items) {
        free(q);
        return NULL;
    }

    q->head = 0;
    q->tail = 0;
    q->capacity = capacity;
    return q;
}

static void queueDestroy(Queue* q) {
    if (!q) {
        return;
    }
    free(q->items);
    free(q);
}

static bool queueIsEmpty(const Queue* q) {
    return !q || q->head == q->tail;
}

static bool queuePush(Queue* q, int value) {
    if (!q || q->tail >= q->capacity) {
        return false;
    }

    q->items[q->tail++] = value;
    return true;
}

static bool queuePop(Queue* q, int* value) {
    if (queueIsEmpty(q) || !value) {
        return false;
    }

    *value = q->items[q->head++];
    return true;
}
