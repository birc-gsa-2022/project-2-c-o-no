#include <malloc.h>
#include <stdlib.h>
#include "minunit.h"
#include "../src/suffix_tree.h"

void test_setup(void) {

}

void test_teardown(void) {
    //Nothing
}

// Compares node_edge with all edges and returns which has matched.
// Returns 0 if no match.
int get_edge_position(char *node_edge, char *edge_cmp1, char *edge_cmp2, char *edge_cmp3) {
    if (strcmp(node_edge,edge_cmp1) == 0) return 2;
    if (strcmp(node_edge,edge_cmp2) == 0) return 3;
    if (strcmp(node_edge,edge_cmp3) == 0) return 5;
    return 0;
}

MU_TEST(test_atataa_constr) {
    /* This test is slightly complicated; however it is a little neat.
     * It does not care about the order of edge labels. This works by using primes; see get_edge_position.
     * So as long the tree is "semantically" correct, this test should pass.
     * It also tests for child/parent relationships and null-pointer issues
     *
     * (Admittedly, this way of testing is way too thorough as, although it is very low coupled to the implementation,
     * I later switched to testing in another way)
     */

    // The string we should build a suffix tree for is ATATAA
    char *str = "ATATAA";
    struct SuffixTree *st = construct_st(str);

    // At root layer
    mu_assert_int_eq(0, st->root->range.start);
    mu_assert_int_eq(0, st->root->range.end);
    mu_assert_int_eq(-1, st->root->leaf_label);
    mu_assert(st->root->parent == NULL, "Root parent should be NULL");
    mu_assert(st->root->sibling == NULL, "Root should not have siblings");

    // At depth 1 we should expect 3 edges: \0   TA   A
    struct SuffixTreeNode *node1 = st->root->child;
    mu_assert(node1 != NULL, "Node 1 is NULL");
    struct SuffixTreeNode *node2 = node1->sibling;
    mu_assert(node2 != NULL, "Node 1 is NULL");
    struct SuffixTreeNode *node3 = node2->sibling;
    mu_assert(node3 != NULL, "Node 3 is NULL");

    char *node1_str = range_of_string(node1->range, str);
    char *node2_str = range_of_string(node2->range, str);
    char *node3_str = range_of_string(node3->range, str);
    char *edge_1 = "";
    char *edge_2 = "TA";
    char *edge_3 = "A";

    // Get 3 primes.
    int node_prime1 = get_edge_position(node1_str, edge_1, edge_2, edge_3);
    int node_prime2 = get_edge_position(node2_str, edge_1, edge_2, edge_3);
    int node_prime3 = get_edge_position(node3_str, edge_1, edge_2, edge_3);
    // If all edges have been found, we should have a prime factor of 2*3*5=30
    mu_assert_int_eq(30, node_prime1*node_prime2*node_prime3);

    // We'll use "Known nodes" (for lack of a better word) to determine which are which.
    // We want [0] to be "", [1] to be "A", [2] to be "TA"
    struct SuffixTreeNode *known_nodes[3];
    int primes[3] = {2,3,5};
    for (int i = 0; i < 3; i++) {
        if (node_prime1 == primes[i]) known_nodes[i] = node1;
        if (node_prime2 == primes[i]) known_nodes[i] = node2;
        if (node_prime3 == primes[i]) known_nodes[i] = node3;
    }

    // The node with ingoing edge "" should have leaf_label 6. This is the last suffix.
    mu_assert_int_eq(6, known_nodes[0]->leaf_label);
    mu_assert(known_nodes[0]->child == NULL, "Leaf 6 should not have children");
    mu_assert_string_eq("", range_of_string(known_nodes[0]->range, str));
    mu_assert(known_nodes[0]->parent == st->root, "Leaf 6 should have root as parent");

    mu_assert_int_eq(-1, known_nodes[1]->leaf_label);
    mu_assert_int_eq(-1, known_nodes[2]->leaf_label);

    mu_assert(known_nodes[1]->parent == st->root, "Known_node 1 should have root as parent");
    mu_assert(known_nodes[2]->parent == st->root, "Known_node 2 should have root as parent");

    mu_assert(known_nodes[2]->sibling == NULL, "Known_node 2 should not have a sibling (end of linked list)");

    // Does not test the whole tree, but it is good enough.

}


