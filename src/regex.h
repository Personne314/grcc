#pragma once

#include <string>
#include <vector>


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



// Macro definition to manipulate tokens.
#define TOKEN_MAKE(type, val) ((type<<8) + val)
#define TOKEN_SPLIT(token, type, val) \
	type = token>>8; \
	val = token&0xFF;
#define TOKEN_TYPE(token, type) (type = token>>8)
#define TOKEN_VAL(token, val) (val = token&0xFF)





class RegexTreeNode {
public:
	RegexTreeNode();
	virtual ~RegexTreeNode() = 0;
	virtual RegexTreeType getType() const = 0;
	virtual void print(int depth) const = 0;
};



class RegexTreeExpr : public RegexTreeNode {
public :

	RegexTreeExpr();
	~RegexTreeExpr();

	RegexTreeType getType() const;
	void print(int depth = 0) const;

	friend bool regexParseExpr(std::vector<int>::const_iterator it, RegexTreeExpr &tree);

private :

	RegexTreeNode *m_node;
	bool m_begin;
	bool m_end;

};



class RegexTreeUnion : public RegexTreeNode {
public :

	RegexTreeUnion();
	~RegexTreeUnion();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseUnion(std::vector<int>::const_iterator &it, RegexTreeUnion *&node);

private :

	std::vector<RegexTreeNode*> m_nodes;

};



class RegexTreeConcat : public RegexTreeNode {
public :

	RegexTreeConcat();
	~RegexTreeConcat();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseConcat(std::vector<int>::const_iterator &it, RegexTreeConcat *&node);

private :

	std::vector<RegexTreeNode*> m_nodes;

};



class RegexTreeQuant : public RegexTreeNode {
public :

	RegexTreeQuant();
	~RegexTreeQuant();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseQuant(std::vector<int>::const_iterator &it, RegexTreeQuant *&node);
	friend bool regexParseQuant1(std::vector<int>::const_iterator &it, RegexTreeQuant *&node); 
	friend bool regexParseQuant2(std::vector<int>::const_iterator &it, RegexTreeQuant *&node);

private :

	int m_min;
	int m_max;

};



class RegexTreeAtom : public RegexTreeNode {
public :

	RegexTreeAtom();
	~RegexTreeAtom();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseAtom(std::vector<int>::const_iterator &it, RegexTreeAtom *&node);
	friend bool regexParseAtom1(std::vector<int>::const_iterator &it, RegexTreeAtom *&node);

private :

	RegexTreeNode *m_node;
	RegexTreeQuant *m_quant;

};



class RegexTreeSkip : public RegexTreeNode {
public :

	RegexTreeSkip();
	~RegexTreeSkip();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseAtom1(std::vector<int>::const_iterator &it, RegexTreeAtom *&node);

private :

	bool m_match;
	RegexTreeNode *m_node;

};



class RegexTreeClass : public RegexTreeNode {
public :

	RegexTreeClass();
	~RegexTreeClass();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseList(std::vector<int>::const_iterator &it, RegexTreeClass *&node);

private :

	bool m_match;
	std::vector<RegexTreeNode*> m_nodes;

};



class RegexTreeLitt : public RegexTreeNode {
public :

	RegexTreeLitt();
	~RegexTreeLitt();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend void regexParseLitt(std::vector<int>::const_iterator &it, RegexTreeLitt *&node);

private :

	bool m_esc;
	char m_val;

};



class RegexTreeSeq : public RegexTreeNode {
public :

	RegexTreeSeq();
	~RegexTreeSeq();

	RegexTreeType getType() const;
	void print(int depth) const;

	friend bool regexParseElt1(std::vector<int>::const_iterator &it, std::vector<RegexTreeNode*> &nodes, RegexTreeLitt *&node);

private :

	RegexTreeLitt *m_litt_min;
	RegexTreeLitt *m_litt_max;

};
