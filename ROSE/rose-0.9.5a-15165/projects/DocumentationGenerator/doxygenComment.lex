/*
   File for scanning doxygen comments and exctracting info from them.
   Originally written by Peter.
   Modified by Ramakrishna.
*/
CMD  ("\\"|"@")
B    [ \t]
NB   [^ \t]
W    [ \t\n]
NW   [^ \t\n]

%x MAIN
%x PROTO
%x NAME

%option noyywrap

%{

#include <doxygenGrammar.h>
#include <doxygenComment.h>
#include <iostream>

using namespace std;

static DxNodeList *currentList;
static DxNode *currentNode;
static int braceCount;

static void nextNode(DxNode *n) 
   {
     currentList->push_back(n);
     currentNode = n;
   }

static void appendToCurrent(char *str) 
   {
     if (!currentNode) 
        {
          nextNode(new DxText);
        }
     currentNode->body.append(str);
   }

#define YY_DECL static int doxygenLex(void)

YY_DECL;

%}

%%

"/*!"|"//!" {
   BEGIN(MAIN);
}

. {
   puts("Internal error: unknown character outside comment");
   yyterminate();
}

<MAIN>{CMD}"brief"{B}+/{NB} {
   nextNode(new DxBrief);
}

<MAIN>{CMD}("sa"|"see"){B}+/{NB} {
   nextNode(new DxSeeAlso);
}

<MAIN>{CMD}"fn"{B}+ {
   nextNode(new DxFunctionDeclaration);
   BEGIN(PROTO);
}

<MAIN>{CMD}"var"{B}+ {
   nextNode(new DxVariableDeclaration);
   BEGIN(PROTO);
}

<MAIN>{CMD}"class"{B}+ {
   nextNode(new DxClassDeclaration);
   BEGIN(PROTO);
}

<PROTO>"(" {
/* Perhaps in the future we can extend the following to extract the
 * function name automatically */
   appendToCurrent(yytext);
   braceCount++;
}

<PROTO>")" {
   appendToCurrent(yytext);
   braceCount--;
}

<PROTO>\n {
   if (braceCount == 0)
      {
        BEGIN(MAIN);
        nextNode(new DxText);
      }
   appendToCurrent(yytext);
}

<MAIN>{CMD}"param"{B}+{NB}+{B}+ {
   char *paramStart = yytext+6;
   while (*paramStart == ' ' || *paramStart == '\t') paramStart++;
   char *paramEnd = paramStart;
   while (*paramEnd != ' ' && *paramEnd != '\t') paramEnd++;
   nextNode(new DxParameter(string(paramStart, paramEnd-paramStart)));
}

<MAIN>{CMD}"return"{B}+/{NB} {
   nextNode(new DxReturn);
}

<MAIN>{CMD}"deprecated"{B}+/{NB} {
   nextNode(new DxDeprecated);
}

<MAIN>{CMD}"name"{B}+ {
   nextNode(new DxName);
   BEGIN(NAME);
}

<NAME>\n {
   nextNode(new DxText);
   appendToCurrent(yytext);
   BEGIN(MAIN);
}

<MAIN>\n({B}*\n)+ {
   if (dynamic_cast<DxBrief *>(currentNode))
      {
	nextNode(new DxText);
      }
   appendToCurrent(yytext);
}

<MAIN,PROTO,NAME>. {
   appendToCurrent(yytext);
}

<MAIN>\n {
   appendToCurrent(yytext);
}

<MAIN,PROTO,NAME>"*/" {
   BEGIN(INITIAL);
}

<MAIN><<EOF>> {
   // commented out to allow C++ style comments
   // puts("Internal error: unterminated comment");
   yyterminate();
}

<INITIAL><<EOF>> {
   yyterminate();
}

%%

DxNodeList *parseDoxygenComment(string str) 
   {
     currentList = new DxNodeList;
     currentNode = 0;
     braceCount = 0;
     yy_scan_string(str.c_str());
     BEGIN(INITIAL);
     doxygenLex();
     return currentList;
   }


