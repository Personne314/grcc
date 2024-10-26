#include "regex.h"

#include "constants.h"

#include <iostream>
#include <cctype>
#include <format>



// Usings.
using std::string, std::vector, std::endl;
using std::format, std::isgraph;
using grcc::ansi::RED, grcc::ansi::RST, grcc::ansi::BOLD;



// All the diffrent types of tokens.
enum RegexTokenType : char {
	REG_TK_CHAR,
	REG_TK_ESC,
	REG_TK_OP,
	REG_TK_LOOKAHEAD,
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
	REG_LEX_LOOKAHEAD
};





// Converts a token to a string.
string stringFromToken(int tk) {
	char type, val;
	TOKEN_SPLIT(tk, type, val);

	// Identifies the type of the token.
	string str = "(";
	switch (type) {
	case REG_TK_CHAR:
		str += "char: "; break;
	case REG_TK_ESC:
		str += "esc: "; break;
	case REG_TK_OP:
		str += "op: "; break;
	case REG_TK_LOOKAHEAD:
		str += "lookahead: "; break;
	case REG_TK_INT:
		str += "int: "; break;
	case REG_TK_EOE:
		str += "EOE: "; break;
	}

	// Then adds the value as a char if it's printable or it's hex value.
	if (isgraph(val)) str = str + "'" + val + "'";
	else str += "0x" + format("{:02X}", val);
	return str + ")";

}



// Analyses the current character in case of a new token.
void regexLexerNone(const string &str, int &val, vector<int> &tokens, 
	char c, RegexLexerState &state, bool flags[3], size_t pos) {
	switch(c) {
	// Case of a start-line operator or a negation operator.
	case '^':
		if (pos > 0 && str[pos-1] == '[') tokens.push_back(TOKEN_MAKE(REG_TK_OP, '^', pos));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '^', pos));
		break;
	// Case of the point representing all charaters.
	case '.':
		if (!flags[0]) tokens.push_back(TOKEN_MAKE(REG_TK_ESC, c, pos));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c, pos));
		break; 
	// Case of the start of an escape sequence.
	case '\\':
		state = REG_LEX_ESC;
		break;
	// Case of classes.
	case '[': case ']':
		flags[0] = c == '[';
		tokens.push_back(TOKEN_MAKE(REG_TK_OP, c, pos));
		break;
	// Case of an interval.
	case '-':
		if (flags[0]) tokens.push_back(TOKEN_MAKE(REG_TK_OP, '-', pos));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '-', pos));
		break;
	// Case of quantification separator.
	case ',':
		if (flags[1]) tokens.push_back(TOKEN_MAKE(REG_TK_OP, ',', pos));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, ',', pos));
		break;
	// Case of all other operators.
	case '|': case '(': case ')': case '{': 
	case '}': case '+': case '*': case '?':
		if (c == '(') state = REG_LEX_BRK;
		flags[1] = c == '{';
		if (!flags[0]) tokens.push_back(TOKEN_MAKE(REG_TK_OP, c, pos));
		else tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c, pos));
		break;
	// Case of numbers, considered as int only if contained in a quantificator.
	case '0': case '1': case '2': case '3': case '4': 
	case '5': case '6': case '7': case '8': case '9': 
		if (!flags[1]) tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c, pos));
		else {
			flags[2] = true;
			val = 10*val + c-'0';	
		}
		break;
	// All other characters are considered as characters.
	default:
		tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c, pos));
	}
}

// Analyses the current character in case of an escape sequence.
bool regexLexerEsc(vector<int> &tokens, char c, RegexLexerState &state, size_t pos) {
	state = REG_LEX_NONE;
	switch(c) {
	// Case of an operator. 
	case '(':  case ')': case '[': case ']':
	case '+':  case '*': case '?': case '.':
	case '\\': case '|': case '^': case '$':
	case '{':  case '}': case '-':
		tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, c, pos)); break;
	// Case of special charaters.
	case 'n': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\n', pos)); break;
	case 't': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\t', pos)); break;
	case 'r': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\r', pos)); break;
	case 'v': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\v', pos)); break;
	case 'f': tokens.push_back(TOKEN_MAKE(REG_TK_CHAR, '\f', pos)); break;
	// Case of epsilon.
	case 'e': tokens.push_back(TOKEN_MAKE(REG_TK_ESC, 'e', pos)); break;
	// Case of charater classes.
	case 'w': case 'W': case 'd': case 'D':
	case 's': case 'S': case 'a': case 'A':
	case 'l': case 'L': case 'u': case 'U':
		tokens.push_back(TOKEN_MAKE(REG_TK_ESC, c, pos));
		break;
	// Case of an hex sequence.
	case 'x': state = REG_LEX_HEX_1; break;
	default: return true;
	}
	return false;
}

