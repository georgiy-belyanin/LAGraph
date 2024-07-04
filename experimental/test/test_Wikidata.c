#include <stdio.h>
#include <acutest.h>
#include <LG_Xtest.h>
#include <LG_test.h>
#include <LAGraphX.h>
#include <LAGraph_test.h>
#include <time.h>

#define LEN 512
#define MAX_LABELS 3

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

const matrix_info files [ ] =
{
    { { "rpq_data/410.mtx", NULL }, { "rpq_data/62.mtx", NULL }, "rpq_data/62_meta.txt", "rpq_data/62_sources.txt" },
   { NULL, NULL, NULL, NULL  },
} ;

//****************************************************************************
void test_Wikidata (void)
{
    LAGraph_Init (msg) ;
    struct timespec start, finish;

    for (int k = 0 ; ; k++)
    {
        if (files[k].sources == NULL) break;
        TEST_CASE ("Testing") ;
 
        for (int i = 0; ; i++ ) {
            const char *name = files[k].graphs[i] ;

            if (name == NULL) break;
            if (strlen(name) == 0) continue;


            snprintf (filename, LEN, LG_DATA_DIR "%s", name) ;
            FILE *f = fopen (filename, "r") ;
            TEST_CHECK (f != NULL) ;
            OK (LAGraph_MMRead (&A, f, msg)) ;
            printf("%s\n", msg);
            OK (fclose (f));

            OK (LAGraph_New (&(G[i]), &A, LAGraph_ADJACENCY_UNDIRECTED, msg)) ;
            TEST_CHECK (A == NULL) ;
        }

        for (int i = 0; ; i++ ) {
            const char *name = files[k].fas[i] ;

            if (name == NULL) break;
            if (strlen(name) == 0) continue;

            snprintf (filename, LEN, LG_DATA_DIR "%s", name) ;
            FILE *f = fopen (filename, "r") ;
            TEST_CHECK (f != NULL) ;
            OK (LAGraph_MMRead (&A, f, msg)) ;
            OK (fclose (f));

            OK (LAGraph_New (&(R[i]), &A, LAGraph_ADJACENCY_UNDIRECTED, msg)) ;
            TEST_CHECK (A == NULL) ;
        }

        int qs;
        uint64_t source;
        uint64_t sources[16];
        int ns = 0;

        const char *name = files[k].sources;

        snprintf (filename, LEN, LG_DATA_DIR "%s", name) ;
        FILE *f = fopen (filename, "r") ;
        TEST_CHECK (f != NULL) ;
        while(fscanf(f, "%ld", &source) != EOF)
            sources[ns++] = source - 1;
        OK(fclose(f));

        name = files[k].fa_meta;

        snprintf (filename, LEN, LG_DATA_DIR "%s", name) ;
        f = fopen (filename, "r") ;
        TEST_CHECK (f != NULL) ;
        fscanf(f, "%d", &qs);
        OK(fclose(f));
        qs--;


        GrB_Matrix reachable = NULL ;

        clock_gettime(CLOCK_MONOTONIC, &start);
        OK (LAGraph_RegularPathQuery(&reachable, G, MAX_LABELS, R, qs, sources, ns, msg)) ;
        clock_gettime(CLOCK_MONOTONIC, &finish);
        double elapsed = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        GxB_print(reachable, 1);
        printf("%f\n", elapsed);

        OK (GrB_free (&reachable)) ;
        for (int i = 0; i < MAX_LABELS; i++ ) {
            if (G[i] == NULL) continue;
            OK (LAGraph_Delete (&(G[i]), msg)) ;
        }

        for (int i = 0; i < MAX_LABELS ; i++ ) {
            if (R[i] == NULL) continue;
            OK (LAGraph_Delete (&(R[i]), msg)) ;
        }
    }

    LAGraph_Finalize (msg) ;
}


TEST_LIST = {
    {"Wikidata", test_Wikidata},
    {NULL, NULL}
};
