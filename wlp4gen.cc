#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <stack>
using namespace std;

map<string, map <string, pair<string, int>>> table;
map<string, int> tableToParams;
string curProcedure;
string curID;
bool refLvalue = false;
int ifCount = 1;
int loopCount = 1;
int countArgs = 0;
int skipDeleteCount = 1;
bool mipsArray = false;

class TreeNode {
public:
    string lvalue;
    string value;
    string expr;
    string type;
    vector<TreeNode*> children;

    TreeNode(string lvalue, string expr) : lvalue(lvalue), expr(expr), type("") {
        istringstream iss(expr);
        iss >> value;
        iss >> value;
        string token;
        while (iss >> token) {
            type = token;
        }
    }

    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }

    void addChild(TreeNode* child) {
        children.push_back(child);
    }
};

TreeNode* buildTree() {
    string line;
    getline(cin, line);
    istringstream iss(line);
    string token;
    iss >> token;
    string lvalue = token;
    TreeNode* root = new TreeNode(lvalue, line);
    if (lvalue == "start" || lvalue == "procedures" || lvalue == "main" || lvalue == "type" || lvalue == "dcl" || lvalue == "dcls" || lvalue == "expr" || lvalue == "term" || lvalue == "factor" || lvalue == "lvalue" || lvalue == "procedure" || lvalue == "params" || lvalue == "paramlist" || lvalue == "arglist" || lvalue == "statement" || lvalue == "statements" || lvalue == "test") {
        while (iss >> token && token != ".EMPTY") {
            if (token == ":") break;
            root->addChild(buildTree());
        }
    }
    return root;
}

TreeNode* factorNode;
// ______________________________________________________________________

void code(string a) {
    int offset = table[curProcedure][a].second;
    cout << "lw $3, " << offset << "($29)" << endl;
}

