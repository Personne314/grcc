#include <iostream>
#include <filesystem>
#include <cassert>

#include "../src/grammar.h"





// This is used to test the grammar.h/grammar.cpp code.
int main(int argc, char *argv[]) {
    (void)argc;
    std::filesystem::current_path(
        std::filesystem::absolute(std::filesystem::path(argv[0])).remove_filename() 
    );

    // Test of vectorFirst.
    // Typical use case.
    std::vector<bool> eps = {1,1,0,0,0,0,0,0};
    std::vector<std::set<int>> first = {{4},{5,4},{6},{7},{4},{5},{6},{7}};
    std::vector<int> word = {0,1,0,7,3};
    std::set<int> res;
    vectorFirst(eps, first, word.begin(), word.end(), res);
    assert(*res.begin() == 4);
    assert(*std::next(res.begin()) == 5);
    assert(*std::next(res.begin(),2) == 7);

    // Case where the word can be derivated into epsilon.
    word = {0};
    res.clear();
    vectorFirst(eps, first, word.begin(), word.end(), res);
    assert(*res.begin() == 4);

    // Case of an empty word.
    word = {};
    res.clear();
    vectorFirst(eps, first, word.begin(), word.end(), res);
    assert(res.size() == 0);


/*

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
*/






/*
    Grammar grammar("./grammar_regex.txt");
    if (!grammar.getInitState()) {
         std::cout << "Init Error" << std::endl;
         return 0;
    }

    grammar.reduce();
    grammar.derec();
    grammar.print();
    grammar.fact();
    //grammar.print();
*/

    std::cout << "[\e[0;32mOK\e[0;30m] test_grammar" << std::endl;

}
