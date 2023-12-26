/*
 * IR - Lightweight JIT Compilation Framework
 * (IR loader)
 * Copyright (C) 2022 Zend by Perforce.
 * Authors: Dmitry Stogov <dmitry@php.net>
 *
 * This file is generated from "ir.g". Do not edit!
 *
 * To generate ir_load.c use llk <https://github.com/dstogov/llk>:
 * php llk.php ir.g
 */

#include "ir.h"
#include "ir_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef _WIN32
# include <unistd.h>
#endif

const unsigned char *yy_buf;
const unsigned char *yy_end;
const unsigned char *yy_pos;
const unsigned char *yy_text;
uint32_t yy_line;

typedef struct _ir_parser_ctx {
	ir_ctx    *ctx;
	uint32_t   undef_count;
	ir_strtab  var_tab;
} ir_parser_ctx;

static ir_strtab type_tab;
static ir_strtab op_tab;

#define IR_IS_UNRESOLVED(ref) \
	((ref) < (ir_ref)0xc0000000)
#define IR_ENCODE_UNRESOLVED_REF(ref, op) \
	((ir_ref)0xc0000000 - ((ref) * sizeof(ir_ref) + (op)))
#define IR_DECODE_UNRESOLVED_REF(ref) \
	((ir_ref)0xc0000000 - (ref))

static ir_ref ir_use_var(ir_parser_ctx *p, uint32_t n, const char *str, size_t len) {
	ir_ref ref;
	uint32_t len32;

	IR_ASSERT(len <= 0xffffffff);
	len32 = (uint32_t)len;
	ref = ir_strtab_find(&p->var_tab, str, len32);
	if (!ref) {
		p->undef_count++;
		/* create a linked list of unresolved references with header in "var_tab" */
		ref = IR_UNUSED; /* list terminator */
		ir_strtab_lookup(&p->var_tab, str, len32, IR_ENCODE_UNRESOLVED_REF(p->ctx->insns_count, n));
	} else if (IR_IS_UNRESOLVED(ref)) {
		/* keep the linked list of unresolved references with header in "var_tab" */
		/* "ref" keeps the tail of the list */
		ir_strtab_update(&p->var_tab, str, len32, IR_ENCODE_UNRESOLVED_REF(p->ctx->insns_count, n));
	}
	return ref;
}

static void ir_define_var(ir_parser_ctx *p, const char *str, size_t len, ir_ref ref) {
	ir_ref old_ref;
	uint32_t len32;

	IR_ASSERT(len <= 0xffffffff);
	len32 = (uint32_t)len;
	old_ref = ir_strtab_lookup(&p->var_tab, str, len32, ref);
	if (ref != old_ref) {
		if (IR_IS_UNRESOLVED(old_ref)) {
			p->undef_count--;
			/* update the linked list of unresolved references */
			do {
				ir_ref *ptr = ((ir_ref*)(p->ctx->ir_base)) + IR_DECODE_UNRESOLVED_REF(old_ref);
				old_ref = *ptr;
				*ptr = ref;
			} while (old_ref != IR_UNUSED);
			ir_strtab_update(&p->var_tab, str, len32, ref);
		} else {
			fprintf(stderr, "ERROR: Redefined variable `%.*s` on line %d\n", (int)len32, str, yy_line);
			exit(2);
		}
	}
}

static void report_undefined_var(const char *str, uint32_t len, ir_ref val)
{
	if (IR_IS_UNRESOLVED(val)) {
		fprintf(stderr, "ERROR: Undefined variable `%.*s`\n", (int)len, str);
	}
}

static void ir_check_indefined_vars(ir_parser_ctx *p)
{
	ir_strtab_apply(&p->var_tab, report_undefined_var);
	exit(2);
}

/* forward declarations */
static void yy_error(const char *msg);
static void yy_error_sym(const char *msg, int sym);
static void yy_error_str(const char *msg, const char *str);

#define YYPOS cpos
#define YYEND cend

#define YY_EOF 0
#define YY_EXTERN 1
#define YY__SEMICOLON 2
#define YY_STATIC 3
#define YY__EQUAL 4
#define YY__LBRACE 5
#define YY__COMMA 6
#define YY__RBRACE 7
#define YY_VAR 8
#define YY_CONST 9
#define YY__LBRACK 10
#define YY__RBRACK 11
#define YY_SYM 12
#define YY__LPAREN 13
#define YY__RPAREN 14
#define YY__PLUS 15
#define YY_FUNC 16
#define YY_VOID 17
#define YY__POINT_POINT_POINT 18
#define YY__COLON 19
#define YY___FASTCALL 20
#define YY___BUILTIN 21
#define YY__STAR 22
#define YY__SLASH 23
#define YY_NULL 24
#define YY_INF 25
#define YY_NAN 26
#define YY__MINUS 27
#define YY_ID 28
#define YY_DECNUMBER 29
#define YY_HEXNUMBER 30
#define YY_FLOATNUMBER 31
#define YY_CHARACTER 32
#define YY_STRING 33
#define YY_EOL 34
#define YY_WS 35
#define YY_ONE_LINE_COMMENT 36
#define YY_COMMENT 37

static const char * sym_name[] = {
	"<EOF>",
	"extern",
	";",
	"static",
	"=",
	"{",
	",",
	"}",
	"var",
	"const",
	"[",
	"]",
	"sym",
	"(",
	")",
	"+",
	"func",
	"void",
	"...",
	":",
	"__fastcall",
	"__builtin",
	"*",
	"/",
	"null",
	"inf",
	"nan",
	"-",
	"<ID>",
	"<DECNUMBER>",
	"<HEXNUMBER>",
	"<FLOATNUMBER>",
	"<CHARACTER>",
	"<STRING>",
	"<EOL>",
	"<WS>",
	"<ONE_LINE_COMMENT>",
	"<COMMENT>",
	NULL
};

#define YY_IN_SET(sym, set, bitset) \
	(bitset[sym>>3] & (1 << (sym & 0x7)))