// Analyses the current character in case of the start of an hex sequence.
bool regexLexerHex1(vector<int> &tokens, char c, RegexLexerState &state, size_t pos) {
	int val = 0;
	bool res = false;
	if (c >= '0' && c <= '9') val = c-'0';
	else if (c >= 'a' && c <= 'f') val = c-'a'+10;
	else if (c >= 'A' && c <= 'F') val = c-'A'+10;
	else res = true;
	tokens.push_back(TOKEN_MAKE(0, val, pos));
	state = REG_LEX_HEX_2;
	return res;
}

// Analyses the current character in case of the end of an hex sequence.
bool regexLexerHex2(vector<int> &tokens, char c, RegexLexerState &state, size_t pos) {
	int val; TOKEN_VAL(tokens.back(), val);
	bool res = false;
	if (c >= '0' && c <= '9') val = 16*val + c-'0';
	else if (c >= 'a' && c <= 'f') val = 16*val + c-'a'+10;
	else if (c >= 'A' && c <= 'F') val = 16*val + c-'A'+10;
	else res = true;
	tokens.back() = TOKEN_MAKE(REG_TK_CHAR, val, pos);
	state = REG_LEX_NONE;
	return res;
}

// Analyses the character following (? to detect a lookahead.
bool regexLexerLook(vector<int> &tokens, char c, RegexLexerState &state, size_t pos) {
	if (c == '!' || c == '=') {
		state = REG_LEX_NONE;
		tokens.pop_back();
		tokens.push_back(TOKEN_MAKE(REG_TK_LOOKAHEAD, c, pos));
	} else return true;
	return false;
}

// Analyses the character following ( to detect a lookahead.
void regexLexerBrk(const string &str, int &val, vector<int> &tokens, 
	char c, RegexLexerState &state, bool flags[3], int pos) {
	state = REG_LEX_NONE;
	if (c == '?') state = REG_LEX_LOOKAHEAD;
	else regexLexerNone(str, val, tokens, c, state, flags, pos);
}



// Lexer for regex.
// str : regex string to analyse.
// tokens : list of generated tokens.
bool regexLexer(const string &str, vector<int> &tokens) {

	// Initialization of lexer variables.
	char c = 0;								// Current character.
	int val = 0;							// Int value readed.
	bool flags[3] = {false};				// Flags.
	RegexLexerState state = REG_LEX_NONE; 	// Scanner state.

	// For each char in the string.
	size_t pos = 0;
	while (pos < str.size()) {
		c = str[pos];

		// Detects ints in quantifiers.
		if (flags[2] && (c == ',' || c == '}')) {
			tokens.push_back(TOKEN_MAKE(REG_TK_INT, val, pos-1));
			flags[2] = false;
			val = 0;
		}

		// Scans each character depending of the current state.
		switch (state) {
		case REG_LEX_NONE: 
			regexLexerNone(str, val, tokens, c, state, flags, pos);
			break;
		case REG_LEX_ESC:
			if (regexLexerEsc(tokens, c, state, pos)) goto ERROR_UNKNOWN_ESCAPE;  
			break;
		case REG_LEX_HEX_1:
			if (regexLexerHex1(tokens, c, state, pos)) goto ERROR_UNKNOWN_HEX;
			break;
		case REG_LEX_HEX_2:
			if (regexLexerHex2(tokens, c, state, pos)) goto ERROR_UNKNOWN_HEX;
			break;
		case REG_LEX_LOOKAHEAD:
			if (regexLexerLook(tokens, c, state, pos)) goto ERROR_UNKNOWN_LOOKAHEAD;
			break;
		case REG_LEX_BRK:
			regexLexerBrk(str, val, tokens, c, state, flags, pos); 
			break;
		}
		++pos;

	}

	// Tests that the state is ok then add the token End Of Expression.
	if (state == REG_LEX_NONE) {
		tokens.push_back(TOKEN_MAKE(REG_TK_EOE, 0, str.size()));
		return true;
	} else if (state == REG_LEX_ESC) goto ERROR_UNKNOWN_ESCAPE;

// Error cases.
ERROR_UNKNOWN_HEX:
	tokens.push_back(TOKEN_MAKE(0,0, pos));
	grcc::cerr << "unknown hex sequence: '\\x" 
		<< ((state == REG_LEX_HEX_2) ? ""+str[pos-1] : "") << c << "'" << endl;  
	return false;
ERROR_UNKNOWN_ESCAPE:
	tokens.push_back(TOKEN_MAKE(0,0, pos));
	grcc::cerr << "unknown escape sequence: '\\" << c << "'" << endl;
	return false;
ERROR_UNKNOWN_LOOKAHEAD:
	tokens.push_back(TOKEN_MAKE(0,0, pos));
	grcc::cerr << "unknown lookahead: '(?" << c << "'" << endl;
	return false;

}



