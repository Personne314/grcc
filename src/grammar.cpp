#include "grammar.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <numeric>
#include <set>



// Usings.
using std::vector, std::string, std::stringstream, std::set, std::unordered_map;
using std::cout, std::cerr, std::endl;



// Grammar parser states.
enum GrammarParserState {
	GPS_NONE,
	GPS_NAME,
	GPS_TERM,
	GPS_NTERM,
	GPS_AXIOM,
	GPS_RULES
};



// Calculate firsts lisr of a sequence of symbols.
// eps : epsilon-derivable symbols indicator.
// first : firsts lists of symbols.
// begin, end : boundaries of the sequence.
// res : results as first symbols vector.
void vectorFirst(const vector<bool> &eps, const vector<set<int>> &first,
	vector<int>::const_iterator begin, vector<int>::const_iterator end, set<int> &res) {

	// No firsts if sequence is empty.
	if (end-begin <= 0) return;

	// Add iterativelly the firsts.
	bool all_eps = true;
	for (auto it = begin; it < end && all_eps; ++it) {
		res.insert(first[*it].begin(), first[*it].end());
		all_eps &= eps[*it];
	}

}

// Calculate the rules to delete in case of an inferior reduce.
// terminals : terminal symbol indicator.
// rules : to-be-reduced grammar rules.
// inf_rules : restult as a to-be-removed indicator.
void inferiorReduce(const vector<bool> &terminals,const vector<Rule> &rules,
	vector<bool> &inf_symbols) {
	inf_symbols = vector<bool>(terminals);

	// Run until we find a stable state.
	bool changed = true;
	while (changed) {
		changed = false;	

		// For each rule where initial non-terminal isn't in inf_symbols.
		for (const Rule& rule : rules) {
			if (inf_symbols[rule.symbol]) continue;

			// Check if it must be added.
			bool res = true;
			for (int symbol : rule.derivate) res &= inf_symbols[symbol];

			// Then update the result.
			if (res) {
				changed = true;
				inf_symbols[rule.symbol] = true;
			}

		}

	}

	// Calculate the to-be-removed symbols.
	for (auto it = inf_symbols.begin(); it != inf_symbols.end(); ++it) *it = !*it;

}

// Calculate the rules to delete in case of a superior reduce.
// axiom : to-be-reduced grammar axiom.
// terminals : to-be-reduced grammar terminals indicator.
// rules : to-be-reduced grammar rules.
// inf_rules : restult as a to-be-removed indicator.
void superiorReduce(int axiom, const vector<bool> &terminals,
	const vector<Rule> &rules, vector<bool> &sup_symbols) {
	sup_symbols = vector<bool>(terminals);

	// Add the axiom to the indicator.
	sup_symbols[axiom] = true;

	// Run until we find a stable state.
	bool changed = true;
	while (changed) {
		changed = false;	

		// For each rule eccessible from the axiom.
		for (const Rule &rule : rules) {
			if (!sup_symbols[rule.symbol]) continue;

			// Check if it's accessible from the axiom.
			for (int symbol : rule.derivate) {
				if (sup_symbols[symbol]) continue;
				changed = true;
				sup_symbols[symbol] = true;
			}

		}

	}

	// Calculate the to-be-removed symbols.
	for (auto it = sup_symbols.begin(); it != sup_symbols.end(); ++it) *it = !*it;

}

