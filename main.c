#include "graph.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static void printVertex(int index, int data, void* userData);

int main(void) {
    printf("=== Graph demo ===\n\n");

    // 无向图
    printf("[Undirected graph]\n");
    Graph* undirected = graphCreate(10, false);
    if (!undirected) {
        fprintf(stderr, "Failed to create graph.\n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        graphAddVertex(undirected, i);
    }

    graphAddEdge(undirected, 0, 1, 1);
    graphAddEdge(undirected, 0, 2, 1);
    graphAddEdge(undirected, 1, 3, 1);
    graphAddEdge(undirected, 2, 3, 1);
    graphAddEdge(undirected, 3, 4, 1);

    graphPrint(undirected);

    printf("\nDFS: ");
    graphDFS(undirected, 0, printVertex, NULL);
    printf("\nBFS: ");
    graphBFS(undirected, 0, printVertex, NULL);
    printf("\nDegree of vertex 0: %d\n", graphOutDegree(undirected, 0));

    graphDestroy(undirected);
    printf("Undirected graph destroyed.\n\n");

    // 有向带权图
    printf("[Directed weighted graph]\n");
    Graph* directed = graphCreate(10, true);
    if (!directed) {
        fprintf(stderr, "Failed to create graph.\n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        graphAddVertex(directed, i);
    }

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

    printf("\nDFS: ");
    graphDFS(directed, 0, printVertex, NULL);
    printf("\nBFS: ");
    graphBFS(directed, 0, printVertex, NULL);
    printf("\nVertex 1 in-degree: %d, out-degree: %d\n",
           graphInDegree(directed, 1),
           graphOutDegree(directed, 1));

    int count = graphVertexCount(directed);
    int* dist = (int*)malloc(sizeof(int) * (size_t)count);
    int* prev = (int*)malloc(sizeof(int) * (size_t)count);
    int* path = (int*)malloc(sizeof(int) * (size_t)count);
    if (!dist || !prev || !path) {
        fprintf(stderr, "Out of memory.\n");
        free(dist);
        free(prev);
        free(path);
        graphDestroy(directed);
        return 1;
    }

    printf("\nShortest paths from vertex 0:\n");
    graphDijkstra(directed, 0, dist, prev);
    for (int i = 0; i < count; i++) {
        if (dist[i] == INT_MAX) {
            printf("To vertex[%d]: unreachable\n", i);
            continue;
        }

        int length = graphBuildPath(prev, count, i, path, count);
        printf("To vertex[%d]: distance=%d, path=", i, dist[i]);
        for (int j = 0; j < length; j++) {
            printf("%d ", path[j]);
        }
        printf("\n");
    }

    free(dist);
    free(prev);
    free(path);

    printf("\nAfter removing edge 0->1:\n\n");
    graphRemoveEdge(directed, 0, 1);
    graphPrint(directed);

    graphDestroy(directed);
    printf("Directed graph destroyed.\n");

    return 0;
}

static void printVertex(int index, int data, void* userData) {
    (void)index;
    (void)userData;
    printf("%d ", data);
}
