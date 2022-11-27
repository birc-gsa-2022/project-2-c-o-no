#ifndef SUFFIX_TREE_H
#define SUFFIX_TREE_H

struct Range {
    int start;
    int end;
};

struct SuffixTreeNode {
    int leaf_label;
    struct Range range;
    struct SuffixTreeNode *parent;
    struct SuffixTreeNode *child;
    struct SuffixTreeNode *sibling;
};

struct SuffixTreeNodePool {
    struct SuffixTreeNode *nodes;
    struct SuffixTreeNode *next;
};

struct SuffixTree {
    struct SuffixTreeNode *root;
    struct SuffixTreeNodePool *st_pool;
};

struct SuffixTree *construct_st(char *str);
void insert_node(struct SuffixTree *st, char *suffix, int suffix_len, char *str, int str_len);
char *range_of_string(struct Range r, char *str);
void search(struct SuffixTree st, char *pattern);

#endif //SUFFIX_TREE_H
