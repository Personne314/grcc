#include "../src/grammar.h"

int main() {
    Grammar grammar("grammar_regex.txt");

    // This causes a SIGSEGV.
    // Don't know why, it worked before.
    // I'll debug that later 'cause I'm tired right now.

    grammar.reduce();
    grammar.print();
    grammar.derec();
    grammar.print();
    grammar.fact();
    grammar.print();

}
