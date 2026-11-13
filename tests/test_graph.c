#include "graph.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

#define MAX_VISITS 16

// 记录遍历结果
typedef struct {
    int items[MAX_VISITS];
    int count;
} VisitLog;

static void testCreateAndGrow(void);
static void testDirectedEdges(void);
static void testUndirectedEdges(void);
static void testRemoveVertex(void);
static void testTraversal(void);
static void testDijkstra(void);
static Graph* buildUndirectedDemo(void);
static Graph* buildDirectedDemo(void);
static void logVertex(int index, int data, void* userData);

int main(void) {
    testCreateAndGrow();
    testDirectedEdges();
    testUndirectedEdges();
    testRemoveVertex();
    testTraversal();
    testDijkstra();

    printf("All tests passed!\n");
    return 0;
}

// 扩容
static void testCreateAndGrow(void) {
    assert(graphCreate(0, false) == NULL);
    graphDestroy(NULL);

    Graph* g = graphCreate(2, false);
    assert(g != NULL);

    assert(graphAddVertex(g, 10) == 0);
    assert(graphAddVertex(g, 20) == 1);
    // 容量 2，加第三个时会扩容
    assert(graphAddVertex(g, 30) == 2);

    assert(graphVertexCount(g) == 3);
    assert(g->capacity >= 3);
    assert(graphGetVertexIndex(g, 30) == 2);
    assert(graphGetVertexIndex(g, 99) == -1);
    assert(graphGetVertexData(g, 1) == 20);
    assert(graphGetVertexData(g, 3) == INT_MIN);

    graphDestroy(g);
}

// 有向边
static void testDirectedEdges(void) {
    Graph* g = graphCreate(4, true);
    assert(g != NULL);
    for (int i = 0; i < 3; i++) {
        assert(graphAddVertex(g, i) == i);
    }

    assert(graphAddEdge(g, 0, 1, 7));
    assert(graphHasEdge(g, 0, 1));
    assert(!graphHasEdge(g, 1, 0));
    assert(graphGetEdgeWeight(g, 0, 1) == 7);
    assert(graphGetEdgeWeight(g, 0, 2) == INT_MAX);

    assert(!graphAddEdge(g, 0, 1, 9));
    assert(!graphAddEdge(g, 0, 3, 1));
    assert(!graphAddEdge(g, -1, 0, 1));

    assert(graphRemoveEdge(g, 0, 1));
    assert(!graphHasEdge(g, 0, 1));
    assert(!graphRemoveEdge(g, 0, 1));

    graphDestroy(g);
}

// 无向边
static void testUndirectedEdges(void) {
    Graph* g = graphCreate(4, false);
    assert(g != NULL);
    for (int i = 0; i < 3; i++) {
        graphAddVertex(g, i);
    }

    assert(graphAddEdge(g, 0, 1, 5));
    assert(graphHasEdge(g, 0, 1));
    assert(graphHasEdge(g, 1, 0));
    assert(graphGetEdgeWeight(g, 1, 0) == 5);
    assert(graphOutDegree(g, 0) == 1);
    assert(graphOutDegree(g, 1) == 1);

    assert(graphRemoveEdge(g, 0, 1));
    assert(!graphHasEdge(g, 0, 1));
    assert(!graphHasEdge(g, 1, 0));

    // 自环只存一条
    assert(graphAddEdge(g, 2, 2, 3));
    assert(graphOutDegree(g, 2) == 1);
    assert(graphHasEdge(g, 2, 2));
    assert(graphRemoveEdge(g, 2, 2));
    assert(!graphHasEdge(g, 2, 2));

    graphDestroy(g);
}