// Prints a node and it's name.
void printPreIndent(string str, int depth) {
	grcc::cout << "";
	for (int i = 0; i < depth; ++i) 
		grcc::cout() << ((i == depth-1) ? "├╴" : "│ ");
	grcc::cout() << BOLD << str << RST << endl;
}

// Prints the indent following a node.
void printPostIndent(int depth) {
	if (depth <= 0) return; 
	grcc::cout << "";
	for (int i = 0; i < depth; ++i) grcc::cout() << "│ ";
	grcc::cout() << endl;
}



// Abstract class to represent an AST node.
RegexTreeNode::RegexTreeNode() {}
RegexTreeNode::~RegexTreeNode() {}

// Node class that serves as the expression root.
RegexTreeExpr::RegexTreeExpr() : m_node(nullptr), m_look(nullptr), m_match(true) {}
RegexTreeExpr::~RegexTreeExpr() {
	if (m_node) delete m_node;
	if (m_look) delete m_look;
}
RegexTreeType RegexTreeExpr::getType() const {return REG_TREE_EXPR;}
string RegexTreeExpr::to_string() const {
	return (m_node ? m_node->to_string() : "") + 
	(m_look ? ("(?" + string(m_match ? "=" : "!") + m_look->to_string() + ")") : "");
}
void RegexTreeExpr::print(int depth) const {
	grcc::cout << BOLD << "Expr :" << RST << endl;
	if (m_node) m_node->print(depth);
	grcc::cout << BOLD << "Followed by " << RST << (m_match ? "(matching)" : "(unmatching)") << endl;
	if (m_look) m_look->print(depth);
	else grcc::cout << BOLD << "None" << RST << endl;
}

// Node class used to represent an union.
RegexTreeUnion::RegexTreeUnion() : m_nodes() {}
RegexTreeUnion::~RegexTreeUnion() {
	for (RegexTreeNode *node : m_nodes) if (node) delete node;
}
RegexTreeType RegexTreeUnion::getType() const {return REG_TREE_UNION;}
string RegexTreeUnion::to_string() const {
	string str = m_nodes[0]->to_string();
	for (auto it = m_nodes.begin()+1; it < m_nodes.end(); ++it) str += "|" + (*it)->to_string();
	return str;
}
void RegexTreeUnion::print(int depth) const {
	printPreIndent("Union", depth);
	for (RegexTreeNode *node : m_nodes) if (node) node->print(depth+1);
	printPostIndent(depth);
}

// Node class used to represent a concatenation.
RegexTreeConcat::RegexTreeConcat() : m_nodes() {}
RegexTreeConcat::~RegexTreeConcat() {
	for (RegexTreeNode *node : m_nodes) if (node) delete node;
}
RegexTreeType RegexTreeConcat::getType() const {return REG_TREE_CONCAT;}
string RegexTreeConcat::to_string() const {
	string str = "";
	for (const RegexTreeNode *node : m_nodes) {
		bool u = node->getType() == REG_TREE_UNION;
		str += (u ? "(" : "") + node->to_string() + (u ? ")" : "");
	}
	return str;
}
void RegexTreeConcat::print(int depth) const {
	printPreIndent("Concat", depth);
	for (RegexTreeNode *node : m_nodes) if (node) node->print(depth+1);
	printPostIndent(depth);
}

// Node class used to represent a quantifier.
RegexTreeQuant::RegexTreeQuant() : m_min(0), m_max(0) {}
RegexTreeQuant::~RegexTreeQuant() {}
bool RegexTreeQuant::isValid() const {
	return m_max < 0 || (m_min <= m_max);
}
RegexTreeType RegexTreeQuant::getType() const {return REG_TREE_QUANT;}
string RegexTreeQuant::to_string() const {
	if (m_min == 1 && m_max == -1) return "+";
	if (m_min == 0 && m_max == -1) return "*";
	if (m_min == 0 && m_max == 1) return "?";
	if (m_min == m_max) return "{" + std::to_string(m_min) + "}";
	if (m_max == -1) return "{" + std::to_string(m_min) + ",}";
	return "{" + std::to_string(m_min) + "," + std::to_string(m_max) + "}";
}
void RegexTreeQuant::print(int depth) const {
	printPreIndent(string("Quant") + RST + " (" + std::to_string(m_min) + "," + std::to_string(m_max) + ")", depth);
}

// Node class used to represent an atom.
RegexTreeAtom::RegexTreeAtom() : m_node(nullptr), m_quant(nullptr) {}
RegexTreeAtom::~RegexTreeAtom() {
	if (m_node) delete m_node;
	if (m_quant) delete m_quant;
}
RegexTreeType RegexTreeAtom::getType() const {return REG_TREE_ATOM;}
string RegexTreeAtom::to_string() const {
	bool u = m_node->getType() == REG_TREE_UNION;
	return (u ? "(" : "") + m_node->to_string() + (u ? ")" : "") + m_quant->to_string();
}
void RegexTreeAtom::print(int depth) const {
	printPreIndent("Atom", depth);
	if (m_node) m_node->print(depth+1);
	if (m_quant) m_quant->print(depth+1);
	printPostIndent(depth);
}

