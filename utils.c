#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "utils.h"

/*  
 *  Creates a new Argument Node and set the next ArgNode to pNext if not NULL
 */
struct argNode *newArgNode(char *val, struct argNode *pNext)
{
    struct argNode *pTmpArgN;
    pTmpArgN = (struct argNode *)calloc(1, sizeof(struct argNode));
    pTmpArgN->argVal = strndup(val, strlen(val) + 1);
    if (pNext)
        pTmpArgN->pNext = pNext;
    return pTmpArgN;
}

/*  
 *  Recursively frees all the allocated memory by given Argument Node DS 
 */
void freeArgNode(struct argNode *tmpArgNode) {
    if (tmpArgNode->argVal) {
        free(tmpArgNode->argVal);
    }
    if (tmpArgNode->pNext) {
        freeArgNode(tmpArgNode->pNext);
    }
    free(tmpArgNode);
}

/* 
 *  Creates a new redirect_s ds to be processed in command grammer   
 */
struct redirect_s *newRedirect(char *filename, enum redirect_type type)
{
    struct redirect_s *tmp_redirect = (struct redirect_s *)calloc(1, sizeof(struct redirect_s));
    tmp_redirect->filename = strndup(filename, strlen(filename) + 1);
    tmp_redirect->type = type;
    return tmp_redirect;
}

/*
 *  Frees all the allocated memory for given redirect_s struct pointer
 */
void freeRedirect(struct redirect_s *rdr)
{
    if (rdr->filename)
        free(rdr->filename);
    rdr->filename   =   NULL;
    free(rdr);
}

/*
 *  creates a new Command Struct from given command value and list of arguments (singly linked list)
 */
struct command *newCommand(char *cmdVal, int argc, struct argNode *pArgN, char *inputFilename, char *outputFilename)
{
    struct command *pTmpCommand = (struct command *)calloc(1, sizeof(struct command));
    pTmpCommand->cmdId = strndup(cmdVal, strlen(cmdVal) + 1);
    if (pArgN != NULL)
        pTmpCommand->pFirstArg = pArgN;
    if (inputFilename != NULL)
        pTmpCommand->inputFilename = strndup(inputFilename, strlen(inputFilename) + 1);
    else
        pTmpCommand->inputFilename = NULL;
    if (outputFilename != NULL)
        pTmpCommand->outputFilename = strndup(outputFilename, strlen(outputFilename) + 1);
    else
        pTmpCommand->outputFilename = NULL;
    return pTmpCommand;
}

/*  
 *  Frees all the allocated memory by given Command Data Structure
 */
void freeCommand(struct command *cmd)
{
    if (!cmd)
        return;
    if (cmd->cmdId)
        free(cmd->cmdId);
    if (cmd->inputFilename)
        free(cmd->inputFilename);
    if (cmd->outputFilename)
        free(cmd->outputFilename);
    if (cmd->pFirstArg)
        freeArgNode(cmd->pFirstArg);
    cmd->pFirstArg=NULL;
    free(cmd);
    cmd=NULL;
}

/*
 *  Process given redirect struct pointer and sets the appropriate redirectFilename such as input or output for given Command Struct pointer
 */
struct command *newCommandWithRedirectFilename(struct command *cmd, struct redirect_s *rdr)
{
    switch (rdr->type)
    {
    case (enum redirect_type)IN:
        cmd->inputFilename = strndup(rdr->filename, strlen(rdr->filename) + 1);
        break;
    case (enum redirect_type)OUT:
        cmd->outputFilename = strndup(rdr->filename, strlen(rdr->filename) + 1);
        break;
    default:
        break;
    }
    freeRedirect(rdr);
    return cmd;
}

/*
 *  convert command struct into list of strings which includes
 *  command_identifier arg#1 arg#2 arg#N ...
 */
char **ls2argv(const struct command *cmd)
{
    char **argv = NULL;
    argv = (char **)malloc(sizeof(char *));
    argv[0] = strndup(cmd->cmdId, strlen(cmd->cmdId) + 1);
    if (!cmd->pFirstArg)
        return argv;
    struct argNode *pTrvrs = cmd->pFirstArg;
    int argc = 1;
    while (pTrvrs != NULL)
    {
        argv = (char **)realloc(argv, sizeof(char *) * (argc + 1));
        argv[argc] = strndup(pTrvrs->argVal, strlen(pTrvrs->argVal) + 1);
        pTrvrs = pTrvrs->pNext;
        argc++;
    }
    argv = (char **)realloc(argv, sizeof(char *) * (argc + 1));
    argv[argc] = NULL;
    return argv;
}

/*  
 *  DEBUG printer to test if the given Command properly initialized     
 */
void printCommand(const struct command *cmd)
{
    printf("Command ID: (%s)\n", cmd->cmdId);
    if (cmd->inputFilename != NULL)
        printf("\tRedirect-In (%s)\n", cmd->inputFilename);
    if (cmd->outputFilename != NULL)
        printf("\tRedirect-Out (%s)\n", cmd->outputFilename);
    struct argNode *pTrvs;
    pTrvs = cmd->pFirstArg;
    while (pTrvs != NULL)
    {
        printf("\t\tArg: (%s)\n", pTrvs->argVal);
        pTrvs = pTrvs->pNext;
    }
}

void redirectInput(char *infile, int *saved_stdin_fd)
{
    /* Save the STDIN file descriptor */
    *saved_stdin_fd = dup(STDIN_FILENO);
    /* open input file for read */
    int infile_fd = open(infile, O_RDONLY);
    /* 
     *  Close standard input at position 0 in file descriptiors list 
     *  and replace it with newly opened file. 
     */
    dup2(infile_fd, STDIN_FILENO);
    close(infile_fd);
}

void redirectOutput(char *outfile, int *saved_stdout_fd)
{
    /* Save the STDIN file descriptor */
    *saved_stdout_fd = dup(STDOUT_FILENO);
    /* open input file for read */
    int outfile_fd = open(outfile, O_CREAT | O_WRONLY);
    /* 
     *  Close standard input at position 0 in file descriptiors list 
     *  and replace it with newly opened file. 
     */
    dup2(outfile_fd, STDOUT_FILENO);
    close(outfile_fd);
}

void executeCommand(const struct command *excCmd)
{
    int status;
    pid_t cpid;
    if ((cpid = fork()) > 0)
    {
        /* 
         *  Parent
         *  Wait for child process to finish.
         */
        printf("\nDEBUG:\tPARENT:\t\tWaiting for Child#%d\n\n", cpid);
        waitpid(cpid, &status, 0);
        printf("\nDEBUG:\tPARENT:\t\tChild#%d exited with status code %d\n\n", cpid, status);
    }
    else if (cpid == 0)
    {
        
        /* 
         *  Child
         *  create a argument array from command identifier and argument linked list from command structure.
         */
        char **argv = ls2argv(excCmd);
        
        /* Handle input redirect */
        int saved_stdin;
        if (excCmd->inputFilename)
            redirectInput(excCmd->inputFilename, &saved_stdin);
        /* Handle output redirect */
        int saved_stdout;
        if (excCmd->outputFilename)
            redirectOutput(excCmd->outputFilename, &saved_stdout);
        
        /* Execute given command */
        execvp(excCmd->cmdId, argv);

        /* Restore STDIN and STDOUT */
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);

        /* Free allocated memory for argument list array */
        int i = 0;
        while (*(argv + i) != NULL)
        {
            free(*(argv + i++));
        }
        free(argv);
    }
    else
    {
        printf("Error: CANNOT create child process!");
        /* error cannot fork */
    }
}