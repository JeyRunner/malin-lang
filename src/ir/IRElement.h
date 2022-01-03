#pragma once
#include <string>

#include <utility>
#include <variant>
using namespace std;


/**
 * Intermediate representation base class.
 */
class IRElement {
  public:
    string name;

    IRElement() {
    }

    IRElement(string name) : name(std::move(name)) {
    }
};