// Node class used to represent an union.
RegexTreeClass::RegexTreeClass() : m_match(false), m_nodes() {}
RegexTreeClass::~RegexTreeClass() {
	for (RegexTreeNode *node : m_nodes) if (node) delete node;
}
RegexTreeType RegexTreeClass::getType() const {return REG_TREE_CLASS;}
string RegexTreeClass::to_string() const {
	string str = (m_match ? "[" : "[^");
	for (const RegexTreeNode *node : m_nodes) str += node->to_string();
	return str + "]";
}
void RegexTreeClass::print(int depth) const {
	printPreIndent(string("Class") + RST + " (" + string(m_match ? "match" : "unmatch") + ")", depth);
	for (const RegexTreeNode *node : m_nodes) if (node) node->print(depth+1);
	printPostIndent(depth);
}

// Node class used to represent a litteral.
RegexTreeLitt::RegexTreeLitt() : m_esc(false), m_val(0) {}
RegexTreeLitt::~RegexTreeLitt() {}
RegexTreeType RegexTreeLitt::getType() const {return REG_TREE_LITT;}
string RegexTreeLitt::to_string() const {
	if (m_esc) return "\\" + m_val;
	else if (isgraph(m_val)) return string(1,m_val);
	else {
		switch (m_val) {
			case '\n': return "\\n";
			case '\t': return "\\t";
			case '\r': return "\\r";
			case '\v': return "\\v";
			case '\f': return "\\f";
			default: return "\\x" + format("{:02X}", m_val);
		}
	}
}
void RegexTreeLitt::print(int depth) const {
	printPreIndent(
		string("Litt") + RST + " (" + string(m_esc ? "escape, " : "char, ") + 
		(isgraph(m_val) ? "'" + string(1,m_val) + "'" : "0x" + format("{:02X}", m_val)) + ")",
		depth
	);
}

// Node class used to represent a sequence of characters.
RegexTreeSeq::RegexTreeSeq() : m_litt_min(nullptr), m_litt_max(nullptr) {}
RegexTreeSeq::~RegexTreeSeq() {
	if (m_litt_min) delete m_litt_min;
	if (m_litt_max) delete m_litt_max;
}
bool RegexTreeSeq::isValid() const {
	int min_val = m_litt_min->m_val;
	int max_val = m_litt_max->m_val;
	return min_val <= max_val;
}
RegexTreeType RegexTreeSeq::getType() const {return REG_TREE_SEQ;}
string RegexTreeSeq::to_string() const {
	return m_litt_min->to_string() + "-" + m_litt_max->to_string();
}
void RegexTreeSeq::print(int depth) const {
	printPreIndent("Seq", depth);
	if (m_litt_min) m_litt_min->print(depth+1);
	if (m_litt_max) m_litt_max->print(depth+1);
	printPostIndent(depth);
}



// Parser recursive functions.
// Detects if a union stops or not.
bool regexParseUnion1(vector<int>::const_iterator &it, RegexTreeUnion *&node, bool verbose) {
	char type;
	TOKEN_TYPE(*it, type);

	// Checks if it's the end of the recursive union.
	if (verbose) grcc::cout << "searching for a recursive Union " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, ')') ||
		type == REG_TK_EOE || type == REG_TK_LOOKAHEAD) {
		return true;

	// Else checks if the recursion continue.
	} else if (TOKEN_CMP(*it, REG_TK_OP, '|')) {
		++it;
		
		// Parses the next union element.
		if (!regexParseUnion(it, node, verbose)) return false;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '$'), (op: ')'), (op: '|') or EOE was expected, not "
		<< stringFromToken(*it) << endl;
	return false;

}