size_t yy_escape(char *buf, unsigned char ch)
{
	switch (ch) {
		case '\\': buf[0] = '\\'; buf[1] = '\\'; return 2;
		case '\'': buf[0] = '\\'; buf[1] = '\''; return 2;
		case '\"': buf[0] = '\\'; buf[1] = '\"'; return 2;
		case '\a': buf[0] = '\\'; buf[1] = '\a'; return 2;
		case '\b': buf[0] = '\\'; buf[1] = '\b'; return 2;
		case '\e': buf[0] = '\\'; buf[1] = '\e'; return 2;
		case '\f': buf[0] = '\\'; buf[1] = '\f'; return 2;
		case '\n': buf[0] = '\\'; buf[1] = '\n'; return 2;
		case '\r': buf[0] = '\\'; buf[1] = '\r'; return 2;
		case '\t': buf[0] = '\\'; buf[1] = '\t'; return 2;
		case '\v': buf[0] = '\\'; buf[1] = '\v'; return 2;
		case '\?': buf[0] = '\\'; buf[1] = 0x3f; return 2;
		default: break;
	}
	if (ch < 32 || ch >= 127) {
		buf[0] = '\\';
		buf[1] = '0' + ((ch >> 3) % 8);
		buf[2] = '0' + ((ch >> 6) % 8);
		buf[3] = '0' + (ch % 8);
		return 4;
	} else {
		buf[0] = ch;
		return 1;
	}
}

const char *yy_escape_char(char *buf, unsigned char ch)
{
	size_t len = yy_escape(buf, ch);
	buf[len] = 0;
	return buf;
}

const char *yy_escape_string(char *buf, size_t size, const unsigned char *str, size_t n)
{
	size_t i = 0;
	size_t pos = 0;
	size_t len;

	while (i < n) {
		if (pos + 8 > size) {
			buf[pos++] = '.';
			buf[pos++] = '.';
			buf[pos++] = '.';
			break;
		}
		len = yy_escape(buf + pos, str[i]);
		i++;
		pos += len;
	}
	buf[pos] = 0;
	return buf;
}

static int skip_EOL(int sym);
static int skip_WS(int sym);
static int skip_ONE_LINE_COMMENT(int sym);
static int skip_COMMENT(int sym);
static int get_sym(void);
static int parse_ir(int sym, ir_loader *loader);
static int parse_ir_sym(int sym, char *buf, uint32_t *flags);
static int parse_ir_sym_size(int sym, size_t *size);
static int parse_ir_sym_data(int sym, ir_loader *loader);
static int parse_ir_func(int sym, ir_parser_ctx *p);
static int parse_ir_func_name(int sym, char *buf);
static int parse_ir_func_proto(int sym, ir_parser_ctx *p, uint32_t *flags, uint8_t *ret_type, uint32_t *params_count, uint8_t *param_types);
static int parse_ir_insn(int sym, ir_parser_ctx *p);
static int parse_type(int sym, uint8_t *t);
static int parse_func(int sym, uint8_t *op);
static int parse_val(int sym, ir_parser_ctx *p, uint8_t op, uint32_t n, ir_ref *ref);
static int parse_const(int sym, uint8_t t, ir_val *val);
static int parse_ID(int sym, const char **str, size_t *len);
static int parse_DECNUMBER(int sym, uint32_t t, ir_val *val);
static int parse_HEXNUMBER(int sym, uint32_t t, ir_val *val);
static int parse_FLOATNUMBER(int sym, uint32_t t, ir_val *val);
static int parse_CHARACTER(int sym, ir_val *val);
static int parse_STRING(int sym, const char **str, size_t *len);

static int get_skip_sym(void) {
	char buf[64];
	int ch;
	int ret;
	int accept = -1;
	const unsigned char *accept_pos;
	const unsigned char *cpos = yy_pos;
	const unsigned char *cend = yy_end;

_yy_state_start:
	yy_text = YYPOS;
	ch = *YYPOS;
	switch (ch) {
		case 'e':
			ch = *++YYPOS;
			if (ch != 'x') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 't') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'e') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'r') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'n') goto _yy_tunnel_4;
			ret = YY_EXTERN;
			goto _yy_state_117;
		case 'v':
			ch = *++YYPOS;
			if (ch == 'a') {
				ch = *++YYPOS;
				if (ch != 'r') goto _yy_tunnel_4;
				ret = YY_VAR;
				goto _yy_state_117;
			} else if (ch == 'o') {
				ch = *++YYPOS;
				if (ch != 'i') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'd') goto _yy_tunnel_4;
				ret = YY_VOID;
				goto _yy_state_117;
			} else {
				goto _yy_tunnel_4;
			}
		case 'c':
			ch = *++YYPOS;
			if (ch != 'o') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'n') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 's') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 't') goto _yy_tunnel_4;
			ret = YY_CONST;
			goto _yy_state_117;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case 'a':
		case 'b':
		case 'd':
		case 'g':
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 't':
		case 'u':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
			goto _yy_state_4;
		case 'f':
			ch = *++YYPOS;
			if (ch != 'u') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'n') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'c') goto _yy_tunnel_4;
			ret = YY_FUNC;
			goto _yy_state_117;
		case 'i':
			ch = *++YYPOS;
			if (ch != 'n') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch != 'f') goto _yy_tunnel_4;
			ret = YY_INF;
			goto _yy_state_117;
		case 'n':
			ch = *++YYPOS;
			if (ch == 'a') {
				ch = *++YYPOS;
				if (ch != 'n') goto _yy_tunnel_4;
				ret = YY_NAN;
				goto _yy_state_117;
			} else if (ch == 'u') {
				ch = *++YYPOS;
				if (ch != 'l') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'l') goto _yy_tunnel_4;
				ret = YY_NULL;
				goto _yy_state_117;
			} else {
				goto _yy_tunnel_4;
			}
		case 's':
			ch = *++YYPOS;
			if (ch == 't') {
				ch = *++YYPOS;
				if (ch != 'a') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 't') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'i') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'c') goto _yy_tunnel_4;
				ret = YY_STATIC;
				goto _yy_state_117;
			} else if (ch == 'y') {
				ch = *++YYPOS;
				if (ch != 'm') goto _yy_tunnel_4;
				ret = YY_SYM;
				goto _yy_state_117;
			} else {
				goto _yy_tunnel_4;
			}
		case '_':
			ch = *++YYPOS;
			if (ch != '_') goto _yy_tunnel_4;
			ch = *++YYPOS;
			if (ch == 'b') {
				ch = *++YYPOS;
				if (ch != 'u') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'i') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'l') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 't') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'i') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'n') goto _yy_tunnel_4;
				ret = YY___BUILTIN;
				goto _yy_state_117;
			} else if (ch == 'f') {
				ch = *++YYPOS;
				if (ch != 'a') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 's') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 't') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'c') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'a') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'l') goto _yy_tunnel_4;
				ch = *++YYPOS;
				if (ch != 'l') goto _yy_tunnel_4;
				ret = YY___FASTCALL;
				goto _yy_state_117;
			} else {
				goto _yy_tunnel_4;
			}
		case ';':
			YYPOS++;
			ret = YY__SEMICOLON;
			goto _yy_fin;
		case '(':
			YYPOS++;
			ret = YY__LPAREN;
			goto _yy_fin;
		case '.':
			ch = *++YYPOS;
			if (ch == '.') {
				ch = *++YYPOS;
				if (ch == '.') {
					YYPOS++;
					ret = YY__POINT_POINT_POINT;
					goto _yy_fin;
				} else {
					goto _yy_state_error;
				}
			} else if ((ch >= '0' && ch <= '9')) {
				goto _yy_state_47;
			} else {
				goto _yy_state_error;
			}
		case ',':
			YYPOS++;
			ret = YY__COMMA;
			goto _yy_fin;
		case ')':
			YYPOS++;
			ret = YY__RPAREN;
			goto _yy_fin;
		case ':':
			YYPOS++;
			ret = YY__COLON;
			goto _yy_fin;
		case '[':
			YYPOS++;
			ret = YY__LBRACK;
			goto _yy_fin;
		case '-':
			ch = *++YYPOS;
			accept = YY__MINUS;
			accept_pos = yy_pos;
			if ((ch >= '0' && ch <= '9')) {
				goto _yy_state_19;
			} else if (ch == '.') {
				ch = *++YYPOS;
				if ((ch >= '0' && ch <= '9')) {
					goto _yy_state_47;
				} else {
					goto _yy_state_error;
				}
			} else {
				ret = YY__MINUS;
				goto _yy_fin;
			}
		case '0':
			ch = *++YYPOS;
			if (ch != 'x') goto _yy_tunnel_19;
			ch = *++YYPOS;
			if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
				goto _yy_state_78;
			} else {
				goto _yy_state_error;
			}
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			goto _yy_state_19;
		case ']':
			YYPOS++;
			ret = YY__RBRACK;
			goto _yy_fin;
		case '=':
			YYPOS++;
			ret = YY__EQUAL;
			goto _yy_fin;
		case '{':
			YYPOS++;
			ret = YY__LBRACE;
			goto _yy_fin;
		case '+':
			YYPOS++;
			ret = YY__PLUS;
			goto _yy_fin;
		case '\'':
			goto _yy_state_24;
		case '}':
			YYPOS++;
			ret = YY__RBRACE;
			goto _yy_fin;
		case '*':
			YYPOS++;
			ret = YY__STAR;
			goto _yy_fin;
		case '"':
			goto _yy_state_27;
		case '/':
			ch = *++YYPOS;
			accept = YY__SLASH;
			accept_pos = yy_pos;
			if (ch == '*') {
				goto _yy_state_59;
			} else if (ch == '/') {
				goto _yy_state_32;
			} else {
				ret = YY__SLASH;
				goto _yy_fin;
			}
		case '\r':
			ch = *++YYPOS;
			if (ch == '\n') {
				yy_line++;
				YYPOS++;
				ret = YY_EOL;
				goto _yy_fin;
			} else {
				ret = YY_EOL;
				goto _yy_fin;
			}
		case '\n':
			yy_line++;
			YYPOS++;
			ret = YY_EOL;
			goto _yy_fin;
		case ' ':
		case '\t':
		case '\f':
		case '\v':
			goto _yy_state_31;
		case '#':
			goto _yy_state_32;
		case '\0':
			if (ch == 0 && YYPOS < YYEND) goto _yy_state_error;
			YYPOS++;
			ret = YY_EOF;
			goto _yy_fin;
		default:
			goto _yy_state_error;
	}
