%{
#include "lex_stuff.H"
%}

%%

^#.*$				{ ; }
^network			{ handle_lex_token(NETWORK,yytext); }
^box				{ handle_lex_token(BOX,yytext); }
^arc				{ handle_lex_token(ARC,yytext); }
^input				{ handle_lex_token(INPUT,yytext); }
[A-Za-z]+			{ handle_lex_token(TEXT_TOK,yytext); }
-?[0-9]+			{ handle_lex_token(INT_TOK,yytext); }
-?[0-9]*\.[0-9]+	{ handle_lex_token(FLOAT_TOK,yytext); }
[A-Za-z]+$			{ handle_lex_token(TEXT_TOK_EOL,yytext); }
\> |
\< |
\= |
\>\= |
\<\=		{ handle_lex_token(PREDICATE_OPERATOR,yytext); }

[\t ]+		{ /* ignore white space */; }
\n          { ; }
.           { handle_lex_token(JUNK,yytext); }
%%
void lexer(char *filename)
{
    FILE *fp;

    fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr,"Could not open %s\n",filename);
        exit(1);
    }
    yyin = fp;
    yylex();
}

