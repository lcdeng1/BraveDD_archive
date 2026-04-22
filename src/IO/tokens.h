#ifndef BRAVE_DD_TOKENS_H
#define BRAVE_DD_TOKENS_H

#include"../defines.h"

#define NUM_KEYWORDS 17

namespace BRAVE_DD{

    enum class TokenType {
        END     = 0,
        COMMA   = ',',
        DOT     = '.',
        SEMI    = ';',
        LPAR    = '(',
        RPAR    = ')',
        LBRAK   = '[',
        RBRAK   = ']',
        LBRACE  = '{',
        RBRACE  = '}',
        GT      = '>',
        LT      = '<',
        ASSIGN  = '=',
        PLUS    = '+',
        MINUS   = '-',
        STAR    = '*',
        SLASH   = '/',
        MOD     = '%',
        COLON   = ':',
        QUEST   = '?',
        TILDE   = '~',
        PIPE    = '|',
        AMP     = '&',
        BANG    = '!',
        POUND   = '#',

    /* Stuff with attributes */

        RULE      = 301,
        CHAR_LIT  = 302,
        INT_LIT   = 303,
        REAL_LIT  = 304,
        STR_LIT   = 305,
        IDENT     = 306,
        NONTERMI  = 307,
        TERMI     = 308,
        ROOTEDGE  = 309,

    /* Symbols */

    /* Keywords */
        FOREST    = 401,
        TYPE      = 402,
        REDUCED   = 403,
        LVLS      = 404,
        DIM       = 405,
        RANGE     = 406,
        NNUM      = 407,
        RNUM      = 408,

        NODES     = 409,
        LVL       = 410,

        ROOTS     = 411,

        BOOL      = 412
    };

    /*
    * Saved tokens
    */
    // struct token {
    //     const char* filename;   /* File containing token */
    //     unsigned line_number;   /* Line number containing token */
    //     TokenType token;        /* Token ID */
    //     char* lexeme;           /* lexeme if needed; otherwise null */
    // };
} // end of namespace

#endif