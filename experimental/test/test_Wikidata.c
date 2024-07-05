#include <stdio.h>
#include <acutest.h>
#include <LG_Xtest.h>
#include <LG_test.h>
#include <LAGraphX.h>
#include <LAGraph_test.h>
#include <time.h>

#define LEN 512
#define MAX_LABELS 16

#define WIKIDATA_DIR "Wikidata/"
#define QUERIES_DIR "Queries/"

#define QUERY_COUNT 660

char msg [LAGRAPH_MSG_LEN] ;
LAGraph_Graph G[MAX_LABELS];
LAGraph_Graph R[MAX_LABELS];
GrB_Matrix A;


char filename [LEN+1] ;

typedef struct
{
    const char* graphs[MAX_LABELS] ;
    const char* fas[MAX_LABELS] ;
    const char *fa_meta;
    const char *sources;
}
matrix_info ;


void test_Wikidata (void)
{
    LAGraph_Init (msg) ;
    struct timespec start, finish;

    for (int query = 0; query < QUERY_COUNT; query++) {
        snprintf (filename, LEN, QUERIES_DIR "%d/meta.txt", query) ;
        FILE *f = fopen (filename, "r") ;
        if (f == NULL) continue;

        uint64_t source, dest;

        fscanf(f, "%ld", &source);
        fscanf(f, "%ld", &dest);

        if (source == 0) continue;

        source--;
        dest--;

        uint64_t label;
        uint64_t labels[MAX_LABELS];
        uint64_t nl = 0;
        while (fscanf(f, "%ld", &label) != EOF)
            labels[nl++] = label;
        fclose(f);

        for (int i = 0; i < nl; i++) {
            snprintf (filename, LEN, WIKIDATA_DIR "%ld.txt", labels[i]) ;
            FILE *f = fopen (filename, "r") ;
            if (f == NULL) continue;

            OK (LAGraph_MMRead (&A, f, msg)) ;
            OK (LAGraph_New (&(G[i]), &A, LAGraph_ADJACENCY_UNDIRECTED, msg)) ;
            fclose(f);
        }

        for (int i = 0; i < nl; i++) {
            snprintf (filename, LEN, QUERIES_DIR "%d/%ld.txt", query, labels[i]) ;
            FILE *f = fopen (filename, "r") ;
            if (f == NULL) continue;

            OK (LAGraph_MMRead (&A, f, msg)) ;
            OK (LAGraph_New (&(R[i]), &A, LAGraph_ADJACENCY_UNDIRECTED, msg)) ;
            fclose(f);
        }

        uint64_t sources[1] = { source };
        GrB_Matrix reachable = NULL ;

        clock_gettime(CLOCK_MONOTONIC, &start);
        OK (LAGraph_RegularPathQuery(&reachable, G, MAX_LABELS, R, 0, sources, 1, msg)) ;
        clock_gettime(CLOCK_MONOTONIC, &finish);
        double elapsed = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        GxB_print(reachable, 1);
        printf("%d %fmus\n", query, elapsed * 1000000);

        OK (GrB_free (&reachable)) ;

    }

    LAGraph_Finalize (msg) ;
}


TEST_LIST = {
    {"Wikidata", test_Wikidata},
    {NULL, NULL}
};