// Delete immadiate left recursion.
// name : name of the starting symbols of the rules.
// rules : rules ordered by initial symbol. Contained sets 
// are ordered by increasing derivation length.
// rules_id : id of the rules set to process.  
// res_symbols, res_terminals : references for return values.
void derec_immadiate(const string &name, vector<set<Rule>>& rules, int rules_id,
	vector<string>& res_symbols, vector<bool>& res_terminals) {

	// Copy the rules to process.
	set<Rule> current_rules(rules[rules_id]);

	// Check if the first rule derivate in espilon an if its the case, remove it.
	// We are sure that there is only one such rule max, because we works with a set.
	const bool epsilon = !current_rules.begin()->derivate.size();
	const auto current_begin = (epsilon) ? ++current_rules.begin() : current_rules.begin();
	const auto current_end = current_rules.end();

	// Function end if there is no rules given.
	const int size = current_rules.size() - 1*epsilon;
	if (size <= 0) return;

	// Common initial symbol and new id in case we need to add a new symbol.
	const int symbol = current_rules.begin()->symbol;
	const int new_id = res_symbols.size(); 

	// Detect immediate left recursive rules.
	bool is_rec = false;
	vector<bool> rec_rules;
	for (auto it = current_begin; it != current_end; ++it) {
		rec_rules.push_back(it->derivate[0] == symbol);
		is_rec |= rec_rules.back();
	}
	if (!is_rec) return;

	// Clear the old rules.
	rules[rules_id].clear();

	// Add a new non-terminal to derecursivate the rules.
	set<Rule> new_rules;
	res_symbols.push_back(name + "_bis");
	res_terminals.push_back(0);

	// For each rule.
	auto it = current_begin;
	for (int i = 0; i < size; ++i) {
		const Rule &rule = *(it++);
		
		// Remplace rules A -> A a by A' -> a A' where A is the initial
		// symbol and A' the new symbol.
		if (rec_rules[i]) {
			Rule new_rule = Rule(new_id, {});
			auto &derivate = new_rule.derivate;
			derivate.insert(
				derivate.end(), 
				rule.derivate.begin() + 1, rule.derivate.end()
			);
			derivate.push_back(new_id);
			new_rules.insert(new_rule);
		
		// Remplace rules A -> b by A -> b A' where A is the initial
		// symbol and A' the new symbol.
		} else {
			Rule new_rule(rule);
			new_rule.derivate.push_back(new_id);
			rules[rules_id].insert(new_rule);
		}

	}

	// Add rule A' -> where A' is the new symbol.
	new_rules.insert(Rule(new_id, {}));
	rules.push_back(new_rules);

}

// Replace all rules Ai -> Aj b, where Aj -> a1 | ... | an, by Ai -> a1 b | ... | an b
// for all Aj < Ai.
// rules : grammar rules.
// terminals : terminal indicator.
// res_rules : reference to an element of rules containing rules to process.
void derec_update_rules(const vector<set<Rule>> &rules,
	const vector<bool> &terminals, set<Rule> &res_rules) {

	// Copy the rules and clear res_rules.
	set<Rule> current_rules(res_rules);
	res_rules.clear();

	// While we're not in a stable state, loop over the rules.
	bool changed;
	do {
		changed = false;
		for (const Rule &current_rule : current_rules) {
			const vector<int> &deriv = current_rule.derivate;
			const int symbol = current_rule.symbol;

			// If rule isn't Ai -> Aj b with i > j, then add it and continue.
			if (!deriv.size() || terminals[deriv[0]] || deriv[0] >= symbol) {
				res_rules.insert(current_rule);
				continue;
			}

			// Else for each rule Aj. Aj -> a1 | ... | an.
			for (const Rule &rule : rules[deriv[0]]) {
				changed = true;

				// Replace Ai -> Aj b by Ai -> ak b. 
				vector<int> new_derivate(rule.derivate);
				new_derivate.insert(
					new_derivate.end(), 
					deriv.begin()+1, deriv.end()
				);

				// Then add it to the result.
				res_rules.insert(Rule(symbol, new_derivate));

			}
		}
		current_rules = res_rules;

	// Loop until we found a stable state.
	} while (changed);

}

