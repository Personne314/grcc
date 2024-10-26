#pragma once

#include <string>
#include <vector>



/*
Here is some code to parse regex into an AST.
The grammar used is described in grammar_regex.txt.
This grammar was processed into a usable LL1 grammar in grammar_regex_ll1.txt.
The corresponding LL1 table, that was used for the implementation of the parser, 
is in grammar_regex_table.txt.
These processed grammar and LL1 table were generated using the Grammar operations.
*/



// Type of nodes in a regex AST.
enum RegexTreeType : unsigned char {
	REG_TREE_EXPR,
	REG_TREE_UNION,
	REG_TREE_CONCAT,
	REG_TREE_ATOM,
	REG_TREE_QUANT,
	REG_TREE_SKIP,
	REG_TREE_CLASS,
	REG_TREE_SEQ,
	REG_TREE_LITT
};



// Macro definitions to manipulate tokens.
#define TOKEN_MAKE(type, val, pos) (((pos)<<16) + ((type)<<8) + val)
#define TOKEN_SPLIT(token, type, val) \
	type = (token)>>8; \
	val = (token)&0xFF;
#define TOKEN_POS(token, pos) (pos = (token)>>16)
#define TOKEN_TYPE(token, type) (type = (token)>>8)
#define TOKEN_VAL(token, val) (val = (token)&0xFF)
#define TOKEN_CMP(tk, type, val) (((tk)&0xFFFF) == ((type)<<8) + val)



// Prints the indent following a node.
class RegexTreeNode {
public:
	RegexTreeNode();
	virtual ~RegexTreeNode() = 0;
	virtual RegexTreeType getType() const = 0;
	virtual void print(int depth) const = 0;
	virtual std::string to_string() const = 0;
};

// Node class that serves as the expression root.
class RegexTreeExpr : public RegexTreeNode {
public :
	RegexTreeExpr();
	~RegexTreeExpr();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth = 0) const;
	friend bool regexParseExpr(std::vector<int>::const_iterator &it, RegexTreeExpr &tree, bool verbose);
private :
	RegexTreeNode *m_node;
	bool m_begin;
	bool m_end;
};

// Node class used to represent an union.
class RegexTreeAtom;
class RegexTreeUnion : public RegexTreeNode {
public :
	RegexTreeUnion();
	~RegexTreeUnion();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseExpr(std::vector<int>::const_iterator &it, RegexTreeExpr &tree, bool verbose);
	friend bool regexParseUnion(std::vector<int>::const_iterator &it, RegexTreeUnion *&node, bool verbose);
	friend bool regexParseAtom1(std::vector<int>::const_iterator &it, RegexTreeAtom *&node, bool verbose);
private :
	std::vector<RegexTreeNode*> m_nodes;
};

// Node class used to represent a concatenation.
class RegexTreeConcat : public RegexTreeNode {
public :
	RegexTreeConcat();
	~RegexTreeConcat();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseUnion(std::vector<int>::const_iterator &it, RegexTreeUnion *&node, bool verbose);
	friend bool regexParseConcat(std::vector<int>::const_iterator &it, RegexTreeConcat *&node, bool verbose);
private :
	std::vector<RegexTreeNode*> m_nodes;
};

// Node class used to represent a quantifier.
class RegexTreeQuant : public RegexTreeNode {
public :
	RegexTreeQuant();
	~RegexTreeQuant();
	bool isValid() const;
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseQuant(std::vector<int>::const_iterator &it, RegexTreeQuant *&node, bool verbose);
	friend bool regexParseQuant1(std::vector<int>::const_iterator &it, RegexTreeQuant *&node, bool verbose); 
	friend bool regexParseQuant2(std::vector<int>::const_iterator &it, RegexTreeQuant *&node, bool verbose);
private :
	int m_min;
	int m_max;
};

// Node class used to represent an atom.
class RegexTreeAtom : public RegexTreeNode {
public :
	RegexTreeAtom();
	~RegexTreeAtom();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseConcat(std::vector<int>::const_iterator &it, RegexTreeConcat *&node, bool verbose);
	friend bool regexParseAtom(std::vector<int>::const_iterator &it, RegexTreeAtom *&node, bool verbose);
	friend bool regexParseAtom1(std::vector<int>::const_iterator &it, RegexTreeAtom *&node, bool verbose);
private :
	RegexTreeNode *m_node;
	RegexTreeQuant *m_quant;
};

// Node class used to represent a skip sequence.
class RegexTreeSkip : public RegexTreeNode {
public :
	RegexTreeSkip();
	~RegexTreeSkip();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseAtom1(std::vector<int>::const_iterator &it, RegexTreeAtom *&node, bool verbose);
private :
	bool m_match;
	RegexTreeNode *m_node;
};

// Node class used to represent an union.
class RegexTreeClass : public RegexTreeNode {
public :
	RegexTreeClass();
	~RegexTreeClass();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseClass(std::vector<int>::const_iterator &it, RegexTreeClass *&node, bool verbose);
	friend bool regexParseLetter1(std::vector<int>::const_iterator &it, RegexTreeNode *&node, bool verbose);
private :
	bool m_match;
	std::vector<RegexTreeNode*> m_nodes;
};

// Node class used to represent a litteral.
class RegexTreeSeq;
class RegexTreeLitt : public RegexTreeNode {
public :
	RegexTreeLitt();
	~RegexTreeLitt();
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend void regexParseLitt(std::vector<int>::const_iterator &it, RegexTreeLitt *&node, bool verbose);
	friend bool regexParseElt(std::vector<int>::const_iterator &it, 
		std::vector<RegexTreeNode*> &nodes, bool verbose);
	friend bool regexParseElt1(std::vector<int>::const_iterator &it, 
		std::vector<RegexTreeNode*> &nodes, RegexTreeLitt *&node, bool verbose);
	friend class RegexTreeSeq;
private :
	bool m_esc;
	char m_val;
};

// Node class used to represent a sequence of characters.
class RegexTreeSeq : public RegexTreeNode {
public :
	RegexTreeSeq();
	~RegexTreeSeq();
	bool isValid() const;
	RegexTreeType getType() const;
	std::string to_string() const;
	void print(int depth) const;
	friend bool regexParseElt1(std::vector<int>::const_iterator &it, 
		std::vector<RegexTreeNode*> &nodes, RegexTreeLitt *&node, bool verbose);
private :
	RegexTreeLitt *m_litt_min;
	RegexTreeLitt *m_litt_max;
};



// Regex parser function.
bool regexParser(RegexTreeExpr &tree, const std::string &str, bool verbose = false);
