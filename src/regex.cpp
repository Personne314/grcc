#include "regex.h"

#include <iostream>
#include <cctype>


using std::string, std::vector, std::cerr, std::endl;


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