_yy_state_4:
	ch = *++YYPOS;
_yy_tunnel_4:
	if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= 'a' && ch <= 'z')) {
		goto _yy_state_4;
	} else {
		ret = YY_ID;
		goto _yy_fin;
	}
_yy_state_19:
	ch = *++YYPOS;
_yy_tunnel_19:
	accept = YY_DECNUMBER;
	accept_pos = yy_pos;
	if ((ch >= '0' && ch <= '9')) {
		goto _yy_state_19;
	} else if (ch == 'E' || ch == 'e') {
		goto _yy_state_51;
	} else if (ch == '.') {
		goto _yy_state_47;
	} else {
		ret = YY_DECNUMBER;
		goto _yy_fin;
	}
_yy_state_24:
	ch = *++YYPOS;
	if (ch == '\\') {
		ch = *++YYPOS;
		if (YYPOS < YYEND) {
			if (ch == '\n') {
				yy_line++;
			}
			goto _yy_state_24;
		} else {
			goto _yy_state_error;
		}
	} else if (ch == '\'') {
		YYPOS++;
		ret = YY_CHARACTER;
		goto _yy_fin;
	} else if (YYPOS < YYEND && (ch <= '&' || (ch >= '(' && ch <= '[') || ch >= ']')) {
		if (ch == '\n') {
			yy_line++;
		}
		goto _yy_state_24;
	} else {
		goto _yy_state_error;
	}
_yy_state_27:
	ch = *++YYPOS;
	if (ch == '\\') {
		ch = *++YYPOS;
		if (YYPOS < YYEND) {
			if (ch == '\n') {
				yy_line++;
			}
			goto _yy_state_27;
		} else {
			goto _yy_state_error;
		}
	} else if (ch == '"') {
		YYPOS++;
		ret = YY_STRING;
		goto _yy_fin;
	} else if (YYPOS < YYEND && (ch <= '!' || (ch >= '#' && ch <= '[') || ch >= ']')) {
		if (ch == '\n') {
			yy_line++;
		}
		goto _yy_state_27;
	} else {
		goto _yy_state_error;
	}
_yy_state_31:
	ch = *++YYPOS;
	if (ch == '\t' || ch == '\v' || ch == '\f' || ch == ' ') {
		goto _yy_state_31;
	} else {
		ret = YY_WS;
		goto _yy_fin;
	}
_yy_state_32:
	ch = *++YYPOS;
	if (ch == '\r') {
		ch = *++YYPOS;
		if (ch == '\n') {
			yy_line++;
			YYPOS++;
			ret = YY_ONE_LINE_COMMENT;
			goto _yy_fin;
		} else {
			ret = YY_ONE_LINE_COMMENT;
			goto _yy_fin;
		}
	} else if (ch == '\n') {
		yy_line++;
		YYPOS++;
		ret = YY_ONE_LINE_COMMENT;
		goto _yy_fin;
	} else if (YYPOS < YYEND && (ch <= '\t' || ch == '\v' || ch == '\f' || ch >= '\016')) {
		goto _yy_state_32;
	} else {
		goto _yy_state_error;
	}
