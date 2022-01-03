#pragma once
#include "ir/IRModule.h"
#include "ir/IRValueVar.h"
#include <set>


/**
 * Ensures uniqueness of ir names that are printed by the IRPrinter.
 */
class IRNamesScope
{
  public:
    map<string, int>  valueNamesLast;
    map<IRElement*, string> valueNames;

    explicit IRNamesScope() {
    }


    virtual /**
     * Delete all stored value names.
     */
    void restNames() {
      valueNamesLast.clear();
      valueNames.clear();
    }


    /**
     * Registers a ir element and returns its new name
     * @param value
     * @return the new name
     */
    string createIRElementName(IRElement &irElement) {
      string name = irElement.name;

      auto n = valueNamesLast.find(irElement.name);
      if (n == valueNamesLast.end()) {
        valueNamesLast.emplace(irElement.name, 0);
        // when name is empty start with 0
        if (name.empty()) {
          name = to_string(0);
        }
      }
      else {
        n->second++;
        name += to_string(n->second);
      }
      valueNames.emplace(&irElement, name);
      return name;
    }


    /**
     * Get name of the IRElement. If not existing name is found a new one is created.
     * @return name string if its found in stored value names
     */
    string getName(IRElement* irElement) {
      auto valStr = valueNames.find(irElement);
      if (valStr == valueNames.end()) {
        return createIRElementName(*irElement);
      }
      return valStr->second;
    }
};


/**
 * Ensures uniqueness of ir value names that are printed by the IRPrinter.
 */
class ValueNamesScope: protected IRNamesScope
{
  public:
    char valueSymbol;

    explicit ValueNamesScope(char valueSymbol) : valueSymbol(valueSymbol) {
    }


    /**
     * Delete all stored value names.
     */
    void restNames() override {
      IRNamesScope::restNames();
    }


    /**
     * Registers a new value and returns its new name as '%<NAME>: TYPE = '
     * @param value
     * @return
     */
    string createValueDeclStr(IRValue &value, bool hasInitValue = true) {
      string out;

      if (holds_alternative<IRTypeVoid>(((IRValue&)value).type)) {
        return "";
      }

      string name = createIRElementName(value);

      /*
      auto n = valueNamesLast.find(value.name);
      if (n == valueNamesLast.end()) {
        valueNamesLast.emplace(value.name, 0);
        // when name is empty start with 0
        if (name.empty()) {
          name = to_string(0);
        }
      }
      else {
        n->second++;
        name += to_string(n->second);
      }
      valueNames.emplace(&value, name);
       */

      out += valueSymbol + name + ": " + irTypeToString(value.type);
      if (hasInitValue) {
        out += " = ";
      }
      return out;
    }


    /**
     * Get name and type of value that is used as a string.
     * @return value name string if its found in stored value names
     */
    optional<string> getValueStr(IRValueVar* value) {
      auto valStr = valueNames.find((IRValue*) value);
      if (valStr == valueNames.end()) {
        return nullopt;
      }
      //return irTypeToString(value->type) + " " + "%" + valueNamesFunction[(IRValue*) value];
      auto typeStr = irTypeToString(((IRValue*)value)->type);
      return typeStr + " " + valueSymbol + valueNames[(IRValue*) value];
    }
};