// Return the length of the longest common prefix of at least two of the 
// derivate part of the rules contained in words.
// words : Contains the rules that you want to calculate the longest common prefix.
// eq_class : contains true for the rules that starts with the common prefix.
int commonPrefix(const vector<Rule> &words, vector<bool> &eq_class) {
	const int n = words.size();
	if (!n) return 0;

	// Initialise the length of the prefix, and the result equivalence class
	// containing the rules starting with the prefix.
	std::size_t len = -1;
	eq_class = vector<bool>(n, true);

	// Initialise the number of classes and the vector containing the class
	// of each rule.
	int nb_classes = 1;
	vector<int> classes(n, 0);

	// While there is still classes that can be found, try to add one letter.
	while (nb_classes > 0) {
		++len;

		// Update the previous result class.
		for (int i = 0; i < n; ++i) {
			eq_class[i] = classes[i] == 0;
		}

		// Put the rules ids in sets depending of the current classes.
		vector<set<int>> old_classes(nb_classes, set<int>());
		for (int i = 0; i < classes[i]; ++i) {
			if (classes[i] < 0) continue;
			old_classes[classes[i]].insert(i);
		}
		nb_classes = 0;

		// For each class, and each id in these classes.
		for (const set<int> &class_set : old_classes) {
			unordered_map<int, int> new_classes;
			for (int id : class_set) {

				// If the id words is too short, remove it from each classes.
				if (words[id].derivate.size() <= len) classes[id] = -1;
				else {

					// Else if the word isn't equivalent to any of the
					// previous words, add a new class.
					if (new_classes.find(words[id].derivate[id]) == new_classes.end()) {
						new_classes[words[id].derivate[id]] = nb_classes + new_classes.size();
						++nb_classes;
					}

					// Then, add the word to it's correct class.
					classes[id] = new_classes[words[id].derivate[id]];

				}

			}
		}

		// After updating the classes, find the classes containing
		// less than two elements to remove it.
		int j = 0;
		vector<int> count(nb_classes, 0);
		for (int c : classes) ++count[c];
		for (int &nb : count) {
			if (nb > 1) nb = j++;
			else nb = -1;
		}
		nb_classes = j;

		// Then change the ids of the classes to remove the useless ones.
		for (int &c : classes) {
			if (c < 0) continue;
			c = count[c];
		}

	}

	// Return the size of the longest prefix found.
	return len;

}



// Constructor.
Rule::Rule() : symbol(-1), derivate() {};

// Constructor.
Rule::Rule(int s, const vector<int> &d) : symbol(s), derivate(d) {};


// Printing function.
void Rule::print(const vector<string> &symbols, std::ostream &str) const {
	str << symbols[symbol] << " -> ";
	for (std::size_t symbol = 0; symbol < derivate.size(); ++symbol) {
		str << symbols[derivate[symbol]];
		if (symbol != derivate.size()-1) str << " ";
	}
	str << endl;
}

// Comparison operator.
// A < B if A.symbol < B.symbol or, in case of equality,
// if A.derivate is shorter than B.derivate.
bool Rule::operator<(const Rule &rule) const {
	if (symbol != rule.symbol) return symbol < rule.symbol;
	if (derivate.size() != rule.derivate.size()) 
		return derivate.size() < rule.derivate.size();
	for (std::size_t i = 0; i < derivate.size(); ++i) {
		if (derivate[i] != rule.derivate[i])
			return derivate[i] < rule.derivate[i];
	}
	return false;
}

// Equality operator.
bool Rule::operator==(const Rule &rule) const {
	if (symbol != rule.symbol) return false;
	if (derivate.size() != rule.derivate.size()) return false;
	for (std::size_t i = 0; i < derivate.size(); ++i) {
		if (derivate[i] != rule.derivate[i]) return false;
	}
	return true;
}



// Constructor.
LLkTable::LLkTable(const vector<bool> &terminals) : 
m_nonterminals(terminals.size(), -1), m_terminals(terminals.size()+1, -1) {

	// Initialize lookups tables.
	int t = 1, nt = 0;
	m_terminals[0] = 0;
	for (std::size_t i = 0; i < terminals.size(); ++i) {
		if (terminals[i]) {
			m_terminals[i+1] = t; ++t;
		} else {
			m_nonterminals[i] = nt; ++nt;
		}
	}

	// Initialize table data.
	m_rows = t+1;
	m_data = vector<set<int>>(
		nt*m_rows, set<int>()
	);

}