_yy_state_47:
	ch = *++YYPOS;
	accept = YY_FLOATNUMBER;
	accept_pos = yy_pos;
	if ((ch >= '0' && ch <= '9')) {
		goto _yy_state_47;
	} else if (ch == 'E' || ch == 'e') {
		goto _yy_state_51;
	} else {
		ret = YY_FLOATNUMBER;
		goto _yy_fin;
	}
_yy_state_51:
	ch = *++YYPOS;
	if (ch == '+' || ch == '-') {
		ch = *++YYPOS;
		if ((ch >= '0' && ch <= '9')) {
			goto _yy_state_81;
		} else {
			goto _yy_state_error;
		}
	} else if ((ch >= '0' && ch <= '9')) {
		goto _yy_state_81;
	} else {
		goto _yy_state_error;
	}
_yy_state_59:
	ch = *++YYPOS;
_yy_tunnel_59:
	if (ch == '*') {
		ch = *++YYPOS;
		if (ch != '/') goto _yy_tunnel_59;
		YYPOS++;
		ret = YY_COMMENT;
		goto _yy_fin;
	} else if (YYPOS < YYEND && (ch <= ')' || ch >= '+')) {
		if (ch == '\n') {
			yy_line++;
		}
		goto _yy_state_59;
	} else {
		goto _yy_state_error;
	}
_yy_state_78:
	ch = *++YYPOS;
	if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
		goto _yy_state_78;
	} else {
		ret = YY_HEXNUMBER;
		goto _yy_fin;
	}
_yy_state_81:
	ch = *++YYPOS;
	if ((ch >= '0' && ch <= '9')) {
		goto _yy_state_81;
	} else {
		ret = YY_FLOATNUMBER;
		goto _yy_fin;
	}
_yy_state_117:
	ch = *++YYPOS;
	if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= 'a' && ch <= 'z')) {
		goto _yy_state_4;
	} else {
		goto _yy_fin;
	}
_yy_state_error:
	if (accept >= 0) {
		yy_pos = accept_pos;
		return accept;
	}
	if (YYPOS >= YYEND) {
		yy_error("unexpected <EOF>");
	} else if (YYPOS == yy_text) {
		yy_error_str("unexpected character",  yy_escape_char(buf, ch));
	} else {
		yy_error_str("unexpected sequence", yy_escape_string(buf, sizeof(buf), yy_text, 1 + YYPOS - yy_text));
	}
	YYPOS++;
	goto _yy_state_start;
_yy_fin:
	yy_pos = YYPOS;
	return ret;
}

static int skip_EOL(int sym) {
	if (sym != YY_EOL) {
		yy_error_sym("<EOL> expected, got", sym);
	}
	sym = get_skip_sym();
	return sym;
}

static int skip_WS(int sym) {
	if (sym != YY_WS) {
		yy_error_sym("<WS> expected, got", sym);
	}
	sym = get_skip_sym();
	return sym;
}

static int skip_ONE_LINE_COMMENT(int sym) {
	if (sym != YY_ONE_LINE_COMMENT) {
		yy_error_sym("<ONE_LINE_COMMENT> expected, got", sym);
	}
	sym = get_skip_sym();
	return sym;
}

static int skip_COMMENT(int sym) {
	if (sym != YY_COMMENT) {
		yy_error_sym("<COMMENT> expected, got", sym);
	}
	sym = get_skip_sym();
	return sym;
}

static int get_sym(void) {
	int sym;
	sym = get_skip_sym();
	while (sym == YY_EOL || sym == YY_WS || sym == YY_ONE_LINE_COMMENT || sym == YY_COMMENT) {
		if (sym == YY_EOL) {
			sym = skip_EOL(sym);
		} else if (sym == YY_WS) {
			sym = skip_WS(sym);
		} else if (sym == YY_ONE_LINE_COMMENT) {
			sym = skip_ONE_LINE_COMMENT(sym);
		} else {
			sym = skip_COMMENT(sym);
		}
	}
	return sym;
}

