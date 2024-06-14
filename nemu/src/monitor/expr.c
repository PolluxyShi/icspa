#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>
#include <string.h>

#define condition (i == 0 || tokens[i-1].type == '+' || tokens[i-1].type == '-' || tokens[i-1].type == '*' || tokens[i-1].type == '/'  || tokens[i-1].type == '(')

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,
	EQ,
	NUM,
	REG,
	SYMB,
    MINUS,
    DEREF
	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"==", EQ},
	{"([0-9]{1,10})|(0[xX][0-9a-fA-F]{1,10})", NUM},
	{"\\$((e?(([a-d]x)|([bs]p)|([ds]i)))|([a-d][lh]))", REG},
	{"[a-zA-z][0-9a-zA-Z_]*", SYMB},
	{"-", '-'},
	{"\\+", '+'},
	{"\\*", '*'},
	{"\\/", '/'},
	{"\\(", '('},
	{"\\)", ')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type)
				{
				case NOTYPE:break;
				case NUM: case REG: case SYMB:
				    strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len); 
				    tokens[nr_token].str[substr_len] = '\0';
				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

bool check_parentheses(int p, int q){return tokens[p].type == '(' && tokens[q].type == ')';}
int opcmp(int op1, int op2){
    if(op1 == DEREF){
        if(op2 == DEREF) return 0;
        return 1;
    }
    if(op2 == DEREF){
        if(op1 == DEREF) return 0;
        return -1;
    }
    if(op1 == MINUS){
        if(op2 == MINUS) return 0;
        return 1;
    }
    if(op2 == MINUS){
        if(op1 == MINUS) return 0;
        return -1;
    }
    if((op1 == '+' || op1 == '-') && (op2 == '+' || op2 == '-')) return 0;
    if((op1 == '*' || op1 == '/') && (op2 == '*' || op2 == '/')) return 0;
    if((op1 == '+' || op1 == '-') && (op2 == '*' || op2 == '/')) return -1;
    if((op1 == '*' || op1 == '/') && (op2 == '+' || op2 == '-')) return 1;
    return 0;
}

uint32_t look_up_symtab(char *sym, bool *success); // 此处为声明，定义在elf.c中

uint32_t eval(int p, int q, bool *success) {
    if(p > q) {
        /* Bad expression */
        *success = false;
        return 0;
    }
    else if(p == q) { 
        /* Single token.
         * For now this token should be a number. 
         * Return the value of the number.
         */ 
        switch(tokens[p].type)
        {
        case NUM:{
            if(tokens[p].str[1] == 'x' || tokens[p].str[1] == 'X') return strtol(tokens[p].str+2,NULL,16);
            return atoi(tokens[p].str);
        }
        case REG:{
            int ret = get_reg_val(tokens[p].str+1,success);
            if(*success) return ret;
            return 0;
        }
        case SYMB:{
            uint32_t ret = look_up_symtab(tokens[p].str,success);
            if(*success) return ret;
            return 0;
        }
        default:
            *success = false;
            return 0;
        }
    }
    else if(check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses. 
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1, success); 
    }
    else {
        // op = the position of dominant operator in the token expression;
        int op = p;
        int parentheses = 0;
        for(int i = p; i <= q; ++i){
            switch(tokens[i].type)
            {
            case '+': case '-': case '*': case '/':
                if(parentheses > 0) continue;
                else if(tokens[op].type != '+' && tokens[op].type != '-' && tokens[op].type != '*' && tokens[op].type != '/') op = i;
                else if(opcmp(tokens[i].type,tokens[op].type) <= 0) op = i;
                continue;
            case '(': parentheses++; continue;
            case ')': parentheses--; continue;
            default: continue;
            }
        }

        if(tokens[p].type == DEREF){
            uint32_t ret = eval(p+1,q,success);
            if(*success) return paddr_read(ret,4);
            return 0;
        }
        else if(tokens[p].type == MINUS){
            uint32_t ret = eval(p+1,q,success);
            if(*success) return -ret;
            return 0;
        }

        uint32_t val1 = eval(p, op - 1, success);
        uint32_t val2 = eval(op + 1, q, success);
        if(*success){
            switch(tokens[op].type) {
                case '+': return val1 + val2;
                case '-': return val1 - val2;
                case '*': return val1 * val2;
                case '/': return val1 / val2;
                default: *success = false; return 0;
            }
        }
    }
    return 0;
}


uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}

    for(int i = 0; i < nr_token; ++i){
        switch(tokens[i].type)
        {
        case '-':
            if(condition) tokens[i].type = MINUS;
            break;
        case '*':
            if(condition) tokens[i].type = DEREF;
            break;
        default: break;
        }
    }
    
    /* TODO: Implement code to evaluate the expression. */
    for(int i = 0; i < nr_token; i++) {
        if(tokens[i].type == '-' && condition) tokens[i].type = MINUS;
        if(tokens[i].type == '*' && condition) tokens[i].type = DEREF;
    }
    
    uint32_t ret = eval(0, nr_token-1, success);
    
    if(*success) return ret;
    
    return 0;
}
