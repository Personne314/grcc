#pragma once

#include <string>
#include <vector>


enum RegexTreeType : unsigned char {
	REG_TREE_EXPR,
	REG_TREE_BEGIN,
	REG_TREE_END,
	REG_TREE_UNION,
	REG_TREE_CONCAT,
	REG_TREE_ATOM,
	REG_TREE_QUANT,
	REG_TREE_LETTER,
	REG_TREE_LIST,
	REG_TREE_ELT,
	REG_TREE_CHAR,
	REG_TREE_ESC,
	REG_TREE_SKIP
};



// Macro definition to manipulate tokens.
#define TOKEN_MAKE(type, val) ((type<<8) + val)
#define TOKEN_SPLIT(token, type, val) \
	type = token>>8; \
	val = token&0xFF;
#define TOKEN_TYPE(token, type) (type = token>>8)
#define TOKEN_VAL(token, val) (val = token&0xFF)





