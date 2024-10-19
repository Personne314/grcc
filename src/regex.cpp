#include "regex.h"

#include <iostream>
#include <cctype>


using std::string, std::vector; 
using std::cout, std::cerr, std::endl;


// All the diffrent types of tokens.
enum RegexTokenType : char {
	REG_TK_CHAR,
	REG_TK_ESC,
	REG_TK_OP,
	REG_TK_SKIP,
	REG_TK_INT,
	REG_TK_EOE
};


// Lexer states.
enum RegexLexerState : char {
	REG_LEX_NONE,
	REG_LEX_ESC,
	REG_LEX_HEX_1,
	REG_LEX_HEX_2,
	REG_LEX_BRK,
	REG_LEX_SKIP
};



// Analyse current character in case of a new token.
void regexLexerNone(string::const_iterator &it, const string &str, int &val,
	vector<int> &tokens, char c, RegexLexerState &state, bool opened[2]) {
	switch(c) {
	// Case of a start-line operator or a negation operator.
	case '^':
		if (it == str.begin()+1 || *(it-1) == '[') 
			tokens.push_back(TOKEN_MAKE(REG_TK_OP, '^'));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '^'));
		break;
	// Case of an end-line operator.
	case '$':
		if (it == str.end()) 
			tokens.push_back(TOKEN_MAKE(REG_TK_OP, '$'));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '$'));
		break;
	// Case of the point representing all charaters.
	case '.':
		if (!opened[0]) tokens.push_back(TOKEN_MAKE(REG_TK_ESC, c));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c));
		break; 
	// Case of the start of an escape sequence.
	case '\\':
		state = REG_LEX_ESC;
		break;
	// Case of classes.
	case '[': case ']':
		opened[0] = c == '[';
		tokens.push_back(TOKEN_MAKE(REG_TK_OP, c));
		break;
	// Case of an interval.
	case '-':
		if (opened[0]) tokens.push_back(TOKEN_MAKE(REG_TK_OP, '-'));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '-'));
		break;
	// Case of quantification separator.
	case ',':
		if (opened[1]) tokens.push_back(TOKEN_MAKE(REG_TK_OP, ','));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, ','));
		break;
	// Case of all other operators.
	case '|': case '(': case ')': case '{': 
	case '}': case '+': case '*': case '?':
		if (c == '(') state = REG_LEX_BRK;
		opened[1] = c == '{';
		if (!opened[0]) tokens.push_back(TOKEN_MAKE(REG_TK_OP, c));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c));
		break;
	// Case of numbers, considered as int only if contained in a quantificator.
	case '0': case '1': case '2': case '3': case '4': 
	case '5': case '6': case '7': case '8': case '9': 
		if (!opened[1]) tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c));
		else {
			opened[2] = true;
			val = 10*val + c-'0';	
		}
		break;
	// All other characters are considered as characters.
	default:
		tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c));
	}
}

// Analyse current character in case of an escape sequence.
bool regexLexerEsc(vector<int> &tokens, char c, RegexLexerState &state) {
	state = REG_LEX_NONE;
	switch(c) {
	// Case of an operator. 
	case '(':  case ')': case '[': case ']':
	case '+':  case '*': case '?': case '.':
	case '\\': case '|': case '^': case '$':
	case '{':  case '}': case '-':
		tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c)); break;
	// Case of special charaters.
	case 'n': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\n')); break;
	case 't': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\t')); break;
	case 'r': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\r')); break;
	case 'v': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\v')); break;
	case 'f': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\f')); break;
	// Case of epsilon.
	case 'e': tokens.push_back(TOKEN_MAKE(REG_TK_ESC, 'e')); break;
	// Case of charater classes.
	case 'w': case 'W': case 'd': case 'D':
	case 's': case 'S': case 'a': case 'A':
	case 'l': case 'L': case 'u': case 'U':
		tokens.push_back(TOKEN_MAKE(REG_TK_ESC, c));
		break;
	// Case of an hex sequence.
	case 'x': state = REG_LEX_HEX_1; break;
	default: return true;
	}
	return false;
}

// Analyse the current character in case of the start of an hex sequence.
bool regexLexerHex1(vector<int> &tokens, char c, RegexLexerState &state) {
	int val = 0;
	if (c >= '0' && c <= '9') val = c-'0';
	else if (c >= 'a' && c <= 'f') val = c-'a'+10;
	else if (c >= 'A' && c <= 'F') val = c-'A'+10;
	else return true;
	tokens.push_back(TOKEN_MAKE(0, val));
	state = REG_LEX_HEX_2;
	return false;
}


