#include <iostream>
#include <filesystem>
#include <cassert>
#include <vector>
#include <algorithm>

#include "../src/grammar.h"


using std::filesystem::current_path, std::filesystem::absolute, std::filesystem::path;
using std::vector, std::set, std::find, std::next;
using std::cout, std::cerr, std::endl;


// This is used to test the grammar.h/grammar.cpp code.
int main(int argc, char *argv[]) {
	(void)argc;
	current_path(absolute(path(argv[0])).remove_filename());

	// Test of vectorFirst.
	// Typical use case.
	vector<bool> first_eps = {1,1,0,0,0,0,0,0};
	vector<set<int>> first_firsts = {{4},{5,4},{6},{7},{4},{5},{6},{7}};
	vector<int> first_word = {0,1,0,7,3};
	set<int> first_res;
	vectorFirst(first_eps, first_firsts, first_word.begin(), first_word.end(), first_res);
	assert(*first_res.begin() == 4);
	assert(*next(first_res.begin()) == 5);
	assert(*next(first_res.begin(),2) == 7);

	// Case where the word can be derivated into epsilon.
	first_word = {0};
	first_res.clear();
	vectorFirst(first_eps, first_firsts, first_word.begin(), first_word.end(), first_res);
	assert(*first_res.begin() == 4);

	// Case of an empty word.
	first_word = {};
	first_res.clear();
	vectorFirst(first_eps, first_firsts, first_word.begin(), first_word.end(), first_res);
	assert(first_res.size() == 0);



	// Test of reduction operation.
	// The grammar used for this test will lose non-terminals during the inferior and
	// superior reduce, ensuring that both of these algorithm work properly.
	Grammar red_grammar("./grammar_reduce.txt");
	if (!red_grammar.getInitState()) {
		cerr << "\e[0;31merror:\e[0;0m unable to find grammar_reduce.txt" << endl;
		return -1;
	}
	red_grammar.reduce();
	auto &red_symbols = red_grammar.getSymbols();

	// B, X, Y and Z are supposed to be the only removed non-terminals.
	assert(find(red_symbols.begin(), red_symbols.end(), "A") != red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "V") != red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "U") != red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "B") == red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "C") != red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "W") != red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "X") == red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "Y") == red_symbols.end());
	assert(find(red_symbols.begin(), red_symbols.end(), "Z") == red_symbols.end());



	// Test of commonPrefix.
	// Typical use case.
	vector<Rule> common_words = {
		Rule(0, {0,1,1,2,4}),
		Rule(0, {0,1,4,2,4}),
		Rule(0, {0,1,4,2,5}),
		Rule(0, {0,1}),
		Rule(0, {1,2,5}),
	};
	vector<bool> common_eq_class;
	int common_len = commonPrefix(common_words, common_eq_class);
	assert(common_len == 4);
	assert(!common_eq_class[0]);
	assert( common_eq_class[1]);
	assert( common_eq_class[2]);
	assert(!common_eq_class[3]);
	assert(!common_eq_class[4]);

	// Case of two rule (1 is empty).
	common_words = {
		Rule(0, {0,1,1,2,4}),
		Rule(0, {})
	};
	common_len = commonPrefix(common_words, common_eq_class);
	assert(common_len == 0);

	// Case of only one rule.
	common_words = {Rule(0, {0,1,1,2,4})};
	common_len = commonPrefix(common_words, common_eq_class);
	assert(common_len == 0);

	// Case of no rules.
	common_words = {};
	common_len = commonPrefix(common_words, common_eq_class);
	assert(common_len == 0);



	// Test the derec operation.
	Grammar derec_grammar("./grammar_derec.txt");
	if (!derec_grammar.getInitState()) {
		cerr << "\e[0;31merror:\e[0;0m unable to find grammar_derec.txt" << endl;
		return -1;
	}
	derec_grammar.derec();
	//derec_grammar.print();
	/*
	Here we can verify that the print shows us the rules we're expecting. 
	A -> B a
	A -> C b
	B -> d B_rec
	B -> C b c B_rec
	C -> f C_rec
	C -> d B_rec a e C_rec
	B_rec -> 
	B_rec -> a c B_rec
	C_rec -> 
	C_rec -> b e C_rec
	C_rec -> b c B_rec a e C_rec
	*/

	// Test the fact operation.
	Grammar fact_grammar("./grammar_fact.txt");
	if (!fact_grammar.getInitState()) {
		cerr << "\e[0;31merror:\e[0;0m unable to find grammar_fact.txt" << endl;
		return -1;
	}
	fact_grammar.fact();
	//fact_grammar.print();
	/*
	Here we can verify that the print shows us the rules we're expecting. 
	X -> a X_2
	Y -> b c Y_1
	Z -> c d
	X_1 -> 
	X_1 -> d Z
	X_2 -> 
	X_2 -> Y b X X_1
	Y_1 -> a
	Y_1 -> Z
	*/

	cout << "[\e[0;32m OK \e[0;30m] test_grammar" << endl;

}