// Destructorr.
LLkTable::~LLkTable() {}


// Access operator.
// -1 is $ (end of word), all other ids correspond to the grammar ones.
const set<int>& LLkTable::operator[](int nt, int t) const {
	return m_data[m_nonterminals[nt]*m_rows+m_terminals[t+1]];
}


// Return true if the table is LL1.
bool LLkTable::isLL1() const {
	for (const set<int>& rules : m_data) {
		if (rules.size() > 1) return false;
	}
	return true;
}



// Table printing function.
void LLkTable::print(const Grammar &grammar, std::ostream &str) const {
	const vector<string> &symbols = grammar.getSymbols();
	const vector<Rule> &rules = grammar.getRules();
	str << "[LLkTable]" << endl;

	// For each non-terminal, print it.
	for (std::size_t i = 0; i < m_nonterminals.size(); ++i) {
		if (m_nonterminals[i] < 0) continue;
		str << symbols[i] << endl;

		// For each terminal, if a rule exists in the table.
		for (std::size_t j = 0; j < m_terminals.size(); ++j) {
			if (m_terminals[j] < 0) continue;

			// Get the rules.
			const set<int>& current_rules = (*this)[i,j-1];
			if (current_rules.size() == 0) continue;

			// Print the terminal.
			if (j == 0) str << "\\--> $ :" << endl;
			else str << "\\--> " << symbols[j-1] << " :" << endl;

			// Print the rules.
			for (int id : current_rules) {
				str << "     \\--> ";
				rules[id].print(symbols, str);
			}

		}
		str << endl;
	}

}

// Function to add a rule in the table.
void LLkTable::addRule(int nt, int t, int rule) {
	m_data[m_nonterminals[nt]*m_rows+m_terminals[t+1]].insert(rule);	
}