// Detects if a concatenation stops or not.
bool regexParseConcat1(vector<int>::const_iterator &it, RegexTreeConcat *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Checks if it's the end of the recursive concatenation.
	if (verbose) grcc::cout << "searching for a recursive Concat " << stringFromToken(*it) << ")" << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '|') ||
		TOKEN_CMP(*it, REG_TK_OP, ')') ||
		type == REG_TK_EOE || type == REG_TK_LOOKAHEAD) {
		return true;

	// Else checks if the recursion continue.
	} else if (
		TOKEN_CMP(*it, REG_TK_OP, '(') ||
		TOKEN_CMP(*it, REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {

		// Parses the next concatenation element.
		if (!regexParseConcat(it, node, verbose)) return false;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '$'), (op: '|'), (op: ')'), (op: '('), (op: '['), litteral or EOE was expected, not "
		<< stringFromToken(*it) << endl;
	return false;

}

// Parses a 2 number long quantifier.
bool regexParseQuant1(vector<int>::const_iterator &it, RegexTreeQuant *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Checks if the second number is explicitly given.
	if (verbose) grcc::cout << "searching for the second int in a Quant " << stringFromToken(*it) << endl;
	if (type == REG_TK_INT) {
		TOKEN_VAL(*it, node->m_max);
		++it;
		if (!TOKEN_CMP(*it, REG_TK_OP, '}')) {
			grcc::cerr << "(op: '}') was expected, not " << stringFromToken(*it) << endl;
			return false;
		}
		++it;
		return true;

	// Else there is no upper bound.
	} else if (TOKEN_CMP(*it, REG_TK_OP, '}')) {
		++it;
		node->m_max = -1;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '}') or integer was expected, not " << stringFromToken(*it) << endl;
	return false;

}

// Detects if a quantifier is 1 or 2 number long.
bool regexParseQuant2(vector<int>::const_iterator &it, RegexTreeQuant *&node, bool verbose) {

	// If the quantifier ends here, it's max is it's min.
	if (verbose) grcc::cout << "searching for a Quant length " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '}')) {
		node->m_max = node->m_min;
		++it;
		if (verbose) grcc::cout << "a 1 number quantifier was found " << stringFromToken(*it) << endl;
		return true;

	// Else try to detects the second part of the quantifier.
	} else if (TOKEN_CMP(*it, REG_TK_OP, ',')) {
		++it;
		if (verbose) grcc::cout << "a 2 number quantifier was found " << stringFromToken(*it) << endl;
		if (!regexParseQuant1(it, node, verbose)) return false;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '}') or (op: ',') was expected, not " << stringFromToken(*it) << endl;
	return false;

}

// Detects the structure of a class with optional exclusion operator.
bool regexParseLetter1(vector<int>::const_iterator &it, RegexTreeNode *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Initialises the class tree and checks if there is an exclusion operator.
	if (verbose) grcc::cout << "searching for an exclusion operator in a Class " << stringFromToken(*it) << endl;
	RegexTreeClass *class_node = new RegexTreeClass();
	node = class_node;
	if (TOKEN_CMP(*it, REG_TK_OP, '^')) {
		if (verbose) grcc::cout << "an exclusion operator was detected" << endl;
		++it;
	} else class_node->m_match = true;
	
	// Then ensures that the class isn't empty.
	TOKEN_TYPE(*it, type);
	if (type != REG_TK_CHAR && type != REG_TK_ESC) {
		grcc::cerr << "a litteral was expected, not " << stringFromToken(*it) << endl;
		return false;
	}

	// Parses the class.
	if (!regexParseClass(it, class_node, verbose)) return false;

	// Checks if the class is closed.
	if (!TOKEN_CMP(*it, REG_TK_OP, ']')) {
		grcc::cerr << "a class must end with (op: ']'), not " << stringFromToken(*it) << endl;
		return false;
	}
	++it;
	return true;

}

// Detects a recursive class or the end of a class.
bool regexParseClass1(vector<int>::const_iterator &it, RegexTreeClass *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);
	if (verbose) grcc::cout << "searching for a recursive Class " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, ']')) return true;
	if (type != REG_TK_CHAR && type != REG_TK_ESC) {
		grcc::cerr << "a litteral was expected, not " << stringFromToken(*it) << endl;
		return false;
	}
	if (!regexParseClass(it, node, verbose)) return false;
	return true;
}

// Parses the end of a class and a sequence in a class.
bool regexParseElt1(vector<int>::const_iterator &it, 
	vector<RegexTreeNode*> &nodes, RegexTreeLitt *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Checks if it's the end of the class or the beginning of a new element.
	if (verbose) grcc::cout << "searching for a Seq... " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, ']') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) return true;
	if (!TOKEN_CMP(*it, REG_TK_OP, '-')) {
		grcc::cerr << "'-' expected, not " << stringFromToken(*it) << endl;
		return false;
	}
	++it;

	// Detects a litteral in case of an interval.
	TOKEN_TYPE(*it, type);
	if (type != REG_TK_CHAR) {
		grcc::cerr << "a character was expected, not " << stringFromToken(*it) << endl;
		return false;
	}
	
	// Here we are in a sequence.
	// Detects the second litteral of a sequence.
	if (verbose) grcc::cout << "parsing a Seq " << stringFromToken(*it) << endl;
	RegexTreeLitt *litt_node = new RegexTreeLitt();
	regexParseLitt(it, litt_node, verbose);

	// Updates the last tree node.
	RegexTreeSeq *seq_node = new RegexTreeSeq();
	seq_node->m_litt_min = node;
	seq_node->m_litt_max = litt_node;
	nodes.back() = seq_node;
	return seq_node->isValid();

}



