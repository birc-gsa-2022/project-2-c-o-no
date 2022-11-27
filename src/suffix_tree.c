#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include "suffix_tree.h"

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
        // Todo: Refactor the for-loop to function "follow-edge" that returns (int: matched_characters)
        for (int i = matched_characters; i < child->range.end; i++) {
            if (suffix[i] == str[child->range.start+i]) {
                matched_characters++;
            } else {
                // Early stopping on unmatched characters
                break;
            }
        }

        if (matched_characters) {
            // We can follow the edge
            if (matched_characters != child->range.end) {
                // We did not match all characters; we should split

                // steal the address of the current child which functions as a root in this subtree
                struct SuffixTreeNode *subtree = child;

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

                //snd_child->sibling = subtree->sibling;

                snd_child->leaf_label = subtree->leaf_label;
                snd_child->range.start = subtree->range.start+matched_characters;
                snd_child->range.end = subtree->range.end;

                /*
                 * Now as the split has been created, we have to clean up the edge from the
                 * subtree to the subtree parent
                 */
                subtree->child = fst_child;
                subtree->range.end = subtree->range.start+matched_characters;
                subtree->leaf_label = -1;

                return;
            } else {
                // We have matched all characters, so we should explore the child of the current child
                place_to_insert = child;
                child = child->child;
                continue;
            }
        }

        // Explore all siblings
        child = child->sibling;
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

void search(struct SuffixTree st, char *pattern) {
    //todo
}


