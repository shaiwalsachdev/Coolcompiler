%{
/*	
*	Lexical analyzer
*/

#include	<string.h>
#include	<stdlib.h>	
#include	"coolc.tab.h"
#include	"symbol.h"

extern int	lineno;		

void lex_init()
{
}

#define	YY_USER_INIT	lex_init();
%}

uppercase		[A-Z]
lowercase		[a-z]
digit			[0-9]
alphanum		[A-Za-z0-9]
alphanumu		[A-Za-z0-9_]
alphastr		[ A-Za-z0-9_\-*/.:()]

%%

\/\/.* ;				/* ignore comments */
\/\*(.*\n)*.*\*\/ ;  /* ignore comments */
[ \t]			break;		/* ignore white space */
"\n" 			++lineno; 

class			{ printf("'%s\' on line #%d\n",yytext,lineno);
				  return CLASS; }
is			{ 	  printf("'%s\' on line #%d\n",yytext,lineno);
				  return IS; }

if			{ 	  printf("'%s\' on line #%d\n",yytext,lineno);
				  return IF; }

then			{ printf("'%s\' on line #%d\n",yytext,lineno); 
				  return THEN; }
else			{ printf("'%s\' on line #%d\n",yytext,lineno);
				  return ELSE; }
fi				{ printf("'%s\' on line #%d\n",yytext,lineno);
				  return FI; }

while			{ printf("'%s\' on line #%d\n",yytext,lineno);
				  return WHILE; }
loop			{ printf("'%s\' on line #%d\n",yytext,lineno);
				  return LOOP; }
pool			{ printf("'%s\' on line #%d\n",yytext,lineno);
				   return POOL; }

begin			{ printf("'%s\' on line #%d\n",yytext,lineno);
				   return BBEGIN; }
end			    { printf("'%s\' on line #%d\n",yytext,lineno);
				return END; }

true			{ printf("'%s\' on line #%d\n",yytext,lineno);
					return TRUE; }
false			{ printf("'%s\' on line #%d\n",yytext,lineno);
				  return FALSE; }
not			{ 	printf("'%s\' on line #%d\n",yytext,lineno);
				return NOT; }

print			{ printf("'%s\' on line #%d\n",yytext,lineno);
					return PRINT; }

{uppercase}{alphanumu}*	{	/* Class name */
				printf("'%s\' on line #%d\n",yytext,lineno);
				yylval.cValue=strdup(yytext);
				return TYPEID;
			}

{lowercase}{alphanumu}*	{	/* Identifier */
				printf("'%s\' on line #%d\n",yytext,lineno);
				yylval.cValue=strdup(yytext);
				return ID;
			}

{digit}+		{	/* Some number */
								printf("'%s\' on line #%d\n",yytext,lineno);
                                yylval.iValue=atoi(yytext);
				return NUMBER;
			}

\"{alphastr}*\"		{	/* String enclosed in quotes */
				yylval.cValue=strdup(yytext);
				return STRING;
			}

"("			return LPAREN;
")"			return RPAREN;
"{"			return LBRACE;
"}"			return RBRACE;
"="			return EQUAL;
","			return COLON;
";"			return SEMICOLON;
":"			return DOTS;
"."			return DOT;
"+"			return PLUS;
"-"			return MINUS;
"<-"		return ASSIGN;

.			{
				fprintf(stderr,	"Illegal character \'%s\' on line #%d\n",yytext,lineno);
				exit(1);
			}

%%

int yywrap()
{ return 1; }

