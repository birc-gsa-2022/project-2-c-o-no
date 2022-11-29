#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include "suffix_tree.h"

/*
 * Follows an edge (Range r)
 * Takes an integer current_matched_chars and returns newly matched characters on the given edge
 */
int follow_edge(struct Range r, int current_matched_chars, const char *suffix, const char *str) {
    int i;
    for (i = 0; i+r.start < r.end; i++) {
        // Break as soon we can't match any more on the edge
        if (suffix[current_matched_chars+i] != str[r.start+i]) break;
    }
    return i;
}

// Splits an edge in the suffix tree
void split_edge(struct SuffixTree *st, int suffix_len, int str_len, struct SuffixTreeNode *subtree, int matched_characters, int matches) {
    st->st_pool->next++;
    struct SuffixTreeNode *fst_child = st->st_pool->next;
    fst_child->range.start = str_len-suffix_len+matched_characters;
    fst_child->range.end = str_len;
    fst_child->leaf_label = str_len-suffix_len;
    fst_child->parent = subtree;

    st->st_pool->next++;
    struct SuffixTreeNode *snd_child = st->st_pool->next;
    fst_child->sibling = snd_child;

    snd_child->parent = subtree;
    snd_child->child = subtree->child;
    snd_child->leaf_label = subtree->leaf_label;
    /*
     * We only want to add 'matches' to range.start and not matched_characters
     * This is because 'matches' only go from range.start
     * (in contrast to matched_characters, which is all the way from the root)
     */
    snd_child->range.start = subtree->range.start+matches;
    snd_child->range.end = subtree->range.end;

    // Now as the split has been created, we have to clean up the edge from the subtree to the subtree parent
    subtree->child = fst_child;
    subtree->range.end = subtree->range.start+matches;
    subtree->leaf_label = -1;
}

void insert_node(struct SuffixTree *st, char *suffix, int suffix_len, char *str, int str_len) {
    // If we have not inserted anything yet, we would like to insert the suffix directly from the root
    struct SuffixTreeNode *place_to_insert = st->root;
    struct SuffixTreeNode *child = st->root->child;
    int matched_characters = 0;

    // Try to follow edges starting from the root
    while (child != NULL) {
        /*
         * If we have matched everything, we should just append the child at
         * the current place_to_insert instead of keep searching
         */
        if (matched_characters == suffix_len) break;

        // Check if the edge is matching characters in the suffix
        int matches = follow_edge(child->range, matched_characters, suffix, str);
        matched_characters += matches;

        // Check if we can not follow the edge. If not; check sibling.
        if (!matches) {
            child = child->sibling;
            continue;
        }

        /*
         * If we have matched all characters from start to end on an edge,
         * we should explore the child of the current child
         */
        if (matches == child->range.end-child->range.start) {
            place_to_insert = child;
            child = child->child;
            continue;
        }

        /*
         * At this point forward we know, that we only matched SOME characters; we should split the edge
         */
        split_edge(st, suffix_len, str_len, child, matched_characters, matches);

        return;
    }

    // Here we assume, that place_to_insert has been set correctly
    st->st_pool->next++;
    struct SuffixTreeNode *new_node = st->st_pool->next;
    new_node->range.start = str_len-suffix_len;
    new_node->range.end = str_len-matched_characters;
    new_node->leaf_label = str_len-suffix_len;

    new_node->parent = place_to_insert;
    new_node->sibling = place_to_insert->child;
    place_to_insert->child = new_node;
}

struct SuffixTree *construct_st(char *str) {
    int n = (int) strlen(str);
    // Edge case for when n is 1
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

    for (int i = 0; i <= n; i++) {
        char *suffix = &str[i];
        insert_node(st, suffix, n-i, str, n);
    }

    return st;
}


// I think range_of_string will only be used in testing?
char *range_of_string(struct Range r, char *str) {
    int len = r.end-r.start;
    char *new_str = malloc(len+1);
    memcpy(new_str, str+r.start, len);
    new_str[len] = '\0';
    return new_str;
}

struct SearchResults *get_leafs(struct SuffixTreeNode *node, struct SearchResults *sr) {
    if (node == NULL) return sr;
    // Check if node is a leaf
    if (node->leaf_label != -1) {
        //printf("%d ", node->leaf_label+1);
        sr->position[sr->total_search_results] = node->leaf_label+1;
        sr->total_search_results++;
    }
    get_leafs(node->sibling, sr);
    get_leafs(node->child, sr);
}

struct SearchResults *search(struct SuffixTree *st, char *pattern, int pattern_len, char *search_string, int search_string_len) {
    struct SuffixTreeNode *child = st->root->child;
    int matched_characters = 0;

    while (child != NULL) {
        int matches = follow_edge(child->range, matched_characters, pattern, search_string);
        matched_characters += matches;
        // can_follow now holds an int of how many characters we took down the edge
        if (matched_characters == pattern_len) break;

        if (matches) {
            child = child->child;
            continue;
        }
        child = child->sibling;
    }

    struct SearchResults *sr = malloc(sizeof *sr);
    sr->total_search_results = 0;
    sr->position = malloc(search_string_len*sizeof sr->position);
    if (matched_characters == pattern_len) {
        return get_leafs(child, sr);
    } else {
        return sr;
    }

}


