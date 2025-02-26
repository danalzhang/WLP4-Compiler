#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include<string>
#include <memory> 
#include <utility>

class ll {
     public:
     std::string key = "";
     std::string val = "";
     std::vector<std::shared_ptr<ll>> next;
     int rule = -1;

};

void dfs(std::shared_ptr<ll> point) {
     std::cout << point->key << " " << point->val << std::endl;
     int size = point->next.size();
     for (int i = 0; i < size; i++) {
          dfs(point->next[i]);
     }
}
int main() {
     std::istringstream iss(WLP4_COMBINED);
     std::string text;

     // .CFG
     getline(iss, text);
     
     std::vector<std::vector<std::string>> rules;
     // Rules
     getline(iss,text);
     
     while (text != ".TRANSITIONS") {
          
          std::istringstream meow(text);
          std::string word;
          std::vector<std::string> transitionpiece;
          
          meow >> word;
          transitionpiece.emplace_back(word);

          while (meow >> word) {
               transitionpiece.emplace_back(word);
          }
          rules.emplace_back(transitionpiece);
          getline(iss,text); 
     }

    
     std::vector<std::vector<std::string>> transitions;
     getline(iss,text);
     while (text != ".REDUCTIONS") {
          std::istringstream meow(text);
          std::string word;
          std::vector<std::string> transitionpiece;
          
          meow >> word;
          transitionpiece.emplace_back(word);
          meow >> word;
          transitionpiece.emplace_back(word);
          meow >> word;
          transitionpiece.emplace_back(word);

          transitions.emplace_back(transitionpiece);
          getline(iss,text); 
     }
     
     std::vector<std::vector<std::string>> reductions;
     
     while (getline(iss,text)) {
          if (text == ".END") break;
          std::istringstream meow(text);
          std::string word;
          std::vector<std::string> transitionpiece;
          
          meow >> word;
          transitionpiece.emplace_back(word);
          meow >> word;
          transitionpiece.emplace_back(word);
          meow >> word;
          transitionpiece.emplace_back(word);

          reductions.emplace_back(transitionpiece);
     }
     // Do actions
     std::vector<std::shared_ptr<ll>> input;
     std::shared_ptr<ll> head = std::make_shared<ll>();
     head->val = "BOF";
     head->key = "BOF";
     input.emplace_back(head);

     while (getline(std::cin, text)) {
          std::istringstream meow(text);
          std::shared_ptr<ll> head = std::make_shared<ll>();
          meow >> text;
          head->key = text;
          meow >> text;
          head->val = text;
          input.emplace_back(head);
     }

     std::shared_ptr<ll> heade = std::make_shared<ll>();
     heade->val = "EOF";
     heade->key = "EOF";
     input.emplace_back(heade);
     
     std::vector<std::string> stateStack;
     std::vector<std::shared_ptr<ll>> leStack;
     stateStack.emplace_back("0");
     for (size_t bookmark = 0; bookmark < input.size(); bookmark++){

          while (true) {
               bool reduce = false;
               int SSsize = stateStack.size();
               int rule = -1;
               for (auto vec : reductions) {
                    if (vec[0] == stateStack[stateStack.size()-1] && vec[2] == input[bookmark]->key) {
                         reduce = true;
                         rule = stoi(vec[1]);
                         break;
                    }
               }
               if (!reduce) {
                    break;
               }
               
               int remove = rules[rule].size() - 1;
               

               std::shared_ptr<ll> head = std::make_shared<ll>();
               
               if (remove == 1 && rules[rule][1] == ".EMPTY") {
                    head->val = ".EMPTY";
                    remove = 0;
               }
               head->key = rules[rule][0];
               for (int i = 1; i < remove; i++) {
                    head->val += rules[rule][i] + " ";
               }
               if (remove > 0) {
                    head->val += rules[rule][remove];
               }

               for (int i = leStack.size() - remove; i < leStack.size(); i++) {
                         head->next.emplace_back(leStack[i]);
               }
               for (int i = 0; i < remove; i++)  {
                    stateStack.pop_back();
                    leStack.pop_back();
               }
               
               leStack.emplace_back(head);
               bool check = false;
               SSsize = stateStack.size();
               for (auto vec : transitions) {
                    if (vec[0] == stateStack[SSsize-1] && vec[1] == leStack[leStack.size() - 1]->key) {
                         stateStack.emplace_back(vec[2]);
                         check = true;
                         break;
                    }
               }

               if (!check) {
                    std::cerr<< "ERROR at " << bookmark - 1 << std::endl;
                    return 0;
               }

          }

          leStack.emplace_back(input[bookmark]);

          bool pushed = false;
          for (auto vec : transitions) {
               if (vec[0] == stateStack[stateStack.size()-1] && vec[1] == input[bookmark]->key) {
                    stateStack.emplace_back(vec[2]);
                    pushed = true;
                    break;
               }
          }

          if (!pushed) {
               std::cerr<< "ERROR at " << bookmark <<  std::endl;
               
               return 0;
          }
     }
     
     std::shared_ptr<ll> header = std::make_shared<ll>();
     header->key = "start";
     header->val = "BOF procedures EOF";
     for (size_t i = 0; i < leStack.size(); i++) {
          header->next.emplace_back(leStack[i]);
     }
     
     dfs(header);
     
     return 0;
}
