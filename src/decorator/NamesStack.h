#pragma once

#include <stack>
#include <map>
#include <utility>
using namespace std;

class AstLink {
  public:
    ASTNode *node;
};
class NamesScope {
  public:
    /**
     * insert new name
     * @return true when insert successful and false when name already exists
     */
    bool addName(const string& name, ASTNode &node) {
      if (findName(name) == nullptr) {
        namesMap.insert({name, AstLink{&node}});
        return true;
      }
      else {
        return false;
      }
    }

    ASTNode * findName(string name) {
      auto it = namesMap.find(name);
      if (it != namesMap.end()) {
        return it->second.node;
      }
      else {
        return nullptr;
      }
    }

  private:
    map<string, AstLink> namesMap;
};

class NamesStack {
  public:
    NamesScope& addNamesScope() {
      return scopes.emplace_back(NamesScope());
    }

    void removeNamesScope(const NamesScope& scope) {
      auto it = find_if(scopes.begin(), scopes.end(), [&](const NamesScope& other) {return &other == &scope;});
      if (it == scopes.end()) {
        throw runtime_error("while decorating: tried to remove a NamesScope that did not exist");
      }
      scopes.erase(it);
    }

    ASTNode * findName(string name) {
      for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
        ASTNode * node = it->findName(name);
        if (node != nullptr) {
          return node;
        }
      }
      return nullptr;
    }

  private:
    list<NamesScope> scopes;
};