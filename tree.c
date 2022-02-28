#include <stdlib.h>

struct tree_s *newTree(void)
{
    struct tree_s *tre = (struct tree_s*) calloc(1, sizeof(struct tree_s));
    return tre;
}