#pragma once

#include "util/util.h"

class ClassDeclaration;



/**
 * Types
 */
class LangType {
  public:
    virtual std::unique_ptr<LangType> clone() const = 0;

    virtual void print(int depth) {
      cout << /*ASTNode::depthToTabs(depth) <<*/ "Type(" << toString() << ")" << endl;
    }

    virtual string toString() = 0;

    virtual bool equals(LangType *other) = 0;

    virtual bool isInvalid()        { return false; }
    virtual bool isVoidType()       { return false; }
    virtual bool isBuildInType()       { return false; }
    virtual bool isNumericalType()  { return false; }
    virtual bool isBooleanType()  { return false; }
    virtual bool isClassType()      { return false; }

    virtual ~LangType() = default;
};

class InvalidType: public LangType {
  public:
    unique_ptr<LangType> clone() const override{
      return std::unique_ptr<InvalidType>();
    }
    string toString() override{
      return "InvalidType";
    }
    bool equals(LangType *other) override {
      return false;
    }
    bool isInvalid() override
    { return true; }
};


class ReferenceType: public LangType {
  public:
    unique_ptr<LangType> innerType;

    ReferenceType(unique_ptr<LangType> innerType) : innerType(move(innerType))
    {}

    ReferenceType(const ReferenceType &original) : innerType(original.innerType->clone())
    { }

    std::unique_ptr<LangType> clone() const override {
      return make_unique<ReferenceType>(*this);
    }

    string toString() override {
      return "Reference<"+ innerType->toString() +">";
    }
    bool equals(LangType *other) override {
      if (auto* o = dynamic_cast<ReferenceType*>(other)) {
        return o->innerType->equals(this->innerType.get());
      }
      return false;
    }
};


class UserDefinedType: public LangType {
};

class ClassType: public UserDefinedType {
  public:
    ClassDeclaration *classDeclaration;

    explicit ClassType(ClassDeclaration *classDecl): classDeclaration(classDecl)
    { }

    std::unique_ptr<LangType> clone() const override {
      return make_unique<ClassType>(*this);
    }

    string toString() override;

    bool equals(LangType *other) override {
      if (auto* o = dynamic_cast<ClassType*>(other)) {
        return o->classDeclaration == this->classDeclaration;
      }
      return false;
    }

    bool isClassType() override {
      return true;
    }
};



enum BUILD_IN_TYPE {
    BuildIn_No_BuildIn = -1,

    BuildIn_i32,
    BuildIn_f32,

    BuildIn_void,
    BuildIn_bool,
    // static string (has contend and length)
    BuildIn_str, // @todo will be replaced by predefined str class
};
class BuildInType: public LangType {
  public:
    BUILD_IN_TYPE type = BuildIn_No_BuildIn;

    BuildInType() {}
    BuildInType(BUILD_IN_TYPE type) : type(type)
    {}

    bool equals(LangType *other) override{
      if (auto* o = dynamic_cast<BuildInType*>(other)) {
        return o->type == this->type;
      }
      return false;
    }

    virtual std::unique_ptr<LangType> clone() const override{
      return make_unique<BuildInType>(*this);
    }

    string toString() override
    {
      string t = string(magic_enum::enum_name(type));
      stringRemovePrefix(t, "BuildIn_");
      return t;
    }

    bool isVoidType() override {
      return type == BuildIn_void;
    }

    bool isBuildInType() override {
      return true;
    }

    bool isNumericalType() override {
      switch (type) {
        case BuildIn_i32:
        case BuildIn_f32:
          return true;
        default:
          return false;
      }
    }

    virtual bool isBooleanType()  { return type==BuildIn_bool; }
};


string buildInTypeToString(BUILD_IN_TYPE type) {
  switch (type) {
    case BuildIn_No_BuildIn:
      return "No_BuildIn";
    case BuildIn_i32:
      return "i32";
    case BuildIn_f32:
      return "f32";
    case BuildIn_void:
      return "void";
    case BuildIn_bool:
      return "bool";
    case BuildIn_str:
      return "str";
  }
}