// 删顶点后下标前移
static void testRemoveVertex(void) {
    Graph* g = graphCreate(4, true);
    assert(g != NULL);
    assert(graphAddVertex(g, 10) == 0);
    assert(graphAddVertex(g, 20) == 1);
    assert(graphAddVertex(g, 30) == 2);
    assert(graphAddVertex(g, 40) == 3);

    assert(graphAddEdge(g, 0, 1, 1));
    assert(graphAddEdge(g, 0, 2, 1));
    assert(graphAddEdge(g, 2, 3, 1));
    assert(graphAddEdge(g, 3, 1, 1));

    assert(graphRemoveVertex(g, 1));
    assert(graphVertexCount(g) == 3);
    assert(graphGetVertexIndex(g, 30) == 1);
    assert(graphGetVertexData(g, 2) == 40);

    // 0->2 变成 0->1，2->3 变成 1->2
    assert(graphHasEdge(g, 0, 1));
    assert(graphHasEdge(g, 1, 2));
    assert(!graphHasEdge(g, 0, 2));
    assert(!graphHasEdge(g, 2, 0));
    assert(graphOutDegree(g, 0) == 1);
    assert(graphOutDegree(g, 2) == 0);
    assert(graphInDegree(g, 1) == 1);

    // 再删第一个顶点
    assert(graphRemoveVertex(g, 0));
    assert(graphVertexCount(g) == 2);
    assert(graphGetVertexIndex(g, 30) == 0);
    assert(graphHasEdge(g, 0, 1));
    assert(!graphRemoveVertex(g, 5));

    graphDestroy(g);
}

// 遍历顺序
static void testTraversal(void) {
    const int expectedDfs[] = {0, 2, 3, 4, 1};
    const int expectedBfs[] = {0, 2, 1, 3, 4};

    Graph* g = buildUndirectedDemo();
    VisitLog dfsLog = {{0}, 0};
    VisitLog bfsLog = {{0}, 0};

    assert(graphDFS(g, 0, logVertex, &dfsLog));
    assert(graphBFS(g, 0, logVertex, &bfsLog));

    assert(dfsLog.count == 5);
    assert(bfsLog.count == 5);
    for (int i = 0; i < 5; i++) {
        assert(dfsLog.items[i] == expectedDfs[i]);
        assert(bfsLog.items[i] == expectedBfs[i]);
    }

    assert(!graphDFS(g, 9, logVertex, &dfsLog));
    assert(!graphBFS(g, -1, logVertex, &bfsLog));
    assert(!graphDFS(g, 0, NULL, &dfsLog));

    graphDestroy(g);
}

// 最短路和路径还原
static void testDijkstra(void) {
    Graph* g = buildDirectedDemo();
    int count = graphVertexCount(g);

    int dist[5] = {0};
    int prev[5] = {0};
    int path[5] = {0};
    const int expectedDist[] = {0, 8, 5, 9, 7};

    assert(graphDijkstra(g, 0, dist, prev));
    for (int i = 0; i < count; i++) {
        assert(dist[i] == expectedDist[i]);
    }

    assert(graphBuildPath(prev, count, 4, path, count) == 3);
    assert(path[0] == 0);
    assert(path[1] == 2);
    assert(path[2] == 4);
    assert(graphBuildPath(prev, count, 0, path, count) == 1);
    assert(graphBuildPath(prev, count, 4, path, 2) == -1);
    assert(graphBuildPath(prev, count, 5, path, count) == -1);
    assert(!graphDijkstra(g, 9, dist, prev));

    graphDestroy(g);
}

// 和 README 里的示例图保持一致
static Graph* buildUndirectedDemo(void) {
    Graph* g = graphCreate(10, false);
    assert(g != NULL);

    for (int i = 0; i < 5; i++) {
        graphAddVertex(g, i);
    }
    graphAddEdge(g, 0, 1, 1);
    graphAddEdge(g, 0, 2, 1);
    graphAddEdge(g, 1, 3, 1);
    graphAddEdge(g, 2, 3, 1);
    graphAddEdge(g, 3, 4, 1);
    return g;
}

// 和 README 里的示例图保持一致
static Graph* buildDirectedDemo(void) {
    Graph* g = graphCreate(10, true);
    assert(g != NULL);

    for (int i = 0; i < 5; i++) {
        graphAddVertex(g, i);
    }
    graphAddEdge(g, 0, 1, 10);
    graphAddEdge(g, 0, 2, 5);
    graphAddEdge(g, 1, 2, 2);
    graphAddEdge(g, 1, 3, 1);
    graphAddEdge(g, 2, 1, 3);
    graphAddEdge(g, 2, 3, 9);
    graphAddEdge(g, 2, 4, 2);
    graphAddEdge(g, 3, 4, 4);
    graphAddEdge(g, 4, 3, 6);
    return g;
}

static void logVertex(int index, int data, void* userData) {
    (void)index;
    VisitLog* log = (VisitLog*)userData;
    if (log->count < MAX_VISITS) {
        log->items[log->count++] = data;
    }
}