// Parser functions.
// Parses a litteral.
void regexParseLitt(vector<int>::const_iterator &it, RegexTreeLitt *&node, bool verbose) {
	char type, val; TOKEN_SPLIT(*it, type, val);
	if (verbose) grcc::cout << "a litteral was found " << stringFromToken(*it) << endl;
	node->m_esc = (type == REG_TK_ESC);	
	node->m_val = val;
	++it;
}

// Parses an element from a class.
bool regexParseElt(vector<int>::const_iterator &it, vector<RegexTreeNode*> &nodes, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Detects if the element starts with a litteral.
	if (verbose) grcc::cout << "parsing en Elt from a Class " << stringFromToken(*it) << endl;
	if (type != REG_TK_CHAR) {
		grcc::cerr << "a character was expected, not " << stringFromToken(*it) << endl;
		return false;
	}
	RegexTreeLitt *litt_node = new RegexTreeLitt();
	nodes.push_back(litt_node);
	regexParseLitt(it, litt_node, verbose);

	// Parses the second part of the element and the next elements.
	if (!regexParseElt1(it, nodes, litt_node, verbose)) return false;
	return true;

}

// Parses the content of a class.
bool regexParseClass(vector<int>::const_iterator &it, RegexTreeClass *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Checks if the content of the class starts with a character.
	if (verbose) grcc::cout << "parsing a Class " << stringFromToken(*it) << endl;
	if (type != REG_TK_CHAR) {
		grcc::cerr << "a character was expected, not " << stringFromToken(*it) << endl;
		return false;
	}

	// Then parses the content.
	if (!regexParseElt(it, node->m_nodes, verbose)) return false;
	if (!regexParseClass1(it, node, verbose)) return false;
	return true;

}

// Parses a letter, which is either a litteral or a class.
bool regexParseLetter(vector<int>::const_iterator &it, RegexTreeNode *&node, bool verbose) {
	char type;
	TOKEN_TYPE(*it, type);

	// Checks if the letter is a litteral.
	if (verbose) grcc::cout << "parsing a Letter " << stringFromToken(*it) << endl;
	node = new RegexTreeQuant();
	if (type == REG_TK_CHAR || type == REG_TK_ESC) {
		RegexTreeLitt *litt_node = new RegexTreeLitt();
		node = litt_node;
		regexParseLitt(it, litt_node, verbose);
		return true;
	
	// Else checks if the letter is a class.
	} else if (TOKEN_CMP(*it, REG_TK_OP, '[')) {
		++it;
		if (!regexParseLetter1(it, node, verbose)) return false;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '[') or litteral was expected, not " << stringFromToken(*it) << endl;
	return false;

}

// Parses a quantifier.
bool regexParseQuant(vector<int>::const_iterator &it, RegexTreeQuant *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Continues if there might be a quantifier.
	if (verbose) grcc::cout << "parsing a Quant " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '|') ||
		TOKEN_CMP(*it, REG_TK_OP, '(') ||
		TOKEN_CMP(*it, REG_TK_OP, ')') ||
		TOKEN_CMP(*it, REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC || 
		type == REG_TK_EOE || type == REG_TK_LOOKAHEAD) return true;
	
	// Matches the quantifier.
	// + corresponds to at least one : [1..+oo[.
	if (verbose) grcc::cout << "a quantifier was detected" << endl;
	node = new RegexTreeQuant(); 
	if (TOKEN_CMP(*it, REG_TK_OP, '+')) {
		node->m_min = 1;
		node->m_max = -1;
		++it;
		return true;

	// * corresponds to any number : [0..+oo[.
	} else if (TOKEN_CMP(*it, REG_TK_OP, '*')) {
		node->m_min = 0;
		node->m_max = -1;
		++it;
		return true;

	// ? corresponds to 0 or 1 : [0..1].
	} else if (TOKEN_CMP(*it, REG_TK_OP, '?')) {
		node->m_min = 0;
		node->m_max = 1;
		++it;
		return true;
	
	// Else the quantifier is {something}.
	} else if (TOKEN_CMP(*it, REG_TK_OP, '{')) {
		++it;
		TOKEN_TYPE(*it, type);
		if (type == REG_TK_INT) {
			TOKEN_VAL(*it, node->m_min);
			++it;
			if (!regexParseQuant2(it, node, verbose)) return false;
			return true;
		}
	
	}

	// Error detected.
	grcc::cerr << "(op: '+'), (op: '*'), (op: '?') or (op: '{') was expected, not " 
		<< stringFromToken(*it) << endl;
	return false;

}