static int parse_ir(int sym, ir_loader *loader) {
	int   sym2;
	const unsigned char *save_pos;
	const unsigned char *save_text;
	int   save_line;
	int alt13;
	ir_parser_ctx p;
	ir_ctx ctx;
	uint8_t ret_type;
	char name[256];
	uint32_t flags = 0;
	size_t size;
	uint32_t params_count;
	uint8_t param_types[256];
	p.ctx = &ctx;
	if (YY_IN_SET(sym, (YY_EXTERN,YY_STATIC,YY_VAR,YY_CONST,YY_FUNC), "\012\003\001\000\000")) {
		do {
			if (sym == YY_EXTERN) {
				sym = get_sym();
				flags = 0;
				if (sym == YY_VAR || sym == YY_CONST) {
					sym = parse_ir_sym(sym, name, &flags);
					if (sym != YY__SEMICOLON) {
						yy_error_sym("';' expected, got", sym);
					}
					sym = get_sym();
					if (loader->external_sym_dcl
					 && !loader->external_sym_dcl(loader, name, flags)) {
						yy_error("extenral_sym_dcl error");
					}
				} else if (sym == YY_FUNC) {
					flags = 0;
					sym = parse_ir_func_name(sym, name);
					sym = parse_ir_func_proto(sym, &p, &flags, &ret_type, &params_count, param_types);
					if (sym != YY__SEMICOLON) {
						yy_error_sym("';' expected, got", sym);
					}
					sym = get_sym();
					if (loader->external_func_dcl
					 && !loader->external_func_dcl(loader, name, flags, ret_type, params_count, param_types)) {
						yy_error("extenral_func_dcl error");
					}
				} else {
					yy_error_sym("unexpected", sym);
				}
			} else if (sym == YY_STATIC || sym == YY_VAR || sym == YY_CONST || sym == YY_FUNC) {
				flags = 0;
				if (sym == YY_STATIC) {
					sym = get_sym();
					flags |= IR_STATIC;
				}
				if (sym == YY_VAR || sym == YY_CONST) {
					sym = parse_ir_sym(sym, name, &flags);
					sym = parse_ir_sym_size(sym, &size);
					if (sym == YY__SEMICOLON) {
						if (loader->sym_dcl && !loader->sym_dcl(loader, name, flags, size, 0)) {
							yy_error("sym_dcl error");
						}
						sym = get_sym();
					} else if (sym == YY__EQUAL) {
						if (loader->sym_dcl && !loader->sym_dcl(loader, name, flags, size, 1)) {
							yy_error("sym_dcl error");
						}
						sym = get_sym();
						if (sym != YY__LBRACE) {
							yy_error_sym("'{' expected, got", sym);
						}
						sym = get_sym();
						sym = parse_ir_sym_data(sym, loader);
						while (1) {
							save_pos  = yy_pos;
							save_text = yy_text;
							save_line = yy_line;
							alt13 = -2;
							sym2 = sym;
							if (sym2 == YY__COMMA) {
								sym2 = get_sym();
								goto _yy_state_13_1;
							} else if (sym2 == YY__RBRACE) {
								alt13 = 17;
								goto _yy_state_13;
							} else {
								yy_error_sym("unexpected", sym2);
							}
_yy_state_13_1:
							if (sym2 == YY_ID) {
								alt13 = 14;
								goto _yy_state_13;
							} else if (sym2 == YY__RBRACE) {
								alt13 = 16;
								goto _yy_state_13;
							} else {
								yy_error_sym("unexpected", sym2);
							}
_yy_state_13:
							yy_pos  = save_pos;
							yy_text = save_text;
							yy_line = save_line;
							if (alt13 != 14) {
								break;
							}
							sym = get_sym();
							sym = parse_ir_sym_data(sym, loader);
						}
						if (alt13 == 16) {
							sym = get_sym();
						}
						if (sym != YY__RBRACE) {
							yy_error_sym("'}' expected, got", sym);
						}
						sym = get_sym();
						if (sym != YY__SEMICOLON) {
							yy_error_sym("';' expected, got", sym);
						}
						sym = get_sym();
						if (loader->sym_data_end && !loader->sym_data_end(loader)) {
							yy_error("sym_data_end error");
						}
					} else {
						yy_error_sym("unexpected", sym);
					}
				} else if (sym == YY_FUNC) {
					sym = parse_ir_func_name(sym, name);
					sym = parse_ir_func_proto(sym, &p, &flags, &ret_type, &params_count, param_types);
					if (sym == YY__SEMICOLON) {
						sym = get_sym();
						if (loader->forward_func_dcl
						 && !loader->forward_func_dcl(loader, name, flags, ret_type, params_count, param_types)) {
							yy_error("forward_func_decl error");
						}
					} else if (sym == YY__LBRACE) {
						if (!loader->func_init(loader, &ctx, name)) yy_error("init_func error");
						ctx.flags |= flags;
						ctx.ret_type = ret_type;
						sym = parse_ir_func(sym, &p);
						if (!loader->func_process(loader, &ctx, name)) yy_error("process_func error");
						ir_free(&ctx);
					} else {
						yy_error_sym("unexpected", sym);
					}
				} else {
					yy_error_sym("unexpected", sym);
				}
			} else {
				yy_error_sym("unexpected", sym);
			}
		} while (YY_IN_SET(sym, (YY_EXTERN,YY_STATIC,YY_VAR,YY_CONST,YY_FUNC), "\012\003\001\000\000"));
	} else if (sym == YY__LBRACE) {
		if (!loader->func_init(loader, &ctx, NULL)) yy_error("ini_func error");
		ctx.ret_type = -1;
		sym = parse_ir_func(sym, &p);
		if (!loader->func_process(loader, &ctx, NULL)) yy_error("process_func error");
		ir_free(&ctx);
	} else {
		yy_error_sym("unexpected", sym);
	}
	return sym;
}

static int parse_ir_sym(int sym, char *buf, uint32_t *flags) {
	const char *name;
	size_t len;
	if (sym == YY_VAR) {
		sym = get_sym();
	} else if (sym == YY_CONST) {
		sym = get_sym();
		*flags |= IR_CONST;
	} else {
		yy_error_sym("unexpected", sym);
	}
	sym = parse_ID(sym, &name, &len);
	if (len > 255) yy_error("name too long");
	memcpy(buf, name, len);
	buf[len] = 0;
	return sym;
}

static int parse_ir_sym_size(int sym, size_t *size) {
	ir_val val;
	if (sym != YY__LBRACK) {
		yy_error_sym("'[' expected, got", sym);
	}
	sym = get_sym();
	sym = parse_DECNUMBER(sym, IR_U64, &val);
	if (sym != YY__RBRACK) {
		yy_error_sym("']' expected, got", sym);
	}
	sym = get_sym();
	*size = val.u64;
	return sym;
}

static int parse_ir_sym_data(int sym, ir_loader *loader) {
	uint8_t t = 0;
	ir_val val;
	void *p;
	const char *name;
	size_t name_len;
	char buf[256];
	uintptr_t offset = 0;
	sym = parse_type(sym, &t);
	if (sym == YY_SYM) {
		sym = get_sym();
		if (sym != YY__LPAREN) {
			yy_error_sym("'(' expected, got", sym);
		}
		sym = get_sym();
		sym = parse_ID(sym, &name, &name_len);
		if (sym != YY__RPAREN) {
			yy_error_sym("')' expected, got", sym);
		}
		sym = get_sym();
		if (sym == YY__PLUS) {
			sym = get_sym();
			sym = parse_const(sym, t, &val);
			offset = (uintptr_t)val.addr;
		}
		if (loader->sym_data_ref) {
				if (name_len > 255) yy_error("name too long");
				memcpy(buf, name, name_len);
				buf[name_len] = 0;
				if (!loader->sym_data_ref(loader, IR_SYM, buf, offset)) {
					yy_error("sym_data_ref error");
				}
			}
	} else if (sym == YY_FUNC) {
		sym = get_sym();
		if (sym != YY__LPAREN) {
			yy_error_sym("'(' expected, got", sym);
		}
		sym = get_sym();
		sym = parse_ID(sym, &name, &name_len);
		if (sym != YY__RPAREN) {
			yy_error_sym("')' expected, got", sym);
		}
		sym = get_sym();
		if (loader->sym_data_ref) {
				if (name_len > 255) yy_error("name too long");
				memcpy(buf, name, name_len);
				buf[name_len] = 0;
				if (!loader->sym_data_ref(loader, IR_FUNC, buf, 0)) {
					yy_error("sym_data_ref error");
				}
			}
	} else if (YY_IN_SET(sym, (YY_DECNUMBER,YY_HEXNUMBER,YY_FLOATNUMBER,YY_CHARACTER,YY_INF,YY_NAN,YY__MINUS), "\000\000\000\356\001")) {
		sym = parse_const(sym, t, &val);
		if (loader->sym_data) {
				switch (ir_type_size[t]) {
					case 1: p = &val.i8;  break;
					case 2: p = &val.i16; break;
					case 4: p = &val.i32; break;
					case 8: p = &val.i64; break;
					default:
						IR_ASSERT(0);
						break;
				}
				if (!loader->sym_data(loader, t, 1, p)) {
					yy_error("sym_data error");
				}
			}
	} else {
		yy_error_sym("unexpected", sym);
	}
	return sym;
}