// Constructor.
// path : grammar file path.
Grammar::Grammar(const string &path) : m_initState(false),
m_symbols(), m_terminals(), m_rules() {
	
	// Open grammar file.
	std::ifstream file(path);
	if (!file.is_open()) return;

	// For each line that are not comment.
	string line;
	unordered_map<string, int> lookup;
	GrammarParserState state = GPS_NONE;
	while (std::getline(file, line)) {
		if (line.size() == 0 || line[0] == '#') continue;

		// Treat the command lines.
		if (line[0] == '%') {
			if (!line.compare("%name")) state = GPS_NAME;
			else if (!line.compare("%terminals")) state = GPS_TERM;
			else if (!line.compare("%nonterminals")) state = GPS_NTERM;
			else if (!line.compare("%axiom")) state = GPS_AXIOM;
			else if (!line.compare("%rules")) state = GPS_RULES;
			else {
				cerr << "grcc: \e[0;31merror:\e[0;0m line \"" << line 
					 << "\" in grammar file isn't recognized" << endl;
				return;
			}

		// Else treat the line depending of the current command.
		} else {
			Rule rule;
			string word;
			stringstream ss;
			vector<string> words;
			unordered_map<string, int>::iterator it;

			// Switch over the parser state.
			const auto end = m_symbols.end();
			const auto begin = m_symbols.begin();
			switch (state) {

			// In the case of a name, use the whole line as a name,
			// then reset the state.
			case GPS_NAME:
				m_name = line;
				state = GPS_NONE;
				break;
				
			// In case of a terminal, test it's unicity and that it's not ->.
			case GPS_TERM:
				if (std::find(begin, end, line) != end
					|| !line.compare("->")) {
						cerr << "grcc: \e[0;31merror:\e[0;0m \"" << line 
							 << "\" can't be used as a terminal name in"
							 << " the grammar" << endl;
						return;
				}
				lookup[line] = lookup.size();
				m_symbols.push_back(line);
				m_terminals.push_back(1);
				break;

			// In case of a non-terminal, test it's unicity and that it's not ->.
			case GPS_NTERM:
				if (std::find(begin, end, line) != end
					|| !line.compare("->")) {
						cerr << "grcc: \e[0;31merror:\e[0;0m \"" << line 
							 << "\" can't be used as a non-terminal name in"
							 << " the grammar" << endl;
						return;
				}
				lookup[line] = lookup.size();
				m_symbols.push_back(line);
				m_terminals.push_back(0);
				break;

			// In case of the axiom, use the whole line if it's an existing non-terminal.
			case GPS_AXIOM:
				it = lookup.find(line);
				if (it == lookup.end() || m_terminals[it->second]) {
					cerr << "grcc: \e[0;31merror:\e[0;0m line \"" << line 
						<< "\" isn't defined as an non-terminal in the grammar"
						<< endl;
					return;
				}
				m_axiom = it->second;
				state = GPS_NONE;
				break;

			// In case of a rule, split it to check words.
			case GPS_RULES:
				ss = stringstream(line);
				while (std::getline(ss, word, ' ')) {
					if (!word.size()) continue;
					words.push_back(word);
				}

				// Checl if the line is long enough.
				if (words.size() < 2) {
					cerr << "grcc: \e[0;31merror:\e[0;0m \"" << line
						 << "\" is too short to be a valid rule in the grammar" 
						 << endl;
					return;
				}

				// Check if the syntax is correct :
				// symbol_0 -> symbol_1 symbol_2 ... symbol_k
				for (std::size_t i = 0; i < words.size(); ++i) {

					// Incorrect syntax error.
					if ((!words[i].compare("->")) != (i == 1)) {
						cerr << "grcc: \e[0;31merror:\e[0;0m invalid rule "
							 << "at \"" << line << "\": syntax incorrect" 
							 << std::endl;
						return; 
					}

					// Unknown symbol error.
					if ((lookup.find(words[i]) == lookup.end() && i != 1)) {
						cerr << "grcc: \e[0;31merror:\e[0;0m invalid rule "
							 << "at \"" << line << "\": unknown symbol '"
							 << words[i] << "'" << endl;
						return; 
					}

				}

				// Check if symbol_0 is a non-terminal.
				it = lookup.find(words[0]);
				if (it == lookup.end() || m_terminals[it->second]) {
					cerr << "grcc: \e[0;31merror:\e[0;0m '" << words[0] 
						 << "' isn't defined as an non-terminal" << endl;
					return;
				}

				// Build and add the rule.
				rule = Rule();
				rule.symbol = it->second;
				for (std::size_t i = 2; i < words.size(); ++i) {
					rule.derivate.push_back(lookup.find(words[i])->second);
				}
				if (std::find(m_rules.begin(), m_rules.end(), rule) 
					== m_rules.end()) m_rules.push_back(rule);
				break;

			// Section undefined.
			case GPS_NONE: 
				cerr << "grcc: \e[0;31merror:\e[0;0m no section has been "
					 << "defined before '" << line << "'" << endl;
				return;
			}
		}

	}

	// Close the file.
	file.close();
	m_initState = true;

}

// Destructor.
Grammar::~Grammar() {}


// Printing function.
void Grammar::print(std::ostream &str) const {
	str << "[" << m_name << "]\n";

	// Print terminals.
	str << "Terminals:\n";
	for (std::size_t i = 0; i < m_terminals.size(); ++i) {
		if (m_terminals[i]) str << m_symbols[i] << '\n';
	}
	
	// Print non-terminals.
	str << "Non-Terminals:\n";
	for (std::size_t i = 0; i < m_terminals.size(); ++i) {
		if (!m_terminals[i]) str << m_symbols[i] << '\n';
	}

	// Print the axiom.
	str << "Axiom:\n" << m_symbols[m_axiom] << '\n';
	
	// Print the rules.
	str << "Rules:\n";
	for (const Rule &rule : m_rules) rule.print(m_symbols, str);

}


