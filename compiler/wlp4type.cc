#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <stack>
#include <map>
#include <set>
using namespace std;

map<string, pair<vector<string>, map<string, string>>> tables;
string curFunction = "";
string type = "";
vector<int> argCount;
vector<string> typeCount;
bool returnState = false;
bool readParams = false;
int readArgList = 0;

class TreeNode {
public:
    string lvalue;
    string expr;
    string type;
    vector<TreeNode*> children;

    TreeNode(string lvalue, string expr) : lvalue(lvalue), expr(expr), type("") {}

    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }

    void addChild(TreeNode* child) {
        children.push_back(child);
    }

    void annotateAndPrint() {
        if (type.empty()) cout << expr;
        else cout << expr << " : " << type;
        for (auto child : children) {
            cout << endl;
            child->annotateAndPrint();
        }
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
    if (lvalue == "start" || lvalue == "procedures" || lvalue == "main" || lvalue == "type" || lvalue == "dcl" || lvalue == "dcls" || lvalue == "expr" || lvalue == "term" || lvalue == "factor" || lvalue == "lvalue" || lvalue == "procedure" || lvalue == "params" || lvalue == "paramlist" || lvalue == "arglist") {
        while (iss >> token && token != ".EMPTY") {
            root->addChild(buildTree());
        }
    }
    return root;
}

