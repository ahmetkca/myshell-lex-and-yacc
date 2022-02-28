#ifndef TREE_H_
#define TREE_H_

struct tree_node_s  {
    void *data;
    struct tree_node_s *right;
    struct tree_node_s *left;
};

struct tree_s {
    struct tree_node_s *root_node;
};

struct tree_s *newTree(void);



#endif