// Analyse the current character in case of the end of an hex sequence.
bool regexLexerHex2(vector<int> &tokens, char c, RegexLexerState &state) {
	int val = tokens.back();
	if (c >= '0' && c <= '9') val = 16*val + c-'0';
	else if (c >= 'a' && c <= 'f') val = 16*val + c-'a'+10;
	else if (c >= 'A' && c <= 'F') val = 16*val + c-'A'+10;
	else return true;
	tokens.back() = TOKEN_MAKE(REG_TK_CHAR, val);
	state = REG_LEX_NONE;
	return false;
}

// Analyse the character following (? to detect skip structure.
bool regexLexerSkip(vector<int> &tokens, char c, RegexLexerState &state) {
	if (c == '!' || c == '=') {
		state = REG_LEX_NONE;
		tokens.push_back(TOKEN_MAKE(REG_TK_SKIP, c));
	} else return true;
	return false;
}

// Analyse the character following ( to detect skip structure.
void regexLexerBrk(string::const_iterator &it, const string &str, int &val, 
	vector<int> &tokens, char c, RegexLexerState &state, bool opened[2]) {
	if (c == '?') state = REG_LEX_SKIP;
	else {
		state = REG_LEX_NONE;
		regexLexerNone(it, str, val, tokens, c, state, opened);
	}
}



// Lexer for regex.
// str : regex string to analyse.
// tokens : list of generated tokens.
bool regexLexer(const string &str, vector<int> &tokens) {

	// Initialization of lexer variables.
	char c = 0;							// Current character.
	int val = 0;						// Int value readed.
	auto it = str.begin();				// Iterator on the input string.
	bool opened[3] = {false};			// Bools for some states.
	RegexLexerState state = REG_LEX_NONE; 	// Scanner state.

	// For each char in the string.
	while (it != str.end()) {
		c = *it;
		++it;

		// Detects ints in quantifiers.
		if (opened[2] && (c == ',' || c == '}')) {
			tokens.push_back(TOKEN_MAKE(REG_TK_INT, val));
			opened[2] = false;
			val = 0;
		}

		// Scan each character depending of the current state.
		switch (state) {
		case REG_LEX_NONE: 
			regexLexerNone(it, str, val, tokens, c, state, opened);
			break;
		case REG_LEX_ESC:
			if (regexLexerEsc(tokens, c, state)) 
				goto ERROR_UNKNOWN_ESCAPE;  
			break;
		case REG_LEX_HEX_1:
			if (regexLexerHex1(tokens, c, state)) 
				goto ERROR_UNKNOWN_HEX;
			break;
		case REG_LEX_HEX_2:
			if (regexLexerHex2(tokens, c, state)) 
				goto ERROR_UNKNOWN_HEX;
			break;
		case REG_LEX_SKIP:
			if (regexLexerSkip(tokens, c, state))
				goto ERROR_UNKNOWN_SKIP;
			break;
		case REG_LEX_BRK:
			regexLexerBrk(it, str, val, tokens, c, state, opened); 
			break;
		}
	}

	// Add the token End Of Expression.
	tokens.push_back(TOKEN_MAKE(REG_TK_EOE, 0));

	// Test that the state is ok.
	if (state == REG_LEX_NONE) return true;
	else if (state == REG_LEX_ESC) goto ERROR_UNKNOWN_ESCAPE;

// Error cases.
ERROR_UNKNOWN_HEX:
	std::cout << ((int)state) << " " <<((int)REG_LEX_HEX_2) << endl;
	cerr << "grcc: \e[0;31merror:\e[0;0m unknown hex sequence: '\\x";
	if (state == REG_LEX_HEX_2) cerr << (*(it-2));
	cerr << c << "'" << endl;  
	return false;

ERROR_UNKNOWN_ESCAPE:
	cerr << "grcc: \e[0;31merror:\e[0;0m unknown escape sequence: " 
		 << "'\\" << c << "'" << endl;
	return false;

ERROR_UNKNOWN_SKIP:
	cerr << "grcc: \e[0;31merror:\e[0;0m unknown skip sequence: " 
		 << "'(?" << c << "'" << endl;
	return false;

}


