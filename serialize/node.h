#ifndef __NODE_H__
#define __NODE_H__

#include <string>
#include <vector>
#include <iostream>

#include <llvm/IR/Value.h>

using std::cout;
using std::endl;

class CodeGenContext;

class Node {
public:
	virtual ~Node() {

	}

	virtual llvm::Value* codeGen(CodeGenContext& context) {
		return nullptr;
	}

	virtual void dump() const = 0;
};

class NIdentifier : public Node {
public:
	NIdentifier(const std::string& name) : name_(name) {

	}

	virtual void dump() const override {
		cout << "ID " << name_;
	}

	const std::string& getName() const {
		return name_;
	}
private:
	std::string name_;
};

class NStatement : public Node {
public:
	virtual ~NStatement() {

	}

	virtual void dump() const override = 0;
};

class NVariable : public Node {
public:
	NVariable(NIdentifier *identp, NIdentifier* typep, std::string* location) :
			identp_(identp), typep_(typep), locp_(location) {

	}

	~NVariable() {
		delete identp_;
		delete typep_;
		delete locp_;
	}

	virtual void dump() const override {

	}

private:
	NIdentifier *identp_;
	NIdentifier *typep_;
	std::string *locp_;
};

class NStructStatement : public NStatement {
public:
	NStructStatement(NIdentifier *identp, std::vector<NVariable *> *varvecp) :
			identp_(identp), varvecp_(varvecp) {

	}

	~NStructStatement() {
		for (auto it = varvecp_->begin(); it != varvecp_->end(); ++it){
    		delete *it;
		}
		varvecp_->clear();
		delete varvecp_;
		delete identp_;
	}

	virtual void dump() const override {

	}
private:
	NIdentifier              *identp_;
	std::vector<NVariable *> *varvecp_;
};

class NUsingStatement : public NStatement {
public:
	NUsingStatement(NIdentifier *dstp, NIdentifier *srcp) :
			dstp_(dstp), srcp_(srcp) {

	}

	virtual ~NUsingStatement() {
		delete dstp_;
		delete srcp_;
	}

	virtual void dump() const override {
		cout << dstp_->getName() << " = " << srcp_->getName() << endl;
	}
private:
	NIdentifier *dstp_;
	NIdentifier *srcp_;
};

class NProgram : public Node {
public:
	NProgram(std::vector<NStatement*> *vecp) : vecp_(vecp) {

	}

	virtual ~NProgram() {
		for (auto it = vecp_->begin(); it != vecp_->end(); ++it) {
			delete *it;
		}
		vecp_->clear();
		delete vecp_;
	}

	void dump() const {
		for (auto it = vecp_->begin(); it != vecp_->end(); ++it) {
			(*it)->dump();
		}
	}
private:
	std::vector<NStatement*> *vecp_;
};

#endif