static int parse_ir_func(int sym, ir_parser_ctx *p) {
	p->undef_count = 0;
	ir_strtab_init(&p->var_tab, 256, 4096);
	if (sym != YY__LBRACE) {
		yy_error_sym("'{' expected, got", sym);
	}
	sym = get_sym();
	while (sym == YY_ID) {
		sym = parse_ir_insn(sym, p);
		if (sym != YY__SEMICOLON) {
			yy_error_sym("';' expected, got", sym);
		}
		sym = get_sym();
	}
	if (sym != YY__RBRACE) {
		yy_error_sym("'}' expected, got", sym);
	}
	sym = get_sym();
	if (p->undef_count) ir_check_indefined_vars(p);
	ir_strtab_free(&p->var_tab);
	return sym;
}

static int parse_ir_func_name(int sym, char *buf) {
	const char *name;
	size_t len;
	if (sym != YY_FUNC) {
		yy_error_sym("'func' expected, got", sym);
	}
	sym = get_sym();
	sym = parse_ID(sym, &name, &len);
	if (len > 255) yy_error("name too long");
	memcpy(buf, name, len);
	buf[len] = 0;
	return sym;
}

static int parse_ir_func_proto(int sym, ir_parser_ctx *p, uint32_t *flags, uint8_t *ret_type, uint32_t *params_count, uint8_t *param_types) {
	int   sym2;
	const unsigned char *save_pos;
	const unsigned char *save_text;
	int   save_line;
	int alt65;
	uint8_t t = 0;
	uint32_t n = 0;
	if (sym != YY__LPAREN) {
		yy_error_sym("'(' expected, got", sym);
	}
	sym = get_sym();
	if (sym == YY_VOID || sym == YY__POINT_POINT_POINT || sym == YY_ID) {
		if (sym == YY_VOID) {
			sym = get_sym();
		} else if (sym == YY__POINT_POINT_POINT) {
			sym = get_sym();
			*flags |= IR_VARARG_FUNC;
		} else {
			sym = parse_type(sym, &t);
			param_types[n++] = t;
			while (1) {
				save_pos  = yy_pos;
				save_text = yy_text;
				save_line = yy_line;
				alt65 = -2;
				sym2 = sym;
				if (sym2 == YY__COMMA) {
					sym2 = get_sym();
					goto _yy_state_65_1;
				} else if (sym2 == YY__RPAREN) {
					alt65 = 70;
					goto _yy_state_65;
				} else {
					yy_error_sym("unexpected", sym2);
				}
_yy_state_65_1:
				if (sym2 == YY_ID) {
					alt65 = 66;
					goto _yy_state_65;
				} else if (sym2 == YY__POINT_POINT_POINT) {
					alt65 = 68;
					goto _yy_state_65;
				} else {
					yy_error_sym("unexpected", sym2);
				}
_yy_state_65:
				yy_pos  = save_pos;
				yy_text = save_text;
				yy_line = save_line;
				if (alt65 != 66) {
					break;
				}
				sym = get_sym();
				sym = parse_type(sym, &t);
				param_types[n++] = t;
				if (n > 256) yy_error("name too params");
			}
			if (alt65 == 68) {
				sym = get_sym();
				if (sym != YY__POINT_POINT_POINT) {
					yy_error_sym("'...' expected, got", sym);
				}
				sym = get_sym();
				*flags |= IR_VARARG_FUNC;
			}
		}
	}
	if (sym != YY__RPAREN) {
		yy_error_sym("')' expected, got", sym);
	}
	sym = get_sym();
	if (sym != YY__COLON) {
		yy_error_sym("':' expected, got", sym);
	}
	sym = get_sym();
	if (sym == YY_ID) {
		sym = parse_type(sym, ret_type);
	} else if (sym == YY_VOID) {
		sym = get_sym();
		*ret_type = IR_VOID;
	} else {
		yy_error_sym("unexpected", sym);
	}
	if (sym == YY___FASTCALL || sym == YY___BUILTIN) {
		if (sym == YY___FASTCALL) {
			sym = get_sym();
			*flags |= IR_FASTCALL_FUNC;
		} else {
			sym = get_sym();
			*flags |= IR_BUILTIN_FUNC;
		}
	}
	*params_count = n;
	return sym;
}

