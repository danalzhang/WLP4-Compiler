#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}
std::string getKind(std::string s) {
  if (s == "(") return "LPAREN";
  else if (s == ")") return "RPAREN";
  else if (s == "{") return "LBRACE";
  else if (s == "}") return "RBRACE";
  else if (s == "return") return "RETURN";
  else if (s == "if") return "IF";
  else if (s == "else") return "ELSE";
  else if (s == "while") return "WHILE";
  else if (s == "println") return "PRINTLN";
  else if (s == "wain") return "WAIN";
  else if (s == "=") return "BECOMES";
  else if (s == "int") return "INT";
  else if (s == "==") return "EQ";
  else if (s == "!=") return "NE";
  else if (s == "<") return "LT";
  else if (s == ">") return "GT";
  else if (s == "<=") return "LE";
  else if (s == ">=") return "GE";
  else if (s == "+") return "PLUS";
  else if (s == "-") return "MINUS";
  else if (s == "*") return "STAR";
  else if (s == "/") return "SLASH";
  else if (s == "%") return "PCT";
  else if (s == ",") return "COMMA";
  else if (s == ";") return "SEMI";
  else if (s == "new") return "NEW";
  else if (s == "delete") return "DELETE";
  else if (s == "[") return "LBRACK";
  else if (s == "]") return "RBRACK";
  else if (s == "&") return "AMP";
  else if (s == "NULL") return "NULL";
  else if (s[0] == '0' || s[0] == '1' || s[0] == '2' || s[0] == '3' || s[0] == '4' || s[0] == '5' || s[0] == '6' || s[0] == '7' || s[0] == '8' || s[0] == '9') return "NUM";
  return "ID";
}


// Locations in the program that you should modify to store the
// DFA information have been marked with four-slash comments:
//// (Four-slash comment)
int main() {
  std::ifstream in("wlp4.dfa");
  std::string s;
  std::unordered_map<char, bool> alphabet;
  std::vector <std::string> states;
  std::string initialState;
  std::vector <std::string> acceptingStates;
  std::map<std::pair<std::string, char>, std::string> transitions;

  std::getline(in, s); // Alphabet section (skip header)
  // Read characters or ranges separated by whitespace
  while(in >> s) {
    if (s == STATES) { 
      break; 
    } else {
      if (isChar(s)) {
        //// Variable 's[0]' is an alphabet symbol
        alphabet[s[0]] = true;
      } else if (isRange(s)) {
        for(char c = s[0]; c <= s[2]; ++c) {
          //// Variable 'c' is an alphabet symbol
          alphabet[c] = true;
        }
      } 
    }
  }

  std::getline(in, s); // States section (skip header)
  // Read states separated by whitespace
  while(in >> s) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      static bool initial = true;
      bool accepting = false;
      if (s.back() == '!' && !isChar(s)) {
        accepting = true;
        s.pop_back();
      }
      //// Variable 's' contains the name of a state
      states.push_back(s);
      if (initial) {
        //// The state is initial
        initialState = s;
        initial = false;
      }
      if (accepting) {
        //// The state is accepting
        acceptingStates.push_back(s);
      }
    }
  }

  std::getline(in, s); // Transitions section (skip header)
  // Read transitions line-by-line
  while(std::getline(in, s)) {
    if (s == INPUT) { 
      // Note: Since we're reading line by line, once we encounter the
      // input header, we will already be on the line after the header
      break; 
    } else {
      std::string fromState, symbols, toState;
      std::istringstream line(s);
      std::vector<std::string> lineVec;
      while(line >> s) {
        lineVec.push_back(s);
      }
      fromState = lineVec.front();
      toState = lineVec.back();
      for(int i = 1; i < int(lineVec.size())-1; ++i) {
        std::string s = lineVec[i];
        if (isChar(s)) {
          symbols += s;
        } else if (isRange(s)) {
          for(char c = s[0]; c <= s[2]; ++c) {
            symbols += c;
          }
        }
      }
      for ( char c : symbols ) {
        //// There is a transition from 'fromState' to 'toState' on 'c'
        transitions[{fromState, c}] = toState;
      }
    }
  }

  std::string line;
  // Input section (already skipped header)
  while (std::getline(std::cin, line)) {
      bool commentLine = false;
      std::istringstream input(line);
      while(input >> s) {
      //// Variable 's' contains an input string for the DFA
      std::string currentState = initialState;
      int ptr = 0;
      int len = 0;
      bool reset = false;
      for (int i = 0; i < int(s.length()); i++) {
          if (alphabet.find(s[i]) == 0) {
              std::cerr << "ERROR";
              break;
          }
          if (transitions.find({currentState, s[i]}) == transitions.end()) {
              bool isAccepted = false;
              for (size_t j = 0; j < acceptingStates.size(); j++) {
                  if (acceptingStates[j] == currentState) isAccepted = true;
              }
              if (isAccepted) {
                  if (s.substr(ptr,len) == "//") {
                    commentLine = true;
                    break;
                  }
                  else if (getKind(s.substr(ptr,len)) == "NUM" && std::stol(s.substr(ptr,len)) > 2147483647) {
                    std::cerr << "ERROR";
                    break;
                  }
                  std::cout << getKind(s.substr(ptr,len)) << " " << s.substr(ptr, len) << std::endl;
                  ptr = i;
                  len = 0;
                  reset = true;
              }
              else {
                  std::cerr << "ERROR";
                  break;
              }
          }
          if (reset) {
              currentState = initialState;
              len = 0;
              reset = false;
              i -= 1;
          }
          else {
              currentState = transitions[{currentState, s[i]}];
              len++;
          }
      }
      // final token
      if (commentLine) break;
      bool isAccepted = false;
      for (size_t j = 0; j < acceptingStates.size(); j++) {
          if (acceptingStates[j] == currentState) isAccepted = true;
      }
      if (s.substr(ptr,len) == "//") break;
      else if (getKind(s.substr(ptr,len)) == "NUM" && std::stol(s.substr(ptr,len)) > 2147483647) {
        std::cerr << "ERROR";
        break;
      }
      else if (isAccepted) std::cout << getKind(s.substr(ptr,len)) << " " << s.substr(ptr, len) << std::endl;
      else std::cerr << "ERROR";
    }
  }
}
