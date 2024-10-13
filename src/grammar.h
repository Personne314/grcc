#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>



// Struct used to modelize grammar rules such as :
// symbol -> derivate.
struct Rule {

	// Constructors.
	Rule();
	Rule(int s, const std::vector<int> &d);

	// Print functions.
	void print(const std::vector<std::string> &symbols,
		std::ostream &str = std::cout) const;

	// Operator overload.
	bool operator<(const Rule &rule) const;
	bool operator==(const Rule &rule) const;

	// Rules data.
	int symbol;
	std::vector<int> derivate;

};





// Class used to modelize LLk tables.
class Grammar;
class LLkTable {
public: 

	// Constructor, destructor.
	LLkTable(const std::vector<bool> &terminals);
	~LLkTable();

	// Getters.
	const std::set<int> &operator[](int nt, int t) const;
	bool isLL1() const;

	// Print function.
	void print(const Grammar &grammar, std::ostream &str = std::cout) const;

	// Gives access to the Grammar class.
	friend class Grammar;

protected:

	// Functions used to add rules.
	void addRule(int nt, int t, int rule);

private:

	// LLk table.
	std::vector<std::set<int>> m_data;

	// Line size.
	int m_rows;

	// Lookup tables.
	std::vector<int> m_nonterminals;
	std::vector<int> m_terminals;

};



// Class used to load and manipulate a context-free grammar defined in a file.
class Grammar {
public:

	// Constructor, destructor.
	Grammar(const std::string &path);
	~Grammar();

	// Print function.
	void print(std::ostream &str = std::cout) const;

	// Grammar operations.
	void reduce();
	void derec();
	void fact();

	// Functions used to calculate epsilon-derivable symbols
	// indicator, first and following symbols lists.
	void epsilon(std::vector<bool> &eps) const;
	void first(
		const std::vector<bool> &eps, 
		std::vector<std::set<int>> &first) const;
	void follow(
		const std::vector<bool> &eps, 
		const std::vector<std::set<int>> &first,
		std::vector<std::set<int>> &follow) const;

	// Calculate the LLk table.
	LLkTable LLk() const;

	// Return initialization state.
	bool getInitState() const;

	// Getters.
	const std::vector<bool> &isTerminals() const;
	const std::vector<std::string> &getSymbols() const;
	const std::vector<Rule> &getRules() const;

private:

	// Delete a symbol and all rules involving that symbol.
	void removeSymbols(const std::vector<bool> &symbols);

	// initialization state.
	bool m_initState;

	// Grammar name.
	std::string m_name;

	// Grammar axiom.
	int m_axiom;

	// Symbols vector, rules vector, and terminal indicator vector.
	std::vector<std::string> m_symbols;
	std::vector<bool> m_terminals;
	std::vector<Rule> m_rules;

};