void push(int x) {
    cout << "sw $" << x << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

void pop (int x) {
    cout << "add $30, $30, $4" << endl;
    cout << "lw $" << x << ", -4($30)" << endl;
}

void evalDcl(TreeNode *root) {
    string type = root->children[0]->value;
    if (type == "STAR") type = "int*";
    else type == "int";
    string id = root->children[1]->value;
    table[curProcedure][id] = {type, (-4*table[curProcedure].size())};
    if (curProcedure == "Fwain" && table[curProcedure].size()-1 == 0) push(1);
    else if (curProcedure == "Fwain" && table[curProcedure].size()-1 == 1) push(2);
}

void evalProDcl(TreeNode *root) {
    if (root->lvalue == "paramlist") {
        // parameter value
        string type = root->children[0]->children[0]->value;
        if (type == "STAR") type = "int*";
        else type = "int";
        table[curProcedure][root->children[0]->children[1]->value] = {type, 4*(countArgs - table[curProcedure].size())};
    }
    else {
        string type = root->children[0]->value;
        if (type == "STAR") type = "int*";
        else type == "int";
        string id = root->children[1]->value;
        table[curProcedure][id] = {type, (-12-4*(table[curProcedure].size() - tableToParams[curProcedure]))};
    }
}

void evalDcls(TreeNode *root) {
    if (root->children.size() > 1) {
        if (root->children[0]->value != ".EMPTY") evalDcls(root->children[0]);
        // get current variable name
        if (curProcedure == "Fwain") evalDcl(root->children[1]);
        else evalProDcl(root->children[1]);
        string num = root->children[3]->value;
        if (num == "NULL") num = "0";
        cout << "lis $5" << endl;
        cout << ".word " << num << endl;
        push(5);
    }
}

void evalLvalue(TreeNode *root) {
    if (root->children[0]->lvalue == "ID") {
        curID = root->children[0]->value;
        refLvalue = false;
    }
    else if (root->children[0]->lvalue == "STAR") {
        factorNode = root->children[1];
        refLvalue = true;
    }
    else if (root->children[0]->lvalue == "LPAREN") evalLvalue(root->children[1]);
}

void countArglist(TreeNode *root) {
    countArgs += 1;
    if (root->children.size() == 3) countArglist(root->children[2]);
}

void evalExpr(TreeNode *root) {
    if (root->lvalue == "expr") {
        if (root->children[0]->lvalue == "term") evalExpr(root->children[0]);
        else if (root->children[0]->type == "int*" || root->children[2]->type == "int*"){
            // plus minus for pointer
            if (root->children[1]->lvalue == "PLUS" && (root->children[0]->type == "int*" && root->children[2]->type == "int")) {
                evalExpr(root->children[0]);
                push(3);
                evalExpr(root->children[2]);
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                pop(5);
                cout << "add $3, $5, $3" << endl;
            }
            else if (root->children[1]->lvalue == "PLUS" && (root->children[0]->type == "int" && root->children[2]->type == "int*")) {
                evalExpr(root->children[2]);
                push(3);
                evalExpr(root->children[0]);
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                pop(5);
                cout << "add $3, $5, $3" << endl;
            }
            else if (root->children[1]->lvalue == "MINUS" && (root->children[0]->type == "int*" && root->children[2]->type == "int")) {
                evalExpr(root->children[0]);
                push(3);
                evalExpr(root->children[2]);
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                pop(5);
                cout << "sub $3, $5, $3" << endl;
            }
            else if (root->children[1]->lvalue == "MINUS" && (root->children[0]->type == "int*" && root->children[2]->type == "int*")) {
                evalExpr(root->children[0]);
                push(3);
                evalExpr(root->children[2]);
                pop(5);
                cout << "sub $3, $5, $3" << endl;
                cout << "div $3, $4" << endl;
                cout << "mflo $3" << endl;
            }
        }
        else {
            // plus, minus
            evalExpr(root->children[0]);
            push(3);
            evalExpr(root->children[2]);
            pop(5);
            if (root->children[1]->lvalue == "PLUS") cout << "add $3, $3, $5" << endl;
            else if (root->children[1]->lvalue == "MINUS") cout << "sub $3, $5, $3" << endl;
        }
    }
    else if (root->lvalue == "term") {
        if (root->children[0]->lvalue == "factor") evalExpr(root->children[0]);
        else {
            // mult, div, pct
            evalExpr(root->children[0]);
            push(3);
            evalExpr(root->children[2]);
            pop(5);
            if (root->children[1]->lvalue == "STAR") {
                cout << "mult $5, $3" << endl;
                cout << "mflo $3" << endl;
            }
            else if (root->children[1]->lvalue == "SLASH") {
                cout << "div $5, $3" << endl;
                cout << "mflo $3" << endl;
            }
            else if (root->children[1]->lvalue == "PCT") {
                cout << "div $5, $3" << endl;
                cout << "mfhi $3" << endl;
            }
        }
    }
    else if (root->lvalue == "factor") {
        if (root->children[0]->lvalue == "ID" && root->children.size() == 1) {
            string id = root->children[0]->value;
            code(id);
        }
        // factor → ID LPAREN RPAREN
        else if (root->children[0]->lvalue == "ID" && root->children.size() == 3) {
            push(29);
            push(31);
            cout << "lis $5" << endl;
            cout << ".word " << "F" + root->children[0]->value << endl;
            cout << "jalr $5" << endl;
            pop(31);
            pop(29);
        }
        // factor → ID LPAREN arglist RPAREN
        else if (root->children[0]->lvalue == "ID" && root->children.size() == 4) {
            countArgs = 0;
            push(29);
            push(31);
            countArglist(root->children[2]);
            int numParams = countArgs;
            evalExpr(root->children[2]);
            cout << "lis $5" << endl;
            cout << ".word " << "F" + root->children[0]->value << endl;
            cout << "jalr $5" << endl;
            for (int i = 0; i < numParams; i++) cout << "add $30, $30, $4" << endl;
            pop(31);
            pop(29);
        }
        else if (root->children[0]->lvalue == "NUM") {
            cout << "lis $3" << endl;
            cout << ".word " << root->children[0]->value << endl;
        }
        else if (root->children[0]->lvalue == "NULL") {
            cout << "add $3, $0, $11" << endl;
        }
        else if (root->children[0]->lvalue == "LPAREN") evalExpr(root->children[1]);
        else if (root->children[0]->lvalue == "STAR") {
            evalExpr(root->children[1]);
            cout << "lw $3, 0($3)" << endl;
        }
        else if (root->children[0]->lvalue == "AMP") {
            evalLvalue(root->children[1]);
            // lvalue -> ID
            if (!refLvalue) {
                int offset = table[curProcedure][curID].second;
                cout << "lis $3" << endl;
                cout << ".word " << offset << endl;
                cout << "add $3, $3, $29" << endl;
            }
            // lvalue -> STAR factor
            else if (refLvalue) {
                evalExpr(factorNode);
            }
        }
        else if (root->children[0]->lvalue == "NEW" && root->children[1]->lvalue == "INT") {
            evalExpr(root->children[3]);
            cout << "add $1, $3, $0" << endl;
            push(31);
            cout << "lis $5" << endl;
            cout << ".word new" << endl;
            cout << "jalr $5" << endl;
            pop(31);
            cout << "bne $3, $0, 1" << endl;
            cout << "add $3, $11, $0" << endl;
        }
    }
    else if (root->lvalue == "arglist") {
        evalExpr(root->children[0]);
        push(3);
        if (root->children.size() == 3) evalExpr(root->children[2]);
    }
}

void evalTest(TreeNode *root) {
    evalExpr(root->children[0]);
    push(3);
    evalExpr(root->children[2]);
    pop(5);
    if (root->children[1]->lvalue == "EQ") {
        if (root->children[0]->type == "int*") {
            cout << "sltu $6, $3, $5" << endl;
            cout << "sltu $7, $5, $3" << endl;
        }
        else {
            cout << "slt $6, $3, $5" << endl;
            cout << "slt $7, $5, $3" << endl;
        }
        cout << "add $3, $6, $7" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
    else if (root->children[1]->lvalue == "NE") {
        if (root->children[0]->type == "int*") {
            cout << "sltu $6, $3, $5" << endl;
            cout << "sltu $7, $5, $3" << endl;
        }
        else {
            cout << "slt $6, $3, $5" << endl;
            cout << "slt $7, $5, $3" << endl;
        }
        cout << "add $3, $6, $7" << endl;
    }
    else if (root->children[1]->lvalue == "LT") {
        if (root->children[0]->type == "int*") cout << "sltu $3, $5, $3" << endl;
        else cout << "slt $3, $5, $3" << endl;
    }
    else if (root->children[1]->lvalue == "LE") {
        if (root->children[0]->type == "int*") cout << "sltu $3, $3, $5" << endl;
        else cout << "slt $3, $3, $5" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
    else if (root->children[1]->lvalue == "GT") {
        if (root->children[0]->type == "int*") cout << "sltu $3, $3, $5" << endl;
        else cout << "slt $3, $3, $5" << endl;
    }
    else if (root->children[1]->lvalue == "GE") {
        if (root->children[0]->type == "int*") cout << "sltu $3, $5, $3" << endl;
        else cout << "slt $3, $5, $3" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
}

void evalStatements(TreeNode *root) {
    if (root->children.size() > 1) {
        if (root->children[0]->value != ".EMPTY") evalStatements(root->children[0]);
        if (root->children[1]->children.size() >= 1 && root->children[1]->children[0]->lvalue == "lvalue") {
            evalLvalue(root->children[1]->children[0]);
            string ID = curID;
            TreeNode *fNode = factorNode;
            if (!refLvalue) {
                evalExpr(root->children[1]->children[2]);
                int offset = table[curProcedure][ID].second;
                cout << "sw $3, " << offset << "($29)" << endl;
            }
            else if (refLvalue) {
                evalExpr(root->children[1]->children[2]);
                push(3);
                evalExpr(fNode);
                pop(5);
                cout << "sw $5, 0($3)" << endl;
            }
        }
        else if (root->children[1]->children.size() >= 1 && root->children[1]->children[0]->lvalue == "IF") {
            int num = ifCount;
            ifCount += 1;
            evalTest(root->children[1]->children[2]);
            cout << "beq $3, $0, else" << num << endl;
            evalStatements(root->children[1]->children[5]);
            cout << "beq $0, $0, endif" << num << endl;
            cout << "else" << num << ":";
            evalStatements(root->children[1]->children[9]);
            cout << "endif" << num << ":";
        }
        else if (root->children[1]->children.size() >= 1 && root->children[1]->children[0]->lvalue == "WHILE") {
            int num = loopCount;
            loopCount += 1;
            cout << "loop" << num << ":";
            evalTest(root->children[1]->children[2]);
            cout << "beq $3 , $0 , endWhile"  << num << endl;
            evalStatements(root->children[1]->children[5]);
            cout << "beq $0 , $0 , loop" << num << endl;
            cout << "endWhile" << num << ":" << endl;
        }
        else if (root->children[1]->children.size() >= 1 && root->children[1]->children[0]->lvalue == "PRINTLN") {
            push(1);
            evalExpr(root->children[1]->children[2]);
            cout << "add $1, $3, $0" << endl;
            push(31);
            cout << "lis $5" << endl;
            cout << ".word print" << endl;
            cout << "jalr $5" << endl;
            pop(31);
            pop(1);
        }
        else if (root->children[1]->children.size() >= 1 && root->children[1]->children[0]->lvalue == "DELETE") {
            evalExpr(root->children[1]->children[3]);
            cout << "beq $3, $11, skipDelete" << skipDeleteCount << endl;
            cout << "beq $3, $0, skipDelete" << skipDeleteCount << endl;
            cout << "add $1, $3, $0" << endl;
            push(31);
            cout << "lis $5" << endl;
            cout << ".word delete" << endl;
            cout << "jalr $5" << endl;
            pop(31);
            cout << "skipDelete" << skipDeleteCount << ":" << endl;
            skipDeleteCount += 1;
        }
    }
}

void evalParams(TreeNode *root) {
    // params → paramlist
    if (root->lvalue == "params" && root->children.size() != 0) {
        evalParams(root->children[0]);
    }
    else if (root->lvalue == "paramlist") {
        evalProDcl(root);
        if (root->children.size() == 3) evalParams(root->children[2]);
    }
}

void countParams(TreeNode *root) {
    if (root->lvalue == "params" && root->children.size() != 0) {
        countArgs = 0;
        countParams(root->children[0]);
    }
    else if (root->lvalue == "paramlist") {
        countArgs += 1;
        if (root->children.size() == 3) countParams(root->children[2]);
    }
    tableToParams[curProcedure] = countArgs;
}

void evalProcedure(TreeNode *root) {
    curProcedure = "F" + root->children[1]->value;
    cout << curProcedure + ":" << endl;
    cout << "sub $29 , $30 , $4" << endl;
    push(5);
    push(6);
    push(7);
    countParams(root->children[3]);
    int numParams = countArgs;
    evalParams(root->children[3]);
    evalDcls(root->children[6]);
    evalStatements(root->children[7]);
    evalExpr(root->children[9]);
    for (int i = 0; i < (int(table[curProcedure].size())-numParams); i++) {
        cout << "add $30, $30, $4" << endl;
    }
    pop(7);
    pop(6);
    pop(5);
    cout << "jr $31" << endl;
}

void evalMain(TreeNode *root) {
    curProcedure = "F" + root->children[1]->value;
    cout << curProcedure + ":" << endl;
    cout << "sub $29, $30, $4" << endl;
    evalDcl(root->children[3]);
    evalDcl(root->children[5]);
    //init
    if (root->children[3]->children[1]->type == "int") cout << "add $2, $0, $0" << endl;
    push(31);
    cout << "lis $5" << endl;
    cout << ".word init" << endl;
    cout << "jalr $5" << endl;
    pop(31);
    //_____________________
    evalDcls(root->children[8]);
    evalStatements(root->children[9]);
    evalExpr(root->children[11]);
    for (int i = 0; i < int(table[curProcedure].size()); i++) {
        cout << "add $30, $30, $4" << endl;
    }
    cout << "add $30, $29, $4" << endl;
    cout << "jr $31" << endl;
}

void convertMips(TreeNode *root) {
    for (auto child : root->children) {
        if (child->lvalue == "start") convertMips(child);
        else if (child->lvalue == "procedures") convertMips(child);
        else if (child->lvalue == "main") evalMain(child);
        else if (child->lvalue == "procedure") evalProcedure(child);
    }
}

int main() {
    TreeNode* root = buildTree();

    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "beq $0, $0, Fwain" << endl;

    convertMips(root);

    delete root;
    return 0;
}