// Reduction operation.
void Grammar::reduce() {

	// Get inferior reduce to-be-removed symbols.
	vector<bool> inf_symbols;
	inferiorReduce(m_terminals, m_rules, inf_symbols);
	if (inf_symbols[m_axiom]) {
		cerr << "grcc: \e[0;31merror:\e[0;0m axiom was removed during grammar "
			 << "inferior reduction\n" << endl;
		return;
	}

	// Remove these symbols. 
	removeSymbols(inf_symbols);

	// Get suerior reduce to-be-removed symbols.
	vector<bool> sup_symbols;
	superiorReduce(m_axiom, m_terminals, m_rules, sup_symbols);

	// Remove these symbols. 
	removeSymbols(sup_symbols);

}

// Left derecursification operation.
void Grammar::derec() {
	
	// Copy and sort the rules depending its initial symbol.
	vector<set<Rule>> rules(m_terminals.size(), set<Rule>());
	for (const Rule& rule : m_rules) {
		rules[rule.symbol].insert(rule);
	}

	// Get each non-terminal identifiant.
	vector<int> id_to_symbol;
	for (std::size_t i = 0; i < m_terminals.size(); ++i) {
		if (!m_terminals[i]) id_to_symbol.push_back(i);
	}
	const int n = id_to_symbol.size();

	// Delete non-immediate left recursivation.
	for (int i = 0; i < n; ++i) {
		derec_update_rules(rules, m_terminals, rules[id_to_symbol[i]]);
		derec_immadiate(
			m_symbols[id_to_symbol[i]], rules,
			id_to_symbol[i], m_symbols, m_terminals
		);
	}

	// Replace old rules by new ones.
	m_rules.clear();
	for (auto &new_rules : rules) {
		for (const Rule &rule : new_rules) m_rules.push_back(rule);
	}

}


// Factorization operation.
void Grammar::fact() {

	cout<<"FACT"<<endl;

	// Copy and sort rules.
	vector<vector<Rule>> rules(m_terminals.size(), vector<Rule>());
	for (const Rule &rule : m_rules) rules[rule.symbol].push_back(rule);
	m_rules.clear();

	// For each rules that have a common initial symbol.
	for (std::size_t k = 0; k < rules.size(); ++k) {
		auto& current_rules = rules[k];
		if (!current_rules.size()) continue;
		const int symbol = current_rules[0].symbol;
		int fact_id = 1;
 
		cout << "symbol: " << m_symbols[symbol] << endl;
		for (const Rule& rule : current_rules) {
			rule.print(m_symbols);
		}

 		// Search for a factorization.
		int length;
		vector<bool> eq_class;
		do {

			// Get the longest common prefix.
			std::cout << "begin" << endl;
			length = commonPrefix(current_rules, eq_class);
			std::cout << "end" << endl;

			cout <<length <<endl;
			cout << eq_class.size() << endl <<": ";
			for (bool b : eq_class) {
				cout << b << " ";
			}
			cout << endl;


			if (!length) break;


			cout << eq_class.size() << endl;
			for (bool b : eq_class) cout << (b?"1":"0") << " ";
			cout << endl;

			// Get a rule containing the prefix.
			int elt = 0;
			while (!eq_class[elt]) ++elt;
			const auto& deriv = current_rules[elt].derivate;

			// Build the new rules and the factorization rule.
			vector<Rule> new_rules({Rule(symbol, vector<int>())});
			vector<int>& fact_deriv = new_rules[0].derivate;

			std::cout << length << " " << deriv.size() << endl;

			fact_deriv.insert(
				fact_deriv.begin(),
				deriv.begin(), std::next(deriv.begin(), length)
			);
			new_rules[0].derivate.push_back(m_symbols.size());

			// Add the new symbol.
			const int new_id = m_symbols.size();
			m_symbols.push_back(
				m_symbols[symbol] + "_" + std::to_string(fact_id)
			);
			m_terminals.push_back(0);

			// Build all the new rules.
			const int size = current_rules.size();
			for (int i = 0; i < size; ++i) {
				const Rule &old_rule = current_rules[i];

				// If the rule contains the common factor, factorize it.
				if (eq_class[i]) {
					m_rules.push_back(Rule(new_id, {}));
					vector<int>& new_deriv = m_rules.back().derivate;
					new_deriv.insert(
						new_deriv.end(),
						old_rule.derivate.begin() + length, 
						old_rule.derivate.end()
					);

				// Else, just copy it.
				} else new_rules.push_back(old_rule);
				
			}

			// Search for the next factorization.
			current_rules = new_rules;
			++fact_id;
		} while(length); 

	}

	// Remplace les anciennes règles par les nouvelles factorisées.
	for (const auto &new_rules : rules) {
		for (const Rule &rule : new_rules) m_rules.push_back(rule);
	}

	// Effectue le trie de la liste des règles.
	std::sort(m_rules.begin(), m_rules.end());

}