bool regexParseExpr(vector<int>::const_iterator it, RegexTreeExpr &tree);
bool regexParseBegin(vector<int>::const_iterator &it, bool &begin);
bool regexParseEnd(vector<int>::const_iterator &it, bool &end);
bool regexParseUnion(vector<int>::const_iterator &it, RegexTreeUnion *&node);
bool regexParseConcat(vector<int>::const_iterator &it, RegexTreeConcat *&node);
bool regexParseAtom(vector<int>::const_iterator &it, RegexTreeAtom *&node);
bool regexParseQuant(vector<int>::const_iterator &it, RegexTreeQuant *&node);
bool regexParseLetter(vector<int>::const_iterator &it, RegexTreeNode *&node);
bool regexParseList(vector<int>::const_iterator &it, RegexTreeClass *&node);
bool regexParseElt(vector<int>::const_iterator &it, vector<RegexTreeNode*> &nodes);
void regexParseLitt(vector<int>::const_iterator &it, RegexTreeLitt *&node);
bool regexParseUnion1(vector<int>::const_iterator &it, RegexTreeUnion *&node);
bool regexParseConcat1(vector<int>::const_iterator &it, RegexTreeConcat *&node);
bool regexParseAtom1(vector<int>::const_iterator &it, RegexTreeAtom *&node);
bool regexParseQuant1(vector<int>::const_iterator &it, RegexTreeQuant *&node);
bool regexParseQuant2(vector<int>::const_iterator &it, RegexTreeQuant *&node);
bool regexParseLetter1(vector<int>::const_iterator &it, RegexTreeNode *&node);
bool regexParseList1(vector<int>::const_iterator &it, RegexTreeClass *&node);
bool regexParseElt1(vector<int>::const_iterator &it, vector<RegexTreeNode*> &nodes, RegexTreeLitt *&node);





bool regexParseExpr(vector<int>::const_iterator it, RegexTreeExpr &tree) {
	char type;
	TOKEN_TYPE(*it, type);

	if (*it == TOKEN_MAKE(REG_TK_OP, '^') ||
		*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		tree = RegexTreeExpr();
		RegexTreeUnion *union_node = nullptr;
		if (!regexParseBegin(it, tree.m_begin)) goto ERROR_PARSE_EXPR;
		if (!regexParseUnion(it, union_node)) goto ERROR_PARSE_EXPR;
		tree.m_node = union_node;
		if (!regexParseEnd(it, tree.m_end)) goto ERROR_PARSE_EXPR;
		return true;
	}

ERROR_PARSE_EXPR:
	tree = RegexTreeExpr();
	return false;

}

bool regexParseBegin(vector<int>::const_iterator &it, bool &begin) {
	char type;
	TOKEN_TYPE(*it, type);

	if (*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		begin = false;
		return true;
	} else if (*it == TOKEN_MAKE(REG_TK_OP, '^')) {
		++it;
		begin = true;
		return true;
	}
	return false;

}

bool regexParseEnd(vector<int>::const_iterator &it, bool &end) {
	char type;
	TOKEN_TYPE(*it, type);
	
	if (*it == TOKEN_MAKE(REG_TK_OP, '$') ) {
		end = true;
		return true;
	} else if (type == REG_TK_EOE) {
		end = false;
		return true;
	}
	return false;

}

bool regexParseUnion(vector<int>::const_iterator &it, RegexTreeUnion *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (!node) node = new RegexTreeUnion();
	vector<RegexTreeNode*> &nodes = node->m_nodes;

	RegexTreeConcat *concat_node = nullptr;
	if (*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		if (!regexParseConcat(it, concat_node)) goto PARSE_UNION_ERROR;
		nodes.push_back(concat_node);
		if (!regexParseUnion1(it, node)) goto PARSE_UNION_ERROR;
		return true;
	}

PARSE_UNION_ERROR:
	if (node) delete node;
	node = nullptr;
	return false;

}

bool regexParseConcat(vector<int>::const_iterator &it, RegexTreeConcat *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (!node) node = new RegexTreeConcat();
	vector<RegexTreeNode*> &nodes = node->m_nodes;

	RegexTreeAtom *atom_node = nullptr;
	if (*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		if (!regexParseAtom(it, atom_node)) goto PARSE_CONCAT_ERROR;
		nodes.push_back(atom_node);
		if (!regexParseConcat1(it, node)) goto PARSE_CONCAT_ERROR;
		return true;
	}

PARSE_CONCAT_ERROR:
	if (node) delete node;
	node = nullptr;
	return false;

}

