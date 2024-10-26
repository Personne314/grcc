#include <iostream>
#include <fstream>
#include <cassert>

#include "../src/regex.h"


using std::cout, std::cerr, std::endl;


// This is used to test regex.h/regex.cpp code.
int main() {
	
	// Tests expressions that should be working.
	RegexTreeExpr tree;
	assert(regexParser(tree, "m"));
	assert(!tree.to_string().compare("m"));
	assert(regexParser(tree, "^[^^](?=a)"));
	assert(!tree.to_string().compare("^[^^](?=a)"));
	assert(regexParser(tree, "a{4,9}b{4,}"));
	assert(!tree.to_string().compare("a{4,9}b{4,}"));
	assert(regexParser(tree, "a|b"));
	assert(!tree.to_string().compare("a|b"));
	assert(regexParser(tree, "a+b*c?"));
	assert(!tree.to_string().compare("a+b*c?"));
	assert(regexParser(tree, "a{0,1}"));
	assert(!tree.to_string().compare("a?"));
	assert(regexParser(tree, "[ae-g]{5}"));
	assert(!tree.to_string().compare("[ae-g]{5}"));
	assert(regexParser(tree, "[a\\n-g]"));
	assert(!tree.to_string().compare("[a\\n-g]"));
	assert(regexParser(tree, "ab(cd)"));
	assert(!tree.to_string().compare("abcd"));
	assert(regexParser(tree, "a|b|(c|d)"));
	assert(!tree.to_string().compare("a|b|c|d"));

	// Tests incorrect expressions. The error messages are expected
	// and so discarded to not pollute the terminal.
	std::streambuf* buf = cerr.rdbuf();
	std::ofstream null_str("dev/null");
	cerr.rdbuf(null_str.rdbuf());
	assert(!regexParser(tree, "^[^^](?=a)m"));
	assert(!regexParser(tree, "[ae-\\n]"));
	assert(!regexParser(tree, "[ae-\\w]"));
	assert(!regexParser(tree, "[a\\w-g]"));
	assert(!regexParser(tree, "\\m"));
	cerr.rdbuf(buf);

	cout << "[\e[0;32m OK \e[0;30m] test_regex" << endl;

}
