#include "grammar.h"
#include "regex.h"

#include <iostream>
#include <filesystem>

#include "constants.h"


using std::filesystem::current_path, std::filesystem::absolute, std::filesystem::path;


int main(int argc, char *argv[]) {
	(void)argc;
	current_path(absolute(path(argv[0])).remove_filename());
/*
	regexParser("^^a*|b+e{0}e{0,}e{0,4}|c?[^^\\na-b]$$");
	regexParser("a?b(c|d+)*", true);
*/

	Grammar grammar("../../grammar_regex.txt");
	grammar.reduce();
	grammar.derec();
	grammar.fact();
	grammar.print();

	LLkTable table = grammar.LLk();
	table.print(grammar);

}
