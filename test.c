#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

struct command{
    char *cmd;
    char **argv;
    int fd[2];
};

struct tree_node_s  {
    void *data;
    struct tree_node_s *parent;
    struct tree_node_s *right;
    struct tree_node_s *left;
};

struct tree_node_s *find_root(struct tree_node_s *tre_nde);
int is_root(const struct tree_node_s *tre_nde);
struct tree_node_s *insertNode(struct tree_node_s *tre_rt, void *val);
int is_leaf(struct tree_node_s *tre_nde);
int right_open(struct tree_node_s *tre_nde);
int left_open(struct tree_node_s *tre_nde);
void executePipes(const struct tree_node_s *root);
struct tree_node_s *createTreeNode(void *data, struct tree_node_s *parent, struct tree_node_s *left, struct tree_node_s *right);
struct tree_node_s *getSibling(struct tree_node_s *tre_nde);
struct command *getPrevCommand(struct tree_node_s *tre_nde);
int is_left(struct tree_node_s *tre_nde);
int is_right(struct tree_node_s *tre_nde);

int main()
{
    struct tree_node_s *root = createTreeNode(1, NULL, NULL, NULL);
    struct command *cmd1 = (struct command*) calloc(1, sizeof(struct command));
    cmd1->cmd="ls";
    char *argv1[4] = { "ls", "-l", "-a", NULL };
    cmd1->argv=argv1;
    struct command *cmd2 = (struct command*) calloc(1, sizeof(struct command));
    cmd2->cmd="tail";
    char *argv2[4] = { "tail", "-n", "1", NULL };
    cmd2->argv=argv2;
    struct command *cmd3 = (struct command*) calloc(1, sizeof(struct command));
    cmd3->cmd="sort";
    char *argv3[3] = { "sort", "--random-sort", NULL };
    cmd3->argv=argv3;
    root = insertNode(root, (struct command *) cmd1);
    root = insertNode(root, (struct command *) cmd2);
//    root = insertNode(root, (struct command *) cmd3);

    executePipes(root);
}

struct command *getPrevCommand(struct tree_node_s *tre_nde){
    if (is_left(tre_nde))
        return NULL;
    if (tre_nde->parent->left) {
        if (tre_nde->parent->left->right) {
            struct tree_node_s *till_right = tre_nde->parent->left->right;
            while (till_right->right != NULL) {
                till_right = till_right->right;
            }
            return ((struct command*)till_right->data);
        } else{
            return ((struct command*) tre_nde->parent->left->data);
        }

    }
    return ((struct command*) tre_nde->parent->left->right->data);
}

int is_right(struct tree_node_s *tre_nde) {
    if (tre_nde->parent == NULL)
        return 0;
    if (tre_nde->parent->right != tre_nde)
        return 0;
    return 1;
}

int is_left(struct tree_node_s *tre_nde) {
    if (tre_nde->parent == NULL)
        return 0;
    if (tre_nde->parent->left != tre_nde)
        return 0;
    return 1;
}

struct tree_node_s *getSibling(struct tree_node_s *tre_nde) {
    if (tre_nde->parent == NULL)
        return NULL;
    if (is_left(tre_nde))
        return tre_nde->parent->right;
    if (is_right(tre_nde))
        return tre_nde->parent->left;
}

void executePipes(const struct tree_node_s *root)
{
    if (root->left)
        executePipes(root->left);
    if (is_leaf(root)) {
        struct command *prevCmd = getPrevCommand(root);
        struct command *currentCmd = (struct command*)root->data;

        /* create pipe for current cmd if it is not the right most node */
        if (!(is_root(root->parent) && is_right(root))) {
            int pres = pipe(currentCmd->fd);
            if (pres == -1)
                exit(EXIT_FAILURE);
        }

        pid_t cpid = fork();
        if (cpid == 0)
        {
            printf("Hello what's going on %d\n", getpid());
            if (prevCmd)
            {
                /* close stdin and replace with previous command's pipe read */
                printf("%s stdin is changed with pipe.\n", currentCmd->argv[0]);
                close(prevCmd->fd[1]);
                dup2(prevCmd->fd[0], STDIN_FILENO);
                close(prevCmd->fd[0]);
                printf("Prev Cmd pipes: %d, %d\n", prevCmd->fd[0], prevCmd->fd[1]);
            }
            /* close stdout and replace with current command's pipe write */
            if (!(currentCmd->fd[0] == 0 && currentCmd->fd[1] == 0)) {
                printf("%s stdout is changed with pipe.\n", currentCmd->argv[0]);
                close(currentCmd->fd[0]);
                dup2(currentCmd->fd[1], STDOUT_FILENO);
                close(currentCmd->fd[1]);
            }
            /*
             * current command will read from prev. command's pipe and
             * will write to it's created pipe so next command can use it's read pipe
             */
            
            execvp(currentCmd->argv[0], currentCmd->argv);
        } else if (cpid > 0)
        {
            int res;
            printf("Executing %s \n", currentCmd->argv[0]);
            printf("Current pipes: %d, %d\n", currentCmd->fd[0], currentCmd->fd[1]);

            printf("prev cmd: %d\n", prevCmd);
            waitpid(NULL, &res, 0);
            printf("%s exited with status code %d \n", currentCmd->argv[0], res);
            return WIFEXITED(res) ? WEXITSTATUS(res) : -WTERMSIG(res);
        }
    }
    if (root->right)
        executePipes(root->right);
}

struct tree_node_s *createTreeNode(void *data, struct tree_node_s *parent, struct tree_node_s *left, struct tree_node_s *right){
    struct tree_node_s *tre_nde = (struct tree_node_s*) calloc(1, sizeof(struct tree_node_s));
    tre_nde->data=data;
    tre_nde->parent=parent;
    tre_nde->left=left;
    tre_nde->right=right;
    return tre_nde;
}

int left_open(struct tree_node_s *tre_nde) {
    if (tre_nde->left == NULL)
        return 1;
    return 0;
}
int right_open(struct tree_node_s *tre_nde) {
    if (tre_nde->right == NULL)
        return 1;
    return 0;
}

int is_leaf(struct tree_node_s *tre_nde)
{
    if(tre_nde->right == NULL && tre_nde->left == NULL)
        return 1;
    return 0;
}

struct tree_node_s *find_root(struct tree_node_s *tre_nde)
{
    if (is_root(tre_nde))
        return tre_nde;
    find_root(tre_nde->parent);
}

int is_root(const struct tree_node_s *tre_nde)
{
    if (tre_nde->parent == NULL)
        return 1;
    return 0;
}

struct tree_node_s *insertNode(struct tree_node_s *tre_rt, void* val)
{
    if (is_root(tre_rt) && is_leaf(tre_rt)) {
        /*
         * first insertion
         */
        tre_rt->left=createTreeNode(val, tre_rt, NULL, NULL);
        return tre_rt;
    } else if (is_root(tre_rt) && right_open(tre_rt)) {
        /*
         * if it is root node but not leaf node check right child
         */
        tre_rt->right=createTreeNode(val, tre_rt, NULL, NULL);
        return tre_rt;
    } else if (is_root(tre_rt) && !is_leaf(tre_rt)){
        /*
         * if it is root node, and it has both left and right child create new root node
         * and make the old root node left child of new root node
         */
        struct tree_node_s *nde = createTreeNode(val, NULL, NULL, NULL);
        struct tree_node_s *new_rt = createTreeNode(&*((int *)tre_rt->data) + 1,
                NULL,
                tre_rt,
                nde);
        nde->parent=new_rt;
        tre_rt->parent=new_rt;
        return new_rt;
    }
}