MU_TEST(test_naive_insert_ATATAA) {
    char *str = "ATATAA";

    // Subroutine of construction:
    int n = (int) strlen(str);
    int num_of_nodes = n == 1 ? 2 : (2*n+1);
    struct SuffixTree *st = malloc(sizeof *st);
    struct SuffixTreeNodePool *st_pool = malloc(sizeof *st_pool);
    struct SuffixTreeNode *pool_nodes = malloc(num_of_nodes*sizeof(struct SuffixTreeNode));
    st_pool->nodes = pool_nodes;
    st_pool->next = pool_nodes;

    st->st_pool = st_pool;
    st->root = st->st_pool->next;
    st->root->range.start = 0;
    st->root->range.end = 0;
    st->root->leaf_label = -1;
    st->root->child = NULL;

    char *suffix1 = "ATATAA";
    insert_node(st, suffix1, (int) strlen(suffix1), str, (int) strlen(str));

    mu_assert_string_eq(suffix1, range_of_string(st->root->child->range, str));
    mu_assert_int_eq(0, st->root->child->leaf_label);

    char *suffix2 = "TATAA";
    insert_node(st, suffix2, (int) strlen(suffix2), str, (int) strlen(str));

    mu_assert_string_eq(suffix2, range_of_string(st->root->child->range, str));
    mu_assert_int_eq(1, st->root->child->leaf_label);

    mu_assert_string_eq(suffix1, range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->leaf_label);

    char *suffix3 = "ATAA";
    insert_node(st, suffix3, (int) strlen(suffix3), str, (int) strlen(str));

    mu_assert_string_eq(suffix2, range_of_string(st->root->child->range, str));
    mu_assert_int_eq(1, st->root->child->leaf_label);

    mu_assert_string_eq("ATA", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->child->sibling->leaf_label);

    char *suffix4 = "TAA";
    insert_node(st, suffix4, (int) strlen(suffix4), str, (int) strlen(str));

    mu_assert_string_eq("ATA", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("TA", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(3, st->root->child->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->child->sibling->leaf_label);

    char *suffix5 = "AA";
    insert_node(st, suffix5, (int) strlen(suffix5), str, (int) strlen(str));

    mu_assert_string_eq("TA", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(3, st->root->child->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("TA", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->child->sibling->child->sibling->leaf_label);

    char *suffix6 = "A";
    insert_node(st, suffix6, (int) strlen(suffix6), str, (int) strlen(str));

    mu_assert_string_eq("TA", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(3, st->root->child->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("TA", range_of_string(st->root->child->sibling->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->sibling->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->child->sibling->sibling->child->sibling->leaf_label);

    char *suffix7 = "";
    insert_node(st, suffix7, (int) strlen(suffix7), str, (int) strlen(str));

    mu_assert_string_eq("", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(6, st->root->child->leaf_label);

    mu_assert_string_eq("TA", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(3, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("", range_of_string(st->root->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("TA", range_of_string(st->root->child->sibling->sibling->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("A", range_of_string(st->root->child->sibling->sibling->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->sibling->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("TAA", range_of_string(st->root->child->sibling->sibling->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->child->sibling->sibling->child->sibling->leaf_label);

}

MU_TEST(test_naive_insert_mississippi) {
    char *str = "mississippi";

    // Subroutine of construction:
    int n = (int) strlen(str);
    int num_of_nodes = n == 1 ? 2 : (2*n+1);
    struct SuffixTree *st = malloc(sizeof *st);
    struct SuffixTreeNodePool *st_pool = malloc(sizeof *st_pool);
    struct SuffixTreeNode *pool_nodes = malloc(num_of_nodes*sizeof(struct SuffixTreeNode));
    st_pool->nodes = pool_nodes;
    st_pool->next = pool_nodes;

    st->st_pool = st_pool;
    st->root = st->st_pool->next;
    st->root->range.start = 0;
    st->root->range.end = 0;
    st->root->leaf_label = -1;
    st->root->child = NULL;

    char *suffix1 = "mississippi";
    insert_node(st, suffix1, (int) strlen(suffix1), str, (int) strlen(str));

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(0, st->root->child->leaf_label);

    char *suffix2 = "ississippi";
    insert_node(st, suffix2, (int) strlen(suffix2), str, (int) strlen(str));

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("ississippi", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(1, st->root->child->leaf_label);

    char *suffix3 = "ssissippi";
    insert_node(st, suffix3, (int) strlen(suffix3), str, (int) strlen(str));

    mu_assert_string_eq("ssissippi", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(2, st->root->child->leaf_label);

    mu_assert_string_eq("ississippi", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->leaf_label);

    char *suffix4 = "sissippi";
    insert_node(st, suffix4, (int) strlen(suffix4), str, (int) strlen(str));

    mu_assert_string_eq("s", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("issippi", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(3, st->root->child->child->leaf_label);

    mu_assert_string_eq("sissippi", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("ississippi", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->leaf_label);

    char *suffix5 = "issippi";
    insert_node(st, suffix5, (int) strlen(suffix5), str, (int) strlen(str));

    mu_assert_string_eq("s", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("issippi", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(3, st->root->child->child->leaf_label);

    mu_assert_string_eq("sissippi", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("issi", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->child->sibling->leaf_label);

    char *suffix6 = "ssippi";
    insert_node(st, suffix6, (int) strlen(suffix6), str, (int) strlen(str));

    mu_assert_string_eq("s", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("issippi", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(3, st->root->child->child->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("issi", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->child->sibling->leaf_label);

    char *suffix7 = "sippi";
    insert_node(st, suffix7, (int) strlen(suffix7), str, (int) strlen(str));

    mu_assert_string_eq("s", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(-1, st->root->child->child->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->child->child->range, str));
    mu_assert_int_eq(6, st->root->child->child->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->child->child->sibling->range, str));
    mu_assert_int_eq(3, st->root->child->child->child->sibling->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("issi", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->child->sibling->leaf_label);

    char *suffix8 = "ippi";
    insert_node(st, suffix8, (int) strlen(suffix8), str, (int) strlen(str));

    mu_assert_string_eq("s", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(-1, st->root->child->child->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->child->child->range, str));
    mu_assert_int_eq(6, st->root->child->child->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->child->child->sibling->range, str));
    mu_assert_int_eq(3, st->root->child->child->child->sibling->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssi", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->child->sibling->child->sibling->leaf_label);

    char *suffix9 = "ppi";
    insert_node(st, suffix9, (int) strlen(suffix9), str, (int) strlen(str));

    mu_assert_string_eq("ppi", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(8, st->root->child->leaf_label);

    mu_assert_string_eq("s", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->child->range, str));
    mu_assert_int_eq(6, st->root->child->sibling->child->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->child->sibling->range, str));
    mu_assert_int_eq(3, st->root->child->sibling->child->child->sibling->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("ssi", range_of_string(st->root->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->sibling->child->sibling->child->sibling->leaf_label);

    char *suffix10 = "pi";
    insert_node(st, suffix10, (int) strlen(suffix10), str, (int) strlen(str));

    mu_assert_string_eq("p", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(9, st->root->child->child->leaf_label);

    mu_assert_string_eq("pi", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(8, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("s", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->child->range, str));
    mu_assert_int_eq(6, st->root->child->sibling->child->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->child->sibling->range, str));
    mu_assert_int_eq(3, st->root->child->sibling->child->child->sibling->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("ssi", range_of_string(st->root->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->sibling->child->sibling->child->sibling->leaf_label);

    char *suffix11 = "i";
    insert_node(st, suffix11, (int) strlen(suffix11), str, (int) strlen(str));

    mu_assert_string_eq("p", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(-1, st->root->child->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->child->range, str));
    mu_assert_int_eq(9, st->root->child->child->leaf_label);

    mu_assert_string_eq("pi", range_of_string(st->root->child->child->sibling->range, str));
    mu_assert_int_eq(8, st->root->child->child->sibling->leaf_label);

    mu_assert_string_eq("s", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("", range_of_string(st->root->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(10, st->root->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->child->sibling->range, str));
    mu_assert_int_eq(3, st->root->child->sibling->child->child->sibling->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ssi", range_of_string(st->root->child->sibling->sibling->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->sibling->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->sibling->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->sibling->child->sibling->sibling->child->sibling->leaf_label);

    char *suffix12 = "";
    insert_node(st, suffix12, (int) strlen(suffix12), str, (int) strlen(str));

    mu_assert_string_eq("", range_of_string(st->root->child->range, str));
    mu_assert_int_eq(11, st->root->child->leaf_label);

    mu_assert_string_eq("p", range_of_string(st->root->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->child->range, str));
    mu_assert_int_eq(9, st->root->child->sibling->child->leaf_label);

    mu_assert_string_eq("pi", range_of_string(st->root->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(8, st->root->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("s", range_of_string(st->root->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("", range_of_string(st->root->child->sibling->sibling->sibling->child->range, str));
    mu_assert_int_eq(10, st->root->child->sibling->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->sibling->child->child->sibling->range, str));
    mu_assert_int_eq(3, st->root->child->sibling->sibling->child->child->sibling->leaf_label);

    mu_assert_string_eq("si", range_of_string(st->root->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->child->sibling->child->range, str));
    mu_assert_int_eq(5, st->root->child->sibling->sibling->child->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->sibling->child->sibling->child->sibling->range, str));
    mu_assert_int_eq(2, st->root->child->sibling->sibling->child->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("mississippi", range_of_string(st->root->child->sibling->sibling->sibling->sibling->range, str));
    mu_assert_int_eq(0, st->root->child->sibling->sibling->sibling->sibling->leaf_label);

    mu_assert_string_eq("i", range_of_string(st->root->child->sibling->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(7, st->root->child->sibling->sibling->sibling->child->sibling->leaf_label);

    mu_assert_string_eq("ssi", range_of_string(st->root->child->sibling->sibling->sibling->child->sibling->sibling->range, str));
    mu_assert_int_eq(-1, st->root->child->sibling->sibling->sibling->child->sibling->sibling->leaf_label);

    mu_assert_string_eq("ppi", range_of_string(st->root->child->sibling->sibling->sibling->child->sibling->sibling->child->range, str));
    mu_assert_int_eq(4, st->root->child->sibling->sibling->sibling->child->sibling->sibling->child->leaf_label);

    mu_assert_string_eq("ssippi", range_of_string(st->root->child->sibling->sibling->sibling->child->sibling->sibling->child->sibling->range, str));
    mu_assert_int_eq(1, st->root->child->sibling->sibling->sibling->child->sibling->sibling->child->sibling->leaf_label);

}



MU_TEST(test_range_1) {
    struct Range range;
    range.start = 3;
    range.end = 7;

    char *str = "ABCDEFGH";

    char *expected = "DEFG";
    char *substr = range_of_string(range, str);

    mu_assert_string_eq(expected, substr);
}

MU_TEST(test_range_2) {
    struct Range range;
    range.start = 1;
    range.end = 1;

    char *str = "A";

    char *expected = "";
    char *substr = range_of_string(range, str);

    mu_assert_string_eq(expected, substr);
}

MU_TEST(test_range_3) {
    struct Range range;
    range.start = 0;
    range.end = 1;

    char *str = "A";

    char *expected = "A";
    char *substr = range_of_string(range, str);

    mu_assert_string_eq(expected, substr);
}

void run_all_tests() {
    MU_RUN_TEST(test_range_1);
    MU_RUN_TEST(test_range_2);
    MU_RUN_TEST(test_range_3);
    MU_RUN_TEST(test_naive_insert_ATATAA);
    MU_RUN_TEST(test_naive_insert_mississippi);
    MU_RUN_TEST(test_atataa_constr);
}

MU_TEST_SUITE(fasta_parser_test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    run_all_tests();
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(fasta_parser_test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}