static int parse_ir_insn(int sym, ir_parser_ctx *p) {
	int   sym2;
	const unsigned char *save_pos;
	const unsigned char *save_text;
	int   save_line;
	int alt76;
	const char *str, *str2 = NULL, *func;
	size_t len, len2 = 0, func_len;
	uint8_t op;
	uint8_t t = 0;
	ir_ref op1 = IR_UNUSED;
	ir_ref op2 = IR_UNUSED;
	ir_ref op3 = IR_UNUSED;
	ir_ref ref;
	ir_val val;
	ir_val count;
	int32_t n;
	uint8_t ret_type;
	uint32_t flags;
	uint32_t params_count;
	uint8_t param_types[256];
	save_pos  = yy_pos;
	save_text = yy_text;
	save_line = yy_line;
	alt76 = -2;
	sym2 = sym;
	if (sym2 == YY_ID) {
		sym2 = get_sym();
		goto _yy_state_76_1;
	} else {
		yy_error_sym("unexpected", sym2);
	}
_yy_state_76_1:
	if (sym2 == YY_ID) {
		alt76 = 77;
		goto _yy_state_76;
	} else if (sym2 == YY__EQUAL) {
		alt76 = 81;
		goto _yy_state_76;
	} else {
		yy_error_sym("unexpected", sym2);
	}
_yy_state_76:
	yy_pos  = save_pos;
	yy_text = save_text;
	yy_line = save_line;
	if (alt76 == 77) {
		sym = parse_type(sym, &t);
		sym = parse_ID(sym, &str, &len);
		if (sym == YY__COMMA) {
			sym = get_sym();
			sym = parse_ID(sym, &str2, &len2);
		}
	} else if (alt76 == 81) {
		sym = parse_ID(sym, &str, &len);
	} else {
		yy_error_sym("unexpected", sym);
	}
	if (sym != YY__EQUAL) {
		yy_error_sym("'=' expected, got", sym);
	}
	sym = get_sym();
	switch (sym) {
		case YY_DECNUMBER:
		case YY_HEXNUMBER:
		case YY_FLOATNUMBER:
		case YY_CHARACTER:
		case YY_INF:
		case YY_NAN:
		case YY__MINUS:
			val.u64 = 0;
			sym = parse_const(sym, t, &val);
			ref = ir_const(p->ctx, val, t);
			break;
		case YY_FUNC:
			sym = get_sym();
			if (sym == YY_ID) {
				sym = parse_ID(sym, &func, &func_len);
				flags = 0;
				sym = parse_ir_func_proto(sym, p, &flags, &ret_type, &params_count, param_types);
				ref = ir_proto(p->ctx, flags, ret_type, params_count, param_types);
				ref = ir_const_func(p->ctx, ir_strl(p->ctx, func, func_len), ref);
			} else if (sym == YY__STAR) {
				sym = get_sym();
				if (sym == YY_DECNUMBER) {
					sym = parse_DECNUMBER(sym, IR_ADDR, &val);
				} else if (sym == YY_HEXNUMBER) {
					sym = parse_HEXNUMBER(sym, IR_ADDR, &val);
				} else {
					yy_error_sym("unexpected", sym);
				}
				flags = 0;
				sym = parse_ir_func_proto(sym, p, &flags, &ret_type, &params_count, param_types);
				ref = ir_proto(p->ctx, flags, ret_type, params_count, param_types);
				ref = ir_const_func_addr(p->ctx, val.addr, ref);
			} else {
				yy_error_sym("unexpected", sym);
			}
			break;
		case YY_SYM:
			sym = get_sym();
			if (sym != YY__LPAREN) {
				yy_error_sym("'(' expected, got", sym);
			}
			sym = get_sym();
			sym = parse_ID(sym, &func, &func_len);
			if (sym != YY__RPAREN) {
				yy_error_sym("')' expected, got", sym);
			}
			sym = get_sym();
			ref = ir_const_sym(p->ctx, ir_strl(p->ctx, func, func_len));
			break;
		case YY_STRING:
			sym = parse_STRING(sym, &func, &func_len);
			ref = ir_const_str(p->ctx, ir_strl(p->ctx, func, func_len));
			break;
		case YY_ID:
			sym = parse_func(sym, &op);
			if (sym == YY__SLASH) {
				sym = get_sym();
				sym = parse_DECNUMBER(sym, IR_I32, &count);
				if (op == IR_PHI || op == IR_SNAPSHOT) count.i32++;
				if (op == IR_CALL || op == IR_TAILCALL) count.i32+=2;
				if (count.i32 < 0 || count.i32 > 255) yy_error("bad number of operands");
				ref = ir_emit_N(p->ctx, IR_OPT(op, t), count.i32);
				if (sym == YY__LPAREN) {
					sym = get_sym();
					if (YY_IN_SET(sym, (YY_ID,YY_STRING,YY_DECNUMBER,YY_NULL,YY_FUNC), "\000\000\001\061\002")) {
						sym = parse_val(sym, p, op, 1, &op1);
						n = 1;
						if (n > count.i32) yy_error("too many operands");
						ir_set_op(p->ctx, ref, n, op1);
						while (sym == YY__COMMA) {
							sym = get_sym();
							sym = parse_val(sym, p, op, n, &op1);
							n++;
							if (n > count.i32) yy_error("too many operands");
							ir_set_op(p->ctx, ref, n, op1);
						}
					}
					if (sym != YY__RPAREN) {
						yy_error_sym("')' expected, got", sym);
					}
					sym = get_sym();
				}
			} else if (sym == YY__LPAREN || sym == YY__SEMICOLON) {
				n = 0;
				if (sym == YY__LPAREN) {
					sym = get_sym();
					if (YY_IN_SET(sym, (YY_ID,YY_STRING,YY_DECNUMBER,YY_NULL,YY_FUNC), "\000\000\001\061\002")) {
						sym = parse_val(sym, p, op, 1, &op1);
						n = 1;
						if (sym == YY__COMMA) {
							sym = get_sym();
							sym = parse_val(sym, p, op, 2, &op2);
							n = 2;
							if (sym == YY__COMMA) {
								sym = get_sym();
								sym = parse_val(sym, p, op, 3, &op3);
								n = 3;
							}
						}
					}
					if (sym != YY__RPAREN) {
						yy_error_sym("')' expected, got", sym);
					}
					sym = get_sym();
				}
				if (IR_IS_FOLDABLE_OP(op)
				 && !IR_IS_UNRESOLVED(op1)
				 && !IR_IS_UNRESOLVED(op2)
				 && !IR_IS_UNRESOLVED(op3)) {
					ref = ir_fold(p->ctx, IR_OPT(op, t), op1, op2, op3);
				} else {
					uint32_t opt;

					if (!IR_OP_HAS_VAR_INPUTS(ir_op_flags[op])) {
						opt = IR_OPT(op, t);
					} else {
						opt = IR_OPTX(op, t, n);
					}
					ref = ir_emit(p->ctx, opt, op1, op2, op3);
				}
			} else {
				yy_error_sym("unexpected", sym);
			}
			break;
		default:
			yy_error_sym("unexpected", sym);
	}
	ir_define_var(p, str, len, ref);
	if (str2) ir_define_var(p, str2, len2, ref);
	return sym;
}

static int parse_type(int sym, uint8_t *t) {
	const char *str;
	size_t len;
	ir_ref ref;
	sym = parse_ID(sym, &str, &len);
	IR_ASSERT(len <= 0xffffffff);
	ref = ir_strtab_find(&type_tab, str, (uint32_t)len);
	if (!ref) yy_error("invalid type");
	*t = ref;
	return sym;
}

static int parse_func(int sym, uint8_t *op) {
	const char *str;
	size_t len;
	ir_ref ref;
	sym = parse_ID(sym, &str, &len);
	IR_ASSERT(len <= 0xffffffff);
	ref = ir_strtab_find(&op_tab, str, (uint32_t)len);
	if (!ref) yy_error("invalid op");
	*op = ref - 1;
	return sym;
}

static int parse_val(int sym, ir_parser_ctx *p, uint8_t op, uint32_t n, ir_ref *ref) {
	const char *str;
	size_t len;
	ir_val val;
	uint32_t kind = IR_OPND_KIND(ir_op_flags[op], n);
	uint8_t ret_type;
	uint32_t flags;
	uint32_t params_count;
	uint8_t param_types[256];
	switch (sym) {
		case YY_ID:
			sym = parse_ID(sym, &str, &len);
			if (!IR_IS_REF_OPND_KIND(kind)) yy_error("unexpected reference");
			*ref = ir_use_var(p, n, str, len);
			break;
		case YY_STRING:
			sym = parse_STRING(sym, &str, &len);
			if (kind != IR_OPND_STR) yy_error("unexpected string");
			*ref = ir_strl(p->ctx, str, len);
			break;
		case YY_DECNUMBER:
			sym = parse_DECNUMBER(sym, IR_I32, &val);
			if (kind != IR_OPND_NUM && kind != IR_OPND_PROB) yy_error("unexpected number");
			if (val.i64 < 0 || val.i64 > 0x7fffffff) yy_error("number out of range");
			*ref = val.i32;
			break;
		case YY_NULL:
			sym = get_sym();
			*ref = IR_UNUSED;
			break;
		case YY_FUNC:
			sym = get_sym();
			if (kind != IR_OPND_PROTO) yy_error("unexpected function prototype");
			flags = 0;
			sym = parse_ir_func_proto(sym, p, &flags, &ret_type, &params_count, param_types);
			*ref = ir_proto(p->ctx, flags, ret_type, params_count, param_types);
			break;
		default:
			yy_error_sym("unexpected", sym);
	}
	return sym;
}

