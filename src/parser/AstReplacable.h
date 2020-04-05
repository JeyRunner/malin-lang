#pragma once

template<class T>
void throwAllSelfNull() {
  throw runtime_error(string("replaceNode for ") + typeid(T).name() +": all self pointers are nullptr");
}


/*
/// @deprecated use Replacable
template<class T>
void replaceNode(T *toReplace, unique_ptr<T> replaceWith) {
  if (!toReplace) {
    throw runtime_error("replaceNode called with toReplace=nullptr");
  }
  constexpr bool a = true;
  // is Expression
  if constexpr (is_base_of<Expression, T>::value) {
    if (toReplace->self) {
      *(toReplace->self) = move(replaceWith);
    }
    else if (toReplace->selfInStatement) {
      *(toReplace->selfInStatement) = move(replaceWith);
    }
    else
      throwAllSelfNull<T>();
  }
  // is not supported type
  static_assert(a, "replaceNode not supported for this type");
};
*/




/**
 * A AstNode can extend this class to provide replacement functionality.
 * Then the SetAstNodeParentAndSelfPass has also to be adapted to set the self pointers of this ast node.
 * @tparam T type of the node
 * @tparam SELF1 node can be stored in unique_ptr<SELF1>
 * @tparam SELF2 node can be stored in unique_ptr<SELF2>
 * @tparam SELF3 node can be stored in unique_ptr<SELF3>
 */
template<class T, class SELF1, class SELF2, class SELF3>
class Replacable {
  public:
    unique_ptr<SELF1> *self1 = nullptr;
    unique_ptr<SELF2> *self2 = nullptr;
    unique_ptr<SELF3> *self3 = nullptr;


    /**
     * Replace node this node with replaceWith.
     * Afterwards the original pointer to this node will be invalid.
     * All properties of the newNode 'replaceWith' have to be set manually except its self props.
     * @tparam NEW type of the new node
     * @param replaceWith replace 'this' with this node
     * @throws runtime_error when node could not be replaced because self props were not assigned to this node
     * @return pointer to the new node that replaced the old one
     */
    template<class NEW>//, enable_if_t<is_base_of<T, NEW>::value>>
    //using NEW = T;
    NEW *replaceNode(unique_ptr<NEW> replaceWith) {
      //T *nodeOld = replaceWith.get();
      //ASTNode *nodeAstNew = nullptr;
      NEW *newNode = replaceWith.get();

      // replace self that is not null with 'replaceWith'
      if (self1) {
        newNode->self1 = this->self1; // set self of new node to self of old node
        // @todo better use self1.swap(replaceWith)
        *(self1) = move(replaceWith); // replace old with new
      }
      else if (self2) {
        newNode->self2 = this->self2; // set self of new node to self of old node
        *(self2) = move(replaceWith); // replace old with new
      }
      else if (self3) {
        newNode->self3 = this->self3; // set self of new node to self of old node
        *(self3) = move(replaceWith); // replace old with new
      }
      else {
        throwAllSelfNull<T>();
      }
      return newNode;
    }

    bool selfSet() {
      return self1 || self2 || self3;
    }
};