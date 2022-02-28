%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include "utils.h"

    #define MAX_ARG_VALUE_LEN 32;

    int yylex(void);
    void yyerror(char *);
%}

%union
{
    char chr;
    char *str;
    struct argNode *pArgNode;
    struct command *pCommand;
    struct redirect_s *pRedirect;
}
%token <str> CMD
%token <str> ARG
%token <str> FILENAME
%token REDIRECT_IN
%token REDIRECT_OUT
%token PIPE
%token TERMINATE
%type <pArgNode> args
%type <pCommand> command;
%type <pRedirect> redirect;


%%

start:  /* empty */
        |
        start command TERMINATE { 
            // printCommand($2); 
            executeCommand($2);
            freeCommand($2); 
            return 0;
        }
        ;

command:
        command redirect   {
            $$ = newCommandWithRedirectFilename($1, $2);
        }
        |
        CMD args        {
                /*          $1 = CMD (char*)    $2 = args (singly linked list)  */
                /* 
                 *  this grammer syntax is for handling following instance of input:
                 *          
                 *          (internal or external command           (list of arguments)
                 *              such as `cd`, `dir`, `python`)      
                 *          programName                             arg#1    arg#2    arg#3    arg#N
                 *      
                 */
                $$ = newCommand($1, 0, $2, NULL, NULL);
                /* free memory of token CMD that has been allocated in lexer by strndup */
                free($1);
            }
        |
        CMD             {
                /*          $1 = CMD (char*)    $2 = args (singly linked list)  */
                /* 
                 *  this grammer handles only commands without any argument given to the command (program) 
                 */
                $$ = newCommand($1, 0, NULL, NULL, NULL);
                /* free memory of token CMD that has been allocated in lexer by strndup to correspond command identifier */
                free($1);
            }
        ;

redirect:
        REDIRECT_IN FILENAME    {
            $$ = newRedirect($2, (enum redirect_type)IN);
            /* free memory of token FILENAME that has been allocated in lexer by strndup to correspond Filename */
            free($2);
        }
        |
        REDIRECT_OUT FILENAME   {
            $$ = newRedirect($2, (enum redirect_type)OUT);
            /* free memory of token FILENAME that has been allocated in lexer by strndup to correspond Filename */
            free($2);
        }
        ;   

args:
    ARG args    {
            /*
             *      $1 is equals to ARG  ( char* ) string.
             *      $2 is equals to args ( argNode* ) singly linked list.
             *  Handles list of arguments in a Singly Linked List.
             *  Inserts the new ArgNode before the head of the args singly linked list.
             *  
             *         ArgNode for Token ARG
             *      ╭――――――――――――――――――――――――――╮
             *      │  String value of ARG($1) │
             *      ├――――――――――――――――――――――――――┤        (Head Node of the args)
             *      │ Pointer to args          ├―――――――→╭―――――――――――――――――――――╮
             *      ╰――――――――――――――――――――――――――╯        │ Argument value # 1  │        ╭――――――――――――――――――――――――╮           ╭――――――――――――――――――╮
             *                                          ├―――――――――――――――――――――┤        (First ArgNode after Head)           ( Number N ArgNode )
             *                                          │ Pointer to next arg ├―――――――→[    Next ArgNode # 2    ]――→ ... ――→[ Next ArgNode # N ]
             *                                          ╰―――――――――――――――――――――╯        ╰――――――――――――――――――――――――╯           ╰――――――――――――――――――╯
             *      
             *  
             */
            $$ = newArgNode($1, $2);
            /* free memory of token ARG that has been allocated in lexer by strndup to correspond argument value */
            free($1);
        }
    |
    ARG         {
            /* 
             *  Starting point of handling list of arguments 
             *  Use string value of ARG and create a ArgNode
             *  and set args to point to this newly created ArgNode (Head of the args singly linked list)
             */
            $$ = newArgNode($1, NULL);
            /* free memory of token ARG that has been allocated in lexer by strndup to correspond argument value */
            free($1);
        }
    ;

%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}

int parser_main(void) {
    do {
        printf("myshell >~ ");
        yyparse();
    } while(1);
    return 0;
}