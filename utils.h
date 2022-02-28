#ifndef UTILS_H_
#define UTILS_H_

/*
 *  Singly Linked List Node for Arguments for Commands
 *  Command DS will use this DS to store its arguments
 */
struct argNode
{
    char *argVal;
    // int     pos;
    struct argNode *pNext;
};

/*
 *  Redirect DS with filename and predefined redirect types such as input (IN) or output (OUT)
 *  this struct makes it easier to implement redirection to command in the grammer.
 */
enum redirect_type
{
    IN = 0,
    OUT
};
struct redirect_s
{
    char *filename;
    enum redirect_type type;
};

/*
 *  Command Data Structure to manage Commands
 */
struct command
{
    char *cmdId;
    int argc;
    struct argNode *pFirstArg;
    char *outputFilename;
    char *inputFilename;
};

struct argNode *newArgNode(char *val, struct argNode *pNext);

void freeArgNode(struct argNode *tmpArgNode);

struct redirect_s *newRedirect(char *filename, enum redirect_type type);

void freeRedirect(struct redirect_s *rdr);

struct command *newCommand(char *cmdVal, int argc, struct argNode *pArgN, char *inputFilename, char *outputFilename);

void freeCommand(struct command *cmd);

struct command *newCommandWithRedirectFilename(struct command *cmd, struct redirect_s *rdr);

char **ls2argv(const struct command *cmd);

void printCommand(const struct command *cmd);

void executeCommand(const struct command *excCmd);

#endif