// Parses an atom, which is a letter associated with a quantifier.
bool regexParseAtom(vector<int>::const_iterator &it, RegexTreeAtom *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Checks if the atom is a letter.
	if (verbose) grcc::cout << "parsing an Atom " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		if (!regexParseLetter(it, node->m_node, verbose)) return false;

	// Checks if the atom is a parenthesis containing something.
	} else if (TOKEN_CMP(*it, REG_TK_OP, '(')) {
		++it;
		RegexTreeUnion *union_node = new RegexTreeUnion();
		node->m_node = union_node;
		if (!regexParseUnion(it, union_node, verbose)) return false;

		// Delete the union node if it contains only one element.
		if (union_node->m_nodes.size() == 1) {
			node->m_node = union_node->m_nodes[0];
			union_node->m_nodes.clear();
			delete union_node;
		}

		// Detects the closing parenthesis.
		if (!TOKEN_CMP(*it, REG_TK_OP, ')')) {
			grcc::cerr << "(op: '(') was expected, not " << stringFromToken(*it) << endl;
			return false;
		}
		++it;
	
	// Error detected.
	} else {
		grcc::cerr << "(op: '('), (op: '[') or a litteral was expected, not " 
			<< stringFromToken(*it) << endl;
		return false;
	}

	// Parse the quantifier if there is one.
	if (!regexParseQuant(it, node->m_quant, verbose)) return false;
	if (node->m_quant && !node->m_quant->isValid()) {
		grcc::cerr << "quantifier upper bound can't be lower than lower bound" << endl;
		it-=2;
		return false;
	}
	return true;

}

// Parses a concatenation of elements.
bool regexParseConcat(vector<int>::const_iterator &it, RegexTreeConcat *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);
	
	// Parses the first element of the concatenation, then try to detect a recursive concatenation.
	if (verbose) grcc::cout << "parsing a Concat " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '(') ||
		TOKEN_CMP(*it, REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		RegexTreeAtom* atom_node = new RegexTreeAtom();
		node->m_nodes.push_back(atom_node);
		if (!regexParseAtom(it, atom_node, verbose)) return false;
		
		// Delete the atom node if it contains only one element.
		if (!atom_node->m_quant) {
			node->m_nodes.back() = atom_node->m_node;
			atom_node->m_node = nullptr;
			delete atom_node;
		}

		// Fuse the concatenation with it's child if it's also a concatenation.
		if (node->m_nodes.back()->getType() == REG_TREE_CONCAT) {
			RegexTreeConcat *concat_node = dynamic_cast<RegexTreeConcat*>(node->m_nodes.back());
			node->m_nodes.pop_back();
			node->m_nodes.insert(node->m_nodes.end(), concat_node->m_nodes.begin(), concat_node->m_nodes.end());
			concat_node->m_nodes.clear();
			delete concat_node;
		}

		// Then parse the end of the concatenation.
		if (!regexParseConcat1(it, node, verbose)) return false;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '('), (op: '[') or a litteral was expected, not " << stringFromToken(*it) << endl;
	return false;

}

// Parses a union of elements.
bool regexParseUnion(vector<int>::const_iterator &it, RegexTreeUnion *&node, bool verbose) {
	char type; TOKEN_TYPE(*it, type);

	// Parses the first element of the union, then try to detect a recursive union.
	if (verbose) grcc::cout << "parsing an Union " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '(') ||
		TOKEN_CMP(*it, REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		RegexTreeConcat *concat_node = new RegexTreeConcat();
		node->m_nodes.push_back(concat_node);
		if (!regexParseConcat(it, concat_node, verbose)) return false;

		// Delete the concatenation node if it contains only one element.
		if (concat_node->m_nodes.size() == 1) {
			node->m_nodes.back() = concat_node->m_nodes[0];
			concat_node->m_nodes.clear();
			delete concat_node;
		}

		// Fuse the union with it's child if it's also a union.
		if (node->m_nodes.back()->getType() == REG_TREE_UNION) {
			RegexTreeUnion *union_node = dynamic_cast<RegexTreeUnion*>(node->m_nodes.back());
			node->m_nodes.pop_back();
			node->m_nodes.insert(node->m_nodes.end(), union_node->m_nodes.begin(), union_node->m_nodes.end());
			union_node->m_nodes.clear();
			delete union_node;
		}

		// The parse the end of the union.
		if (!regexParseUnion1(it, node, verbose)) return false;
		return true;
	}

	// Error detected.
	grcc::cerr << "(op: '('), (op: '[') or a litteral was expected, not " << stringFromToken(*it) << endl;
	return false;

}

