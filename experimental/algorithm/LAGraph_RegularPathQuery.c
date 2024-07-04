#define LG_FREE_WORK                            \
{                                               \
    GrB_free (&frontier) ;                      \
    GrB_free (&next_frontier) ;                 \
    GrB_free (&symbol_frontier) ;               \
    LAGraph_Free ((void **) &A, NULL) ;     \
    LAGraph_Free ((void **) &AT, NULL) ;     \
    LAGraph_Free ((void **) &B, NULL) ;     \
    LAGraph_Free ((void **) &BT, NULL) ;     \
}

#define LG_FREE_ALL                 \
{                                   \
    LG_FREE_WORK ;                  \
    GrB_free (reachable) ;         \
}

#include "LG_internal.h"
#include "LAGraphX.h"

//------------------------------------------------------------------------------
// LAGraph_RegularPathQuery: searching for paths satisfying a finite automata
//------------------------------------------------------------------------------

int LAGraph_RegularPathQuery
(
    // output:
    GrB_Matrix *reachable,      // reachable(i, j): wether the node j is
                                // rechable in state i
    // input:
    LAGraph_Graph *G,           // input graphs
    int32_t nl,                 // label count
    LAGraph_Graph *R,           // input automaton graphs
    GrB_Index qs,               // starting state in regular automaton
    const GrB_Index *sources,   // source vertices to start searching paths
    int32_t ns,                 // number of source vertices
    char *msg
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    LG_CLEAR_MSG ;

    GrB_Matrix frontier = NULL ;
    GrB_Matrix symbol_frontier = NULL ;
    GrB_Matrix next_frontier = NULL ;

    GrB_Index ng = 0 ;                   // # nodes in the graph
    GrB_Index nr = 0 ;                   // # nodes in the automata

    GrB_Semiring bool_semiring = LAGraph_any_one_bool;

    GrB_Matrix *A;
    GrB_Matrix *AT;
    GrB_Matrix *B;
    GrB_Matrix *BT;

    LG_ASSERT (reachable != NULL && G != NULL && R != NULL && sources != NULL, GrB_NULL_POINTER) ;
    (*reachable) = NULL ;
    for (int32_t i = 0; i < nl; i++)
    {
        if (G[i] == NULL) continue;
        LG_TRY (LAGraph_CheckGraph (G[i], msg)) ;
    }
    for (int32_t i = 0; i < nl; i++)
    {
        if (R[i] == NULL) continue;
        LG_TRY (LAGraph_CheckGraph (R[i], msg)) ;
    }

    LG_TRY (LAGraph_Malloc ((void **) &A, nl, sizeof (GrB_Matrix), msg)) ;
    LG_TRY (LAGraph_Malloc ((void **) &AT, nl, sizeof (GrB_Matrix), msg)) ;

    for (int32_t i = 0; i < nl; i++) {
        if (G[i] == NULL) { 
            A[i] = 0;
            continue;
        }

        A[i] = G[i]->A;
        if (G[i]->kind == LAGraph_ADJACENCY_UNDIRECTED ||
            G[i]->is_symmetric_structure == LAGraph_TRUE)
        {
            AT[i] = A[i];
        }
        else
        {
            AT[i] = G[i]->AT ;
            LG_ASSERT_MSG (AT[i] != NULL, LAGRAPH_NOT_CACHED, "G->AT is required") ;
        }
    }

    LG_TRY (LAGraph_Malloc ((void **) &B, nl, sizeof (GrB_Matrix), msg)) ;
    LG_TRY (LAGraph_Malloc ((void **) &BT, nl, sizeof (GrB_Matrix), msg)) ;

    for (int32_t i = 0; i < nl; i++) 
    {
        if (R[i] == NULL) {
            B[i] = 0;
            continue;
        }

        B[i] = R[i]->A;
        if (R[i]->kind == LAGraph_ADJACENCY_UNDIRECTED ||
            R[i]->is_symmetric_structure == LAGraph_TRUE)
        {
            BT[i] = B[i];
        }
        else
        {
            BT[i] = R[i]->AT ;
            LG_ASSERT_MSG (BT[i] != NULL, LAGRAPH_NOT_CACHED, "R->AT is required") ;
        }
    }

    // =========================================================================
    // === initializations =====================================================
    // =========================================================================

    for (int i = 0; i < nl; i++) {
        if (A[i] == NULL) continue;
        GRB_TRY (GrB_Matrix_nrows (&ng, A[i])) ;
        break;
    }
    for (int i = 0; i < nl; i++) {
        if (B[i] == NULL) continue;
        GRB_TRY (GrB_Matrix_nrows (&nr, B[i])) ;
        break;
    }

    // Initialize next frontier with source nodes
    GRB_TRY (GrB_Matrix_new (&next_frontier, GrB_BOOL, nr, ng)) ;
    GRB_TRY (GrB_Matrix_new (reachable, GrB_BOOL, nr, ng)) ;

    for (GrB_Index i = 0; i < ns; i++)
    {
        GrB_Index src = sources [i] ;
        LG_ASSERT_MSG (src < ng, GrB_INVALID_INDEX, "invalid source node") ;
        GRB_TRY (GrB_Matrix_setElement (next_frontier, true, qs, src)) ;
        GRB_TRY (GrB_Matrix_setElement (*reachable, true, qs, src)) ;
    }

    GRB_TRY (GrB_Matrix_new (&frontier, GrB_BOOL, nr, ng)) ;
    GRB_TRY (GrB_Matrix_new (&symbol_frontier, GrB_BOOL, nr, ng)) ;

    GrB_Index states = ns;
    while (states != 0)
    {
        GrB_Matrix old_frontier = frontier;
        frontier = next_frontier;
        next_frontier = old_frontier;

        GRB_TRY (GrB_Matrix_clear(next_frontier));

        for (int32_t i = 0; i < nl; i++) {
            if (A[i] == NULL || B[i] == NULL) continue;
            GRB_TRY (GrB_mxm (symbol_frontier, NULL, NULL, GrB_LOR_LAND_SEMIRING_BOOL, BT[i], frontier, GrB_DESC_R)) ;
            GRB_TRY (GrB_mxm (next_frontier, *reachable, GrB_LOR, GrB_LOR_LAND_SEMIRING_BOOL, symbol_frontier, A[i], GrB_DESC_SC)) ;
        }

        GRB_TRY (GrB_assign (*reachable, *reachable, GrB_NULL, next_frontier, GrB_ALL, nr, GrB_ALL, ng, GrB_DESC_SC));
        GRB_TRY (GrB_Matrix_nvals(&states, next_frontier));
    }


    LG_FREE_WORK ;
    return (GrB_SUCCESS) ;
}