// Calculate the epsilon-derivable indicator.
void Grammar::epsilon(vector<bool> &eps) const {

	// Initialize the indicator.
	eps = vector<bool>(m_terminals.size(), false);

	// Continue while we don't find a stable state.
	bool changed = true;
	while (changed) {
		changed = false;

		// For each rule where initial symbol dont derivate into epsilon.
		for (const Rule &rule : m_rules) {
			if (m_terminals[rule.symbol] || eps[rule.symbol]) 
				continue;

			// Check if the rule can generate epsilon.
			bool res = true;
			for (int symbol : rule.derivate) res &= eps[symbol];
			
			// If it's the case, add the initial symbol to the indicator.
			if (res) {
				eps[rule.symbol] = true;
				changed = true;	
			}

		}

	}

}

// Calculate the firsts list.
void Grammar::first(const vector<bool> &eps, vector<set<int>> &first) const {

	// Initialize firsts list.
	first = vector<set<int>>(m_terminals.size(), set<int>());
	for (std::size_t i = 0; i < m_terminals.size(); ++i) {
		if (m_terminals[i]) first[i].insert(i);
	}

	// Continue while we don't find a stable state.
	bool changed = true;
	while (changed) {
		changed = false;

		// For each rule where derivation isn't empty.
		for (const Rule &rule : m_rules) {
			if (!rule.derivate.size()) continue;
			const std::size_t size = first[rule.symbol].size();

			// Add firsts of derivate symbol from left to right while it can 
			// be derivate into epsilon to the initial symbol firsts.
			bool all_eps = true;
			const auto end = rule.derivate.end();
			for (auto it = rule.derivate.begin(); it < end && all_eps; ++it) {
				first[rule.symbol].insert(
					first[*it].begin(), first[*it].end()
				);
				all_eps &= eps[*it];
			}

			// Update the change indicator.
			if (size != first[rule.symbol].size()) changed = true;

		}

	}

}

// Calculate the follows list.
// -1 is $ (end of word), all other ids correspond to the grammar ones.
void Grammar::follow(const vector<bool> &eps, const vector<set<int>> &first,
	vector<set<int>> &follow) const {

	// Initialize follows list.
	follow.clear();
	follow.reserve(m_terminals.size());
	for (std::size_t i = 0; i < m_terminals.size(); ++i) 
		follow.push_back(set<int>());
	follow[m_axiom].insert(-1);

	// Continue while we don't find a stable state.
	bool changed = true;
	while (changed) {
		changed = false;

		// For each symbol in the derivation of each rule.
		for (const Rule &rule : m_rules) {
			auto const end = rule.derivate.end();
			for (auto it = rule.derivate.begin(); it < end; ++it) {
				const int symbol = *it;
				const std::size_t size = follow[symbol].size();

				// Get the firsts of the part at the right of the current symbol.
				set<int> left;
				vectorFirst(eps, first, it+1, end, left);
				follow[symbol].insert(left.begin(), left.end());

				// If it can be derivated into espilon, add the follows of the initial symbol.
				bool all_eps = true;
				for (auto it_eps = it+1; it_eps < end; ++it_eps) 
					all_eps &= eps[*it_eps];
				if (all_eps) {
					const set<int>& rule_set = follow[rule.symbol];
					follow[symbol].insert(rule_set.begin(), rule_set.end());
				}

				// Update the change indicator.
				if (size != follow[symbol].size()) changed = true;

			}
		}

	}

}