void analysis(TreeNode* root) {
    vector<string> operations;
    bool evaluateOperation = false;
    string funcName = "";
    for (auto child : root->children) {
        if (child->lvalue == "start" || child->lvalue == "procedures" || child->lvalue == "main" || child->lvalue == "type" || child->lvalue == "dcl" || child->lvalue == "dcls" || child->lvalue == "expr" || child->lvalue == "term" || child->lvalue == "factor" || child->lvalue == "lvalue" || child->lvalue == "procedure" || child->lvalue == "params" || child->lvalue == "paramlist" || child->lvalue == "arglist") {
            analysis(child);
        }
        if (child->lvalue == "LPAREN" && (root->lvalue == "main" || root->lvalue == "procedure")) readParams = true;
        else if (child->lvalue == "RPAREN" && (root->lvalue == "main" || root->lvalue == "procedure")) readParams = false;
        if (root->expr == "factor ID LPAREN arglist RPAREN" && child->lvalue == "LPAREN") readArgList += 1;
        if ((child->lvalue == "ID" && root->lvalue == "procedure") || (child->lvalue == "WAIN" && root->lvalue == "main")) {
            curFunction = child->expr.substr(child->expr.find(" ")+1);
            if (tables.find(curFunction) != tables.end()) cerr << "ERROR DUPLICATE PROCEDURE NAMES";
            vector<string> list;
            map<string, string> symbolTable;
            tables[curFunction] = {list, symbolTable};
            continue;
        }
        else if (child->lvalue == "type" && child->children.size() == 1 && child->children[0]->lvalue == "INT") {
            type = "int";
        }
        else if (child->lvalue == "type" && child->children.size() == 2 && child->children[0]->lvalue == "INT" && child->children[1]->lvalue == "STAR") {
            type = "int*";
        }
        else if (child->lvalue == "RETURN") {
            type = "";
            returnState = true;
        }
        if (returnState && child->lvalue == "ID" && root->expr != "factor ID LPAREN RPAREN" && root->expr != "factor ID LPAREN arglist RPAREN") {
            if (tables[curFunction].second[child->expr.substr(child->expr.find(" ")+1)] != "")  type = tables[curFunction].second[child->expr.substr(child->expr.find(" ")+1)];
            else cerr << "ERROR RETURN VALUE DOES NOT EXIST";
        }
        if (child->lvalue == "NULL" && type != "int*" && !returnState) cerr << "ERROR TYPE NOT NULL";
        if (child->lvalue == "NUM" && type != "int" && !returnState) cerr << "ERROR TYPE NOT NUM";
        if (child->lvalue == "NUM") type = "int";
        if (child->lvalue == "NULL") type = "int*";
        if (child->lvalue == "ID" || child->lvalue == "expr" || child->lvalue == "term" || child->lvalue == "factor" || child->lvalue == "NUM" || child->lvalue == "NULL" || child->lvalue == "lvalue") {
            child->type = type;
            if (readArgList != 0 && root->expr != "factor ID LPAREN RPAREN" && root->expr != "factor ID LPAREN arglist RPAREN" && ((child->lvalue == "ID" && tables[curFunction].second[child->expr.substr(child->expr.find(" ")+1)] != "") || child->lvalue == "NUM" || child->lvalue == "NULL")) {
                argCount.back() -= 1;
                typeCount.push_back(type);
            }
            if (child->lvalue == "ID" && readArgList != 0 && root->expr != "factor ID LPAREN RPAREN" && root->expr != "factor ID LPAREN arglist RPAREN" && tables[curFunction].second[child->expr.substr(child->expr.find(" ")+1)] == "") cerr << "ERROR VARIABLE NAME DOES NOT EXIST";
            if (!returnState && child->lvalue == "ID" && root->expr != "factor ID LPAREN RPAREN" && root->expr != "factor ID LPAREN arglist RPAREN") {
                if (tables[curFunction].second[child->expr.substr(child->expr.find(" ")+1)] != "") cerr << "ERROR VARIABLE NAME EXISTS";
                if (readParams) tables[curFunction].first.push_back(child->expr.substr(child->expr.find(" ")+1));
                if (readArgList == 0) tables[curFunction].second[child->expr.substr(child->expr.find(" ")+1)] = type;
            }
            // check if function name exists
            if (root->expr == "factor ID LPAREN RPAREN" && child->lvalue == "ID") {
                if (tables.find(child->expr.substr(child->expr.find(" ")+1)) == tables.end()) cerr << "ERROR UNDECLARED PROCEDURE";
                else if (tables[child->expr.substr(child->expr.find(" ")+1)].first.size() != 0 && tables[child->expr.substr(child->expr.find(" ")+1)].second.size() != 0) cerr << "ERROR ARGUMENT TYPES DIFFER";
                else if (!(tables[child->expr.substr(child->expr.find(" ")+1)].first.empty())) cerr << "ERROR FUNCTION DOES NOT EXIST";
                if (readArgList != 0) {
                    argCount.back() -= 1;
                    typeCount.push_back("int");
                }
                child->type = "";
                type = "int";
            }
            // remember func name
            else if (root->expr == "factor ID LPAREN arglist RPAREN" && child->lvalue == "ID") {
                funcName = child->expr.substr(child->expr.find(" ")+1);
                child->type = "";
                if (tables.find(child->expr.substr(child->expr.find(" ")+1)) == tables.end()) cerr << "ERROR UNDECLARED PROCEDURE";
                if (readArgList != 0) {
                    argCount.back() -= 1;
                    typeCount.push_back("int");
                }
                argCount.push_back(tables[child->expr.substr(child->expr.find(" ")+1)].first.size());
            }
        }
        if ((root->expr == "term term STAR factor" || root->expr == "term term SLASH factor" || root->expr == "term term PCT factor") && (child->lvalue == "term" || child->lvalue == "factor")) {
            evaluateOperation = true;
            operations.push_back(type);
            type = "";
        } 
        else if ((root->expr == "expr expr PLUS term" || root->expr == "expr expr MINUS term") && (child->lvalue == "expr" || child->lvalue == "term")) {
            evaluateOperation = true;
            operations.push_back(type);
            type = "";
        }
    }

    if ((root->expr == "lvalue STAR factor" || root->expr == "factor STAR factor") && type == "int*") type = "int";
    else if ((root->expr == "lvalue STAR factor" || root->expr == "factor STAR factor") && type == "int") cerr << "ERROR STAR OPERATION ERROR";
    if (root->expr == "factor AMP lvalue" && type == "int") type = "int*";
    else if (root->expr == "factor AMP lvalue" && type == "int*") cerr << "ERROR & SIGN ERROR";
    if (root->expr == "factor NEW INT LBRACK expr RBRACK" && type == "int") type = "int*";
    else if (root->expr == "factor NEW INT LBRACK expr RBRACK" && type == "int*") cerr << "ERROR NEW INT ERROR";
    
    if (evaluateOperation) {
        if ((root->expr == "term term STAR factor" || root->expr == "term term SLASH factor" || root->expr == "term term PCT factor") && operations[0] == "int" && operations[1] == "int") type = "int";
        else if ((root->expr == "expr expr PLUS term" || root->expr == "expr expr MINUS term") && operations[0] == "int" && operations[1] == "int") type = "int";
        else if (root->expr == "expr expr PLUS term" && ((operations[0] == "int*" && operations[1] == "int") || (operations[0] == "int" && operations[1] == "int*"))) type = "int*";
        else if (root->expr == "expr expr MINUS term" && operations[0] == "int*" && operations[1] == "int") type = "int*";
        else if (root->expr == "expr expr MINUS term" && operations[0] == "int*" && operations[1] == "int*") type = "int";
        else cerr << "ERROR OPERATION CANNOT BE EVALUATED";
    }

    // parameter type checking
    if (root->expr == "factor ID LPAREN arglist RPAREN" && readArgList != 0) {
        if (argCount.back() != 0) cerr << "ERROR FUNCTION PARAMETERS NOT SAME SIZE";
        else {
            for (int i = tables[funcName].first.size()-1; i >= 0; i--) {
                if (tables[funcName].second[tables[funcName].first[i]] != typeCount.back()) cerr << "ERROR ARGUMENT TYPES DO NOT MATCH";
                typeCount.pop_back();
            }
        }
        argCount.pop_back();
        type = "int";
        readArgList -= 1;
    }

    // check if return value is int
    if ((root->lvalue == "main" || root->expr == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") && type != "int") cerr << "ERROR OUTPUT IS NOT AN INT";
    if (root->expr == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        returnState = false;
        readParams = false;
        type = "";
        curFunction = "";
    }
}

int main() {
    TreeNode* root = buildTree();
    analysis(root);

    // check semantics for wain types
    if ((tables["wain"].first.size() != 2)) cerr << "ERROR INCORRECT PARAMETER SIZE OF WAIN";
    if (tables["wain"].second[tables["wain"].first[1]] != "int") cerr << "ERROR SECOND PARAMETER IN WAIN IS NOT INT";
    if (tables["wain"].first[0] == tables["wain"].first[1]) cerr << "ERROR WAIN PARAMETER NAMES MUST BE DIFFERENT";

    root->annotateAndPrint();
    cout << endl;

    delete root;
    return 0;
}