static int parse_const(int sym, uint8_t t, ir_val *val) {
	switch (sym) {
		case YY_DECNUMBER:
			sym = parse_DECNUMBER(sym, t, val);
			break;
		case YY_HEXNUMBER:
			sym = parse_HEXNUMBER(sym, t, val);
			break;
		case YY_FLOATNUMBER:
			sym = parse_FLOATNUMBER(sym, t, val);
			break;
		case YY_CHARACTER:
			sym = parse_CHARACTER(sym, val);
			break;
		case YY_INF:
			sym = get_sym();
			if (t == IR_DOUBLE) val->d = INFINITY; else val->f = INFINITY;
			break;
		case YY_NAN:
			sym = get_sym();
			if (t == IR_DOUBLE) val->d = NAN; else val->f = NAN;
			break;
		case YY__MINUS:
			sym = get_sym();
			if (sym != YY_INF) {
				yy_error_sym("'inf' expected, got", sym);
			}
			sym = get_sym();
			if (t == IR_DOUBLE) val->d = -INFINITY; else val->f = -INFINITY;
			break;
		default:
			yy_error_sym("unexpected", sym);
	}
	return sym;
}

static int parse_ID(int sym, const char **str, size_t *len) {
	if (sym != YY_ID) {
		yy_error_sym("<ID> expected, got", sym);
	}
	*str = (const char*)yy_text; *len = yy_pos - yy_text;
	sym = get_sym();
	return sym;
}

static int parse_DECNUMBER(int sym, uint32_t t, ir_val *val) {
	if (sym != YY_DECNUMBER) {
		yy_error_sym("<DECNUMBER> expected, got", sym);
	}
	if (t == IR_DOUBLE) val->d = atof((const char*)yy_text);
	else if (t == IR_FLOAT) val->f = strtof((const char*)yy_text, NULL);
	else if (IR_IS_TYPE_SIGNED(t)) val->i64 = atoll((const char*)yy_text);
	else val->u64 = strtoull((const char*)yy_text, NULL, 10);
	sym = get_sym();
	return sym;
}

static int parse_HEXNUMBER(int sym, uint32_t t, ir_val *val) {
	if (sym != YY_HEXNUMBER) {
		yy_error_sym("<HEXNUMBER> expected, got", sym);
	}
	val->u64 = strtoull((const char*)yy_text + 2, NULL, 16);
	sym = get_sym();
	return sym;
}

static int parse_FLOATNUMBER(int sym, uint32_t t, ir_val *val) {
	if (sym != YY_FLOATNUMBER) {
		yy_error_sym("<FLOATNUMBER> expected, got", sym);
	}
	if (t == IR_DOUBLE) val->d = atof((const char*)yy_text); else val->f = strtof((const char*)yy_text, NULL);
	sym = get_sym();
	return sym;
}

static int parse_CHARACTER(int sym, ir_val *val) {
	if (sym != YY_CHARACTER) {
		yy_error_sym("<CHARACTER> expected, got", sym);
	}
	if ((char)yy_text[1] != '\\') {
			val->i64 = (char)yy_text[1];
		} else if ((char)yy_text[2] == '\\') {
			val->i64 = '\\';
		} else if ((char)yy_text[2] == 'r') {
			val->i64 = '\r';
		} else if ((char)yy_text[2] == 'n') {
			val->i64 = '\n';
		} else if ((char)yy_text[2] == 't') {
			val->i64 = '\t';
		} else if ((char)yy_text[2] == '0') {
			val->i64 = '\0';
		} else {
			IR_ASSERT(0);
		}
	sym = get_sym();
	return sym;
}

static int parse_STRING(int sym, const char **str, size_t *len) {
	if (sym != YY_STRING) {
		yy_error_sym("<STRING> expected, got", sym);
	}
	*str = (const char*)yy_text + 1; *len = yy_pos - yy_text - 2;
	sym = get_sym();
	return sym;
}

static void parse(ir_loader *loader) {
	int sym;

	yy_pos = yy_text = yy_buf;
	yy_line = 1;
	sym = parse_ir(get_sym(), loader);
	if (sym != YY_EOF) {
		yy_error_sym("<EOF> expected, got", sym);
	}
}

static void yy_error(const char *msg) {
	fprintf(stderr, "ERROR: %s at line %d\n", msg, yy_line);
	exit(2);
}

static void yy_error_sym(const char *msg, int sym) {
	fprintf(stderr, "ERROR: %s '%s' at line %d\n", msg, sym_name[sym], yy_line);
	exit(2);
}

static void yy_error_str(const char *msg, const char *str) {
	fprintf(stderr, "ERROR: %s '%s' at line %d\n", msg, str, yy_line);
	exit(2);
}

int ir_load(ir_loader *loader, FILE *f) {
	long pos, end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);
	yy_buf = alloca(end - pos + 1);
	yy_end = yy_buf + (end - pos);
	fread((void*)yy_buf, (end - pos), 1, f);
	*(unsigned char*)yy_end = 0;

	parse(loader);

	return 1;
}

void ir_loader_init(void)
{
	ir_ref i;

	ir_strtab_init(&type_tab, IR_LAST_OP, 0);
	for (i = 1; i < IR_LAST_TYPE; i++) {
		ir_strtab_lookup(&type_tab, ir_type_cname[i], (uint32_t)strlen(ir_type_cname[i]), i);
	}

	ir_strtab_init(&op_tab, IR_LAST_OP, 0);
	for (i = 0; i < IR_LAST_OP; i++) {
		ir_strtab_lookup(&op_tab, ir_op_name[i], (uint32_t)strlen(ir_op_name[i]), i + 1);
	}
}

void ir_loader_free(void)
{
	ir_strtab_free(&type_tab);
	ir_strtab_free(&op_tab);
}