// Calculate LLk table.
LLkTable Grammar::LLk() const {
	LLkTable table(m_terminals);

	// Lists to calculate.
	vector<bool> eps;
	vector<set<int>> firsts;
	vector<set<int>> follows;

	// Get epsilon-derivable indicator firsts list and follows list.
	epsilon(eps);
	first(eps, firsts);
	follow(eps, firsts, follows);

	// For each rule.
	for (std::size_t i = 0; i < m_rules.size(); ++i) {
		const int symbol = m_rules[i].symbol;
		const vector<int>& deriv = m_rules[i].derivate;

		// Calculate it's firsts.
		set<int> deriv_firsts;
		vectorFirst(
			eps, firsts,
			deriv.begin(), deriv.end(),
			deriv_firsts
		);

		// Add rule in firsts cell.
		for (int p : deriv_firsts) table.addRule(symbol, p, i);
		
		// If the rule is epsilon-derivable, add it in follows cell.
		bool res = true;
		for (int s : deriv) res &= eps[s];
		if (!res) continue;
		for (int f : follows[symbol]) 
			table.addRule(symbol, f, i);

	}

	// Return the built table.
	return table;

}


// Return initialization state.
bool Grammar::getInitState() const {
	return m_initState;
}
	


// Epsilon-derivable indicator.
const vector<bool> &Grammar::isTerminals() const {
	return m_terminals;
}

// Return a symbol string name.
const vector<string> &Grammar::getSymbols() const {
	return m_symbols;
}

// Return the id rule.
const vector<Rule> &Grammar::getRules() const {
	return m_rules;
}



// Delete symbols for wich symbols[id], and all associated rules.
void Grammar::removeSymbols(const vector<bool> &symbols) {

	// Get all rules containing a to-be-removed symbol.
	vector<bool> rules_to_remove(m_rules.size(), false);
	for (std::size_t i = 0; i < m_rules.size(); ++i) {
		const auto end = m_rules[i].derivate.end();
		rules_to_remove[i] = symbols[m_rules[i].symbol];
		for (auto it = m_rules[i].derivate.begin(); it < end; ++it) {
			rules_to_remove[i] = rules_to_remove[i] || symbols[*it];
			if (rules_to_remove[i]) break;
		}
	}

	// Update the grammar rules.
	vector<Rule> new_rules;
	for (std::size_t i = 0; i < rules_to_remove.size(); ++i) {
		if (rules_to_remove[i]) continue;
		new_rules.push_back(m_rules[i]);
	}
	m_rules = new_rules;

	// Update all symbols. Calcuate lookup, wich associate each symbol id to its new value.
	int j = 0;
	vector<bool> new_terminals;
	vector<string> new_symbols;
	vector<int> lookup(m_terminals.size(), -1);
	for (std::size_t i = 0; i < lookup.size(); ++i) {
		if (symbols[i]) continue;
		new_terminals.push_back(m_terminals[i]);
		new_symbols.push_back(m_symbols[i]);
		lookup[i] = j;
		++j;
	}
	m_terminals = new_terminals;
	m_symbols = new_symbols;

	// Update axiom and rules symbols id.
	m_axiom = lookup[m_axiom];
	for (Rule &rule : m_rules) {
		const auto end = rule.derivate.end();
		rule.symbol = lookup[rule.symbol];
		for (auto it = rule.derivate.begin(); it < end; ++it)
			*it = lookup[*it];
	}

}