bool regexParseAtom(vector<int>::const_iterator &it, RegexTreeAtom *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	node = new RegexTreeAtom();
	if (*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		if (!regexParseLetter(it, node->m_node)) goto ERROR_PARSE_ATOM;
		if (!regexParseQuant(it, node->m_quant)) goto ERROR_PARSE_ATOM;
	} else if (*it == TOKEN_MAKE(REG_TK_OP, '(')) {
		++it;
		if (!regexParseAtom1(it, node)) goto ERROR_PARSE_ATOM;
	}

ERROR_PARSE_ATOM:
	delete node;
	node = nullptr;
	return false;

}

bool regexParseQuant(vector<int>::const_iterator &it, RegexTreeQuant *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	node = new RegexTreeQuant();
	if (*it == TOKEN_MAKE(REG_TK_OP, '$') ||
		*it == TOKEN_MAKE(REG_TK_OP, '|') ||
		*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, ')') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC || type == REG_TK_EOE) {
		return true;

	} else if (*it == TOKEN_MAKE(REG_TK_OP, '+')) {
		node->m_min = 1;
		node->m_max = -1;
		++it;
		return true;

	} else if (*it == TOKEN_MAKE(REG_TK_OP, '*')) {
		node->m_min = 0;
		node->m_max = -1;
		++it;
		return true;

	} else if (*it == TOKEN_MAKE(REG_TK_OP, '?')) {
		node->m_min = 0;
		node->m_max = 1;
		++it;
		return true;
	
	} else if (*it == TOKEN_MAKE(REG_TK_OP, '{')) {
		++it;
		TOKEN_TYPE(*it, type);
		if (type == REG_TK_INT) {
			++it;
			TOKEN_VAL(*it, node->m_min);
			if (regexParseQuant2(it, node)) return true;
		}
	}

	delete node;
	node = nullptr;
	return false;

}

bool regexParseLetter(vector<int>::const_iterator &it, RegexTreeNode *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	node = new RegexTreeQuant();
	if (type == REG_TK_CHAR || type == REG_TK_ESC) {
		RegexTreeLitt *litt_node = new RegexTreeLitt();
		node = litt_node;
		it++;
		regexParseLitt(it, litt_node);
		return true;
	} else if (*it == TOKEN_MAKE(REG_TK_OP, '[')) {
		++it;
		if (!regexParseLetter1(it, node)) return false;
		return true;
	}
	return false;

}

bool regexParseList(vector<int>::const_iterator &it, RegexTreeClass *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (type != REG_TK_CHAR && type != REG_TK_ESC) return false;
	if (!regexParseElt(it, node->m_nodes)) return false;
	if (!regexParseList1(it, node)) return false;
	return true;

}

bool regexParseElt(vector<int>::const_iterator &it, std::vector<RegexTreeNode*> &nodes) {
	char type;
	TOKEN_TYPE(*it, type);

	if (type != REG_TK_CHAR && type != REG_TK_ESC) return false;
	RegexTreeLitt *litt_node = new RegexTreeLitt();
	it++;
	regexParseLitt(it, litt_node);
	nodes.push_back(litt_node);
	if (!regexParseElt1(it, nodes, litt_node)) return false;
	return true;

}



void regexParseLitt(vector<int>::const_iterator &it, RegexTreeLitt *&node) {
	char type, val;
	TOKEN_SPLIT(*it, type, val);
	node->m_esc = (type == REG_TK_ESC);	
	node->m_val = val;
}

bool regexParseUnion1(vector<int>::const_iterator &it, RegexTreeUnion *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (*it == TOKEN_MAKE(REG_TK_OP, '$') ||
		*it == TOKEN_MAKE(REG_TK_OP, ')') ||
		type == REG_TK_EOE) {
		return true;
	} else if (*it == TOKEN_MAKE(REG_TK_OP, '|')) {
		++it;
		if (!regexParseUnion(it, node)) return false;
		return true;
	}
	return false;

}

bool regexParseConcat1(vector<int>::const_iterator &it, RegexTreeConcat *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (*it == TOKEN_MAKE(REG_TK_OP, '$') ||
		*it == TOKEN_MAKE(REG_TK_OP, '|') ||
		*it == TOKEN_MAKE(REG_TK_OP, ')') ||
		type == REG_TK_EOE) {
		return true;
	} else if (
		*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		++it;
		if (!regexParseConcat(it, node)) return false;
		return true;
	}
	return false;

}