// Parses the ending lookahead.
bool regexParseLook(vector<int>::const_iterator &it, RegexTreeNode *&node, bool &match, bool verbose) {
	char type, val; TOKEN_SPLIT(*it, type, val);
	
	// If it's the end of the regex, return.
	if (verbose) grcc::cout << "parsing a lookahead " << stringFromToken(*it) << endl;
	if (type == REG_TK_EOE) return true;
	else if (type == REG_TK_LOOKAHEAD) {
		match = val == '=';
		++it;

		// Parse the content of the lookahead.
		if (verbose) grcc::cout << "a lookahead was found " << stringFromToken(*it) << endl;
		RegexTreeUnion *union_node = new RegexTreeUnion();
		node = union_node;
		if (!regexParseUnion(it, union_node, verbose)) return false;

		// Delete the union node if it contains only one element.
		if (union_node->m_nodes.size() == 1) {
			node = union_node->m_nodes[0];
			union_node->m_nodes.clear();
			delete union_node;
		}

		// Finlly, parse the closing parenthesis.
		if (!TOKEN_CMP(*it, REG_TK_OP, ')')) {
			grcc::cerr << "(op: ')') was expected, not " << stringFromToken(*it) << endl;
			return false;
		}
		++it;
		return true;

	}

	// Error detected.
	grcc::cerr << "lookahead or EOE was expected, not " << stringFromToken(*it) << endl;
	return false;

}

// Parses a regex and returns the corresponding AST.
bool regexParseExpr(vector<int>::const_iterator &it, RegexTreeExpr &tree, bool verbose) {
	char type; TOKEN_TYPE(*it, type);
	if (type == REG_TK_EOE) {
		tree = RegexTreeExpr();
		return true;
	}

	// Detects the ending operator.
	if (verbose) grcc::cout << "parsing an Expr " << stringFromToken(*it) << endl;
	if (TOKEN_CMP(*it, REG_TK_OP, '^') ||
		TOKEN_CMP(*it, REG_TK_OP, '(') ||
		TOKEN_CMP(*it, REG_TK_OP, '[') ||
		type == REG_TK_CHAR || type == REG_TK_ESC) {
		tree = RegexTreeExpr();
		RegexTreeUnion *union_node = new RegexTreeUnion();
		tree.m_node = union_node;
		if (!regexParseUnion(it, union_node, verbose) ||
			!regexParseLook(it, tree.m_look, tree.m_match, verbose)) {
			grcc::cerr << "unable to parse the regex" << endl;
			return false;
		}

		// Delete the union node if it contains only one element.
		if (union_node->m_nodes.size() == 1) {
			tree.m_node = union_node->m_nodes[0];
			union_node->m_nodes.clear();
			delete union_node;
		}

		// At this point the regex is valid if we found EOE.
		if (TOKEN_CMP(*it, REG_TK_EOE, 0)) return true;
		grcc::cerr << "EOE was expected, not " << stringFromToken(*it) << endl;
		grcc::cerr << "unable to parse the regex" << endl;
		return false;

	}

	// Error detected.
	grcc::cerr << "(op: '^'), (op: '('), (op: '[') or a litteral was expected, not " << stringFromToken(*it) << endl;
	grcc::cerr << "unable to parse the regex" << endl;
	return false;

}



// This function build the AST of a given regex in str.
// verbose activate the verbose mode, which prints a lot of additional
// informations regarding the parsing process.
bool regexParser(RegexTreeExpr &tree, const string &str, bool verbose) {

	// Gets the list of tokens from the expression string.
	vector<int> tokens;
	if (!regexLexer(str, tokens)) {
		int pos; TOKEN_POS(tokens.back(), pos);
		grcc::cerr << "the lexer was unable to treat the regex :" << endl;
		grcc::cerr << str << endl;
		grcc::cerr << BOLD << (string(pos+1, ' ').replace(pos,1,"^")) << "- here's the issue" << RST << endl;
		return false;
	}

	// Prints the token list in case of verbose mode.
	if (verbose) {
		grcc::cout << BOLD << "Lexical analysis" << RST << endl;
		grcc::cout << "detected tokens : " ;
		for (size_t i = 0; i < tokens.size(); ++i) {
			grcc::cout() << stringFromToken(tokens[i]);
			if (i < tokens.size()-1) grcc::cout() << ", ";
		}
		grcc::cout() << endl;
		grcc::cout << BOLD << "Syntax analysis" << RST << endl;
	}

	// Finally, parses the tree and prints it in verbose mode.
	vector<int>::const_iterator it = tokens.begin();
	if (!regexParseExpr(it, tree, verbose)) {
		int pos; TOKEN_POS(*it, pos);
		grcc::cerr << "the parser was unable to treat the regex :" << endl;
		grcc::cerr << str << endl;
		grcc::cerr << BOLD << (string(pos+1, ' ').replace(pos,1,"^")) << "- here's the issue" << RST << endl;
		return false;
	}

	// Prints the generated AST in case of verbose mode.
	if (verbose) {
		grcc::cout << "generated the following AST :" << endl;
		tree.print();
	}
	return true;

}
