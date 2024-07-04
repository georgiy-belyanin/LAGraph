#include <stdio.h>
#include <acutest.h>
#include <LG_Xtest.h>
#include <LG_test.h>
#include <LAGraphX.h>
#include <LAGraph_test.h>

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
    { { "rpq_data/a.mtx", "rpq_data/b.mtx", NULL }, { "rpq_data/1_a.mtx", NULL }, "rpq_data/1_meta.txt", "rpq_data/1_sources.txt" },
    { { "rpq_data/a.mtx", "rpq_data/b.mtx", NULL }, { "rpq_data/2_a.mtx", "rpq_data/2_b.mtx", NULL }, "rpq_data/2_meta.txt", "rpq_data/2_sources.txt" },
    { { "rpq_data/a.mtx", "rpq_data/b.mtx", NULL }, { "rpq_data/3_a.mtx", "rpq_data/3_b.mtx", NULL }, "rpq_data/3_meta.txt", "rpq_data/3_sources.txt" },
    { { "rpq_data/a.mtx", "rpq_data/b.mtx", NULL }, { "", "rpq_data/4_b.mtx", NULL }, "rpq_data/4_meta.txt", "rpq_data/4_sources.txt" },
    { NULL, NULL, NULL, NULL  },
} ;

//****************************************************************************
void test_RegularPathQuery (void)
{
    LAGraph_Init (msg) ;

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
            OK (fclose (f));

            OK (LAGraph_New (&(G[i]), &A, LAGraph_ADJACENCY_DIRECTED, msg)) ;
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

            OK (LAGraph_New (&(R[i]), &A, LAGraph_ADJACENCY_DIRECTED, msg)) ;
            OK (LAGraph_Cached_AT (R[i], msg)) ;
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

        OK (LAGraph_RegularPathQuery(&reachable, G, MAX_LABELS, R, qs, sources, ns, msg)) ;
        printf("%s\n", msg);
        GxB_print(reachable, 3);

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
    {"RegularPathQuery", test_RegularPathQuery},
    {NULL, NULL}
};
