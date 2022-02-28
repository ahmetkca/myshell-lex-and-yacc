%{
    // #define YYSTYPE char *
    static int num_args = 0;
    static int is_first = 0;
    #include <string.h>
    #include "myshell_parser.tab.h"
    void yyerror(char *);
%}

%x args_start redirect_in_start redirect_out_start

cmd             [a-zA-Z\.][a-zA-Z0-9\-_/]*
arg             [a-zA-Z\.\-0-9]([a-zA-Z0-9\-_/]?)*
filename        [a-zA-Z\._0-9\-][a-zA-Z0-9\._\-]*

/* redirect_in     ((\<+([\ ]+|[\t]+))|\<) */
/* redirect_out    ((\>+([\ ]+|[\t]+))|\>) */
/* pipe            ((\|+([\ ]+|[\t]+))|\|) */
redirect_in     "<"
redirect_out    ">"
pipe            [\|]

%%

{cmd}   {
    /* 
     *  if there is no special start condition (default condition is INITIAL)
     *  we can expect input to be command identifier.
     *  after entering command, the argument list condition will be enabled.
     */
    // if (is_first == 0) {
    //     printf("first ");
    //     is_first = 1;
    // }
    // printf("command (%s)\n", yytext);
    num_args = 0;
    BEGIN(args_start);
    yylval.str = strndup(yytext, yyleng + 1);
    return CMD;
}

<args_start>{arg}   {
    /* 
     *  after command identifier is entered we can expect arguments
     */
    // printf("arg#%d (%s)(length=%d)\n",++num_args, yytext, yyleng);
    yylval.str = strndup(yytext, yyleng + 1);
    return ARG;
}

<args_start,redirect_in_start,redirect_out_start>{pipe}  {
    /* 
     *  after command is entered we can expect pipe operation 
     */
    // printf(" pipe found! (%s)\n", yytext);
    BEGIN(INITIAL);
    return PIPE;
}

<args_start,redirect_in_start,redirect_out_start>{redirect_in}   {
    /* 
     *  command arguments entered and found '>' symbol at the end of argument list
     *  begin redirect-in condition to retrieve filename
     */
    // printf(" redirect-in found! ");
    BEGIN(redirect_in_start);
    return REDIRECT_IN;
}

<redirect_in_start>{filename} {
    /* 
     *  after redirect-in symbol '>' 
     *  expect filename
     *  until next character is either redirect-in, redirect-out or pipe the condition
     *  will stay in the redirect-in state
     */
    // printf("filename (%s) ", yytext);
    yylval.str = strndup(yytext, yyleng);
    return FILENAME;
    // BEGIN(INITIAL);
}

<args_start,redirect_in_start,redirect_out_start>{redirect_out}  {
    /* 
     *  command arguments entered and found '<' symbol at the end of argument list
     *  begin redirect-out condition to retrieve filename
     *  enters redirect-out state
     */
    // printf(" redirect-out found! ");
    BEGIN(redirect_out_start);
    return REDIRECT_OUT;
}

<redirect_out_start>{filename} {
    /* 
     *  after redirect-out symbol '<' 
     *  expect filename
     *  till next character is either redirect-in, redirect-out or pipe
     *  the condition will stay as redirect-out
     */
    // printf("filename (%s) ", yytext);
    yylval.str = strndup(yytext, yyleng);
    return FILENAME;
    // BEGIN(INITIAL);
}

<args_start,redirect_in_start,redirect_out_start,INITIAL>\n  {
    /* newline (Terminator) resets to INITIAL Condition */
    is_first = 0;
    BEGIN(INITIAL);
    return TERMINATE;
}

<INITIAL,args_start,redirect_in_start,redirect_out_start>[ \t]     { 
    printf("(%s)\n", yytext); 
}

%%

int yywrap(void) {
    return 1;
}