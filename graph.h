#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

// 邻接表的边节点
typedef struct EdgeNode {
    int dest;
    int weight;
    struct EdgeNode* next;
} EdgeNode;

typedef struct {
    int data;
    EdgeNode* firstEdge;
} Vertex;

// vertices 是一块连续内存，满了之后按两倍扩容
typedef struct {
    Vertex* vertices;
    int vertexCount;
    int capacity;
    bool isDirected;
} Graph;

// 遍历时每个顶点回调一次，userData 原样传回
typedef void (*GraphVisitFn)(int index, int data, void* userData);

// capacity 只是初始值，后面不够会自动扩容
Graph* graphCreate(int capacity, bool isDirected);
void graphDestroy(Graph* g);
bool graphReserve(Graph* g, int capacity);
int graphAddVertex(Graph* g, int data);
bool graphRemoveVertex(Graph* g, int index);
int graphGetVertexIndex(const Graph* g, int data);
int graphGetVertexData(const Graph* g, int index);  // 非法下标返回 INT_MIN
int graphVertexCount(const Graph* g);
bool graphAddEdge(Graph* g, int from, int to, int weight);
bool graphRemoveEdge(Graph* g, int from, int to);
bool graphHasEdge(const Graph* g, int from, int to);
int graphGetEdgeWeight(const Graph* g, int from, int to);  // 没有这条边返回 INT_MAX

// 只遍历起点所在的连通分量
bool graphDFS(const Graph* g, int startIndex, GraphVisitFn visit, void* userData);
bool graphBFS(const Graph* g, int startIndex, GraphVisitFn visit, void* userData);

// 边权按非负处理，负权边会被跳过
bool graphDijkstra(const Graph* g, int startIndex, int* dist, int* prev);

// 从 prev 还原起点到 end 的路径，返回顶点个数，空间不够返回 -1
int graphBuildPath(const int* prev, int vertexCount, int end, int* path, int capacity);

int graphInDegree(const Graph* g, int index);
int graphOutDegree(const Graph* g, int index);
void graphPrint(const Graph* g);

#endif
