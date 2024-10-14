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





// Calculate firsts list of a sequence of symbols.
void vectorFirst(const std::vector<bool> &eps, const std::vector<std::set<int>> &first,
	std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end, std::set<int> &res);

// Calculate the rules to delete in case of an inferior reduce.
void inferiorReduce(const std::vector<bool> &terminals,const std::vector<Rule> &rules,
	std::vector<bool> &inf_symbols);

// Calculate the rules to delete in case of a superior reduce.
void superiorReduce(int axiom, const std::vector<bool> &terminals, const std::vector<Rule> &rules, 
	std::vector<bool> &sup_symbols);

// Delete immadiate left recursion.
void derec_immadiate(const std::string &name, std::vector<std::set<Rule>>& rules, int rules_id,
	std::vector<std::string>& res_symbols, std::vector<bool>& res_terminals);

// Replace all rules Ai -> Aj b, where Aj -> a1 | ... | an, by Ai -> a1 b | ... | an b
// for all Aj < Ai.
void derec_update_rules(const std::vector<std::set<Rule>> &rules,
	const std::vector<bool> &terminals, std::set<Rule> &res_rules);

// Return the length of the longest common prefix of at least two of the 
// derivate part of the rules contained in words.
int commonPrefix(const std::vector<Rule> &words, std::vector<bool> &eq_class);