bool regexParseAtom1(vector<int>::const_iterator &it, RegexTreeAtom *&node) {
	char type, val;
	TOKEN_SPLIT(*it, type, val);

	RegexTreeUnion *union_node = nullptr;
	if (*it == TOKEN_MAKE(REG_TK_OP, '(') ||
		*it == TOKEN_MAKE(REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {

		if (!regexParseUnion(it, union_node)) return false;
		node->m_node = union_node;
		if (*(it++) != TOKEN_MAKE(REG_TK_OP, ')')) return false;
		if (!regexParseQuant(it, node->m_quant)) return false;
		return true;

	} else if (type == REG_TK_SKIP) {
		RegexTreeSkip *skip_node = new RegexTreeSkip();
		skip_node->m_match = (val == '=');
		node->m_node = skip_node;
		++it;
		if (!regexParseUnion(it, union_node)) return false;
		skip_node->m_node = union_node;
		if (*(it++) != TOKEN_MAKE(REG_TK_OP, ')')) return false;
		return true;
	}

	return false;
}

bool regexParseQuant1(vector<int>::const_iterator &it, RegexTreeQuant *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (type = REG_TK_INT) {
		++it;
		TOKEN_VAL(*it, node->m_max);
		if (*it == TOKEN_MAKE(REG_TK_OP, '}')) {
			++it;
			return true;
		}
	} else if (*it == TOKEN_MAKE(REG_TK_OP, '}')) {
		++it;
		node->m_max = -1;
		return true;
	}
	return false;

}

bool regexParseQuant2(vector<int>::const_iterator &it, RegexTreeQuant *&node) {

	if (*it == TOKEN_MAKE(REG_TK_OP, '}')) {
		++it;
		node->m_max = node->m_min;
		return true;
	} else if (*it == TOKEN_MAKE(REG_TK_OP, ',')) {
		++it;
		if (regexParseQuant1(it, node)) return true;
	}
	return false;

}

bool regexParseLetter1(vector<int>::const_iterator &it, RegexTreeNode *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	RegexTreeClass *class_node = new RegexTreeClass();
	node = class_node;
	if (*it == TOKEN_MAKE(REG_TK_OP, '^')) ++it;
	else if (type != REG_TK_CHAR && type != REG_TK_ESC) return false;

	regexParseList(it, class_node);
	if (*it != TOKEN_MAKE(REG_TK_OP, ']')) return false;
	++it;
	return true;

}

bool regexParseList1(vector<int>::const_iterator &it, RegexTreeClass *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (*it == TOKEN_MAKE(REG_TK_OP, ']')) return true;
	if (type != REG_TK_CHAR && type != REG_TK_ESC) return false;
	if (!regexParseList(it, node)) return false;
	return true;

}

bool regexParseElt1(vector<int>::const_iterator &it, vector<RegexTreeNode*> &nodes, RegexTreeLitt *&node) {
	char type;
	TOKEN_TYPE(*it, type);

	if (*it == TOKEN_MAKE(REG_TK_OP, ']') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) return true;
	if (*it != TOKEN_MAKE(REG_TK_OP, '-')) return false;

	TOKEN_TYPE(*(++it), type);
	if (type != REG_TK_CHAR && type != REG_TK_ESC) return false;
	
	++it;
	RegexTreeLitt *litt_node = new RegexTreeLitt();
	regexParseLitt(it, litt_node);
	RegexTreeSeq *seq_node = new RegexTreeSeq();
	seq_node->m_litt_min = node;
	seq_node->m_litt_max = litt_node;
	nodes.back() = seq_node;
	return true;

}







bool regexParser(const string &str) {

	// Get the list of tokens from the expression string.
	vector<int> tokens;
	if (!regexLexer(str, tokens)) return false;

	RegexTreeExpr tree;
	if (!regexParseExpr(tokens.begin(), tree)) return false;
	tree.print();

	return true;

}





RegexTreeNode::RegexTreeNode() {}
RegexTreeNode::~RegexTreeNode() {}

RegexTreeExpr::RegexTreeExpr() : m_begin(false), m_end(false), m_node(nullptr) {}
RegexTreeExpr::~RegexTreeExpr() {if (m_node) delete m_node;}
RegexTreeType RegexTreeExpr::getType() const {return REG_TREE_EXPR;}
void RegexTreeExpr::print(int depth) const {
	cout << "Expr -> ";
	if (m_begin) cout << "Begin ";
	if (m_end) cout << "End ";
	cout << endl;
	m_node->print(0);
}

RegexTreeUnion::RegexTreeUnion() : m_nodes() {}
RegexTreeUnion::~RegexTreeUnion() {
	for (RegexTreeNode *node : m_nodes) if (node) delete node;
}
RegexTreeType RegexTreeUnion::getType() const {return REG_TREE_UNION;}
void RegexTreeUnion::print(int depth) const {
	
	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Union" << endl;
		else cout << "│ ";
	}
	
	for (RegexTreeNode *node : m_nodes) {
		if (node) node->print(depth+1);
	}

	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeConcat::RegexTreeConcat() : m_nodes() {}
RegexTreeConcat::~RegexTreeConcat() {
	for (RegexTreeNode *node : m_nodes) if (node) delete node;
}
RegexTreeType RegexTreeConcat::getType() const {return REG_TREE_CONCAT;}
void RegexTreeConcat::print(int depth) const {

	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Concat" << endl;
		else cout << "│ ";
	}
	
	for (RegexTreeNode *node : m_nodes) {
		if (node) node->print(depth+1);
	}

	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeQuant::RegexTreeQuant() : m_min(0), m_max(0) {}
RegexTreeQuant::~RegexTreeQuant() {}
RegexTreeType RegexTreeQuant::getType() const {return REG_TREE_QUANT;}
void RegexTreeQuant::print(int depth) const {
	
	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Quant (" << m_min << "," << m_max << ")" << endl;
		else cout << "│ ";
	}
	
	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeAtom::RegexTreeAtom() : m_node(nullptr), m_quant(nullptr) {}
RegexTreeAtom::~RegexTreeAtom() {
	if (m_node) delete m_node;
	if (m_quant) delete m_quant;
}
RegexTreeType RegexTreeAtom::getType() const {return REG_TREE_ATOM;}
void RegexTreeAtom::print(int depth) const {

	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Atom" << endl;
		else cout << "│ ";
	}
	
	if (m_node) m_node->print(depth+1);
	if (m_quant) m_quant->print(depth+1);
	
	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeSkip::RegexTreeSkip() : m_match(false), m_node(nullptr) {}
RegexTreeSkip::~RegexTreeSkip() {if (m_node) delete m_node;}
RegexTreeType RegexTreeSkip::getType() const {return REG_TREE_SKIP;}
void RegexTreeSkip::print(int depth) const {

	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Skip (" << (m_match ? "match" : "unmatch") << ")" << endl;
		else cout << "│ ";
	}
	
	if (m_node) m_node->print(depth+1);
	
	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeClass::RegexTreeClass() : m_match(false), m_nodes() {}
RegexTreeClass::~RegexTreeClass() {
	for (RegexTreeNode *node : m_nodes) if (node) delete node;
}
RegexTreeType RegexTreeClass::getType() const {return REG_TREE_CLASS;}
void RegexTreeClass::print(int depth) const {

	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Class (" << (m_match ? "match" : "unmatch") << ")" << endl;
		else cout << "│ ";
	}
	
	for (RegexTreeNode *node : m_nodes) {
		if (node) node->print(depth+1);
	}

	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeLitt::RegexTreeLitt() : m_esc(false), m_val(0) {}
RegexTreeLitt::~RegexTreeLitt() {}
RegexTreeType RegexTreeLitt::getType() const {return REG_TREE_LITT;}
void RegexTreeLitt::print(int depth) const {

	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Litt (" << (m_esc ? "escape, " : "char, ");
		else cout << "│ ";
	}
	
	if (std::isgraph(m_val)) cout << ((char)m_val);
	else cout << std::hex << m_val;
	cout << ")" << endl;

	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;

}

RegexTreeSeq::RegexTreeSeq() : m_litt_min(nullptr), m_litt_max(nullptr) {}
RegexTreeSeq::~RegexTreeSeq() {
	if (m_litt_min) delete m_litt_min;
	if (m_litt_max) delete m_litt_max;
}
RegexTreeType RegexTreeSeq::getType() const {return REG_TREE_SEQ;}
void RegexTreeSeq::print(int depth) const {

	for (int i = 0; i < depth; ++i) {
		if (i == depth-1) cout << "├╴Seq" << endl;
		else cout << "│ ";
	}
	
	if (m_litt_min) m_litt_min->print(depth+1);
	if (m_litt_max) m_litt_max->print(depth+1);
	
	for (int i = 0; i < depth-1; ++i) cout << "│ ";
	cout << endl;


}
