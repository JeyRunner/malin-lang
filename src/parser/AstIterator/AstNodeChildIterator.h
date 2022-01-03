#pragma once

#include <array>
#include <tuple>
#include <iostream>
#include <optional>
#include <functional>

using namespace std;

class ASTNode;


template<typename BEGIN, typename END>
class IteratorRange
{
  public:
    explicit IteratorRange(const BEGIN begin, const END end) : childIter(move(begin)), endIter(end)
    {}

    BEGIN begin() const {
      return childIter;
    }
    END end() const {
      return endIter;
    }

  private:
    BEGIN childIter;
    END endIter;
};


template<typename T>
class GeneratorIterator : public std::iterator<std::input_iterator_tag, const T>
{
  public:
    /// end iter
    GeneratorIterator() : currentValue(nullopt) {
    }

    explicit GeneratorIterator(const function<optional<T>()> nextFunction) : nextFunction(move(nextFunction)) {
      currentValue = nextFunction();
    }

    T &operator*() {
      return *currentValue;
    }
    T *operator->() {
      return &(*currentValue);
    }
    GeneratorIterator &operator++() {
      currentValue = nextFunction();
      return *this;
    }
    bool operator==(GeneratorIterator &other) const {
      cout << "called == " << endl;
      return this->currentValue == other.currentValue;
    }
    bool operator!=(GeneratorIterator &other) const {
      cout << "called != " << endl;
      return this->currentValue != other.currentValue;
    }

  private:
    function<optional<T>()> nextFunction;
    optional<T> currentValue;
};


template<typename T>
class CounterIterator : public std::iterator<std::input_iterator_tag, const T>
{
  public:
    /// end iter
    CounterIterator(int iterations) : index(iterations+1) {
    }

    explicit CounterIterator(const function<T(int index)> nextFunction) : nextFunction(move(nextFunction)), index(0) {
      currentValue = nextFunction(index);
      index++;
    }

    T &operator*() {
      return currentValue;
    }
    T *operator->() {
      return &(currentValue);
    }
    CounterIterator &operator++() {
      currentValue = nextFunction(index);
      index++;
      return *this;
    }
    bool operator==(CounterIterator &other) const {
      //cout << "called == " << endl;
      return this->index == other.index;
    }
    bool operator!=(CounterIterator &other) const {
      //cout << "called i:"<< index <<" != other.i:" << other.index << endl;
      return this->index != other.index;
    }

  private:
    function<T(int)> nextFunction;
    T currentValue;
    int index = 0;
};

template <typename T>
IteratorRange<CounterIterator<T>, CounterIterator<T>> makeCountedRange(const function<T(int index)> nextFunction, int iterations) {
  return IteratorRange(CounterIterator(move(nextFunction)), CounterIterator<T>(iterations));
}


template<typename T>
struct containerIter {
    function<T(int)> nextValue;
    int valuesAmount;
    int i = 0;
    containerIter(int valuesAmount, function<T(int)> nextValue) : nextValue(move(nextValue)), valuesAmount(valuesAmount) {
    }
    T getNext() {
      return this->nextValue(i++);
    };

    bool atEnd() {
      return i >= valuesAmount;
    }
};


struct CounterConcatIteratorEnd {
};


/**
 * Iterate over the given values and containerIterators.
 * If type T is a pointer type nullptrs in values will be skipt.
 */
template<typename T>
class CounterConcatIterator : public std::iterator<std::input_iterator_tag, const T>
{
  public:
    /// end iter
    explicit CounterConcatIterator()
        : values(move(values)), containerIters(0) {
    }

    explicit CounterConcatIterator(const vector<T> values, const vector<containerIter<T>> containerIters)
    : values(move(values)), containerIters(move(containerIters)) {
      toNext(true);
    }

    T &operator*() {
      return currentValue;
    }
    T *operator->() {
      return &(currentValue);
    }
    CounterConcatIterator &operator++() {
      toNext();
      return *this;
    }
    void toNext(bool isInit = false) {
      if (valueIndex < values.size()) {
        if (!isInit) {
          valueIndex++;
        }
        if (valueIndex < values.size())
        {
          currentValue = values[valueIndex];
          // skip nullptr is T is pointer
          if constexpr (is_pointer<T>::value) {
            if (currentValue == nullptr) {
              // to next
              if (!isAtEnd()) {
                toNext();
              }
            }
          }
          return;
        }
      }
      // when finished with values
      if (containerItersIndex < containerIters.size()) {
        // to next containerIter
        if (containerIters[containerItersIndex].atEnd()) {
          containerItersIndex++;
          toNext();
        }
        // get next value from current containerIter
        else {
          currentValue = containerIters[containerItersIndex].getNext();
        }
      }
    }

//    // equal when both finished
//    bool operator==(CounterConcatIterator &other) const {
//      return (!isAtEnd() && !other.isAtEnd());
//    }
//    bool operator!=(CounterConcatIterator &other) const {
//      //cout << "called i:"<< index <<" != other.i:" << other.index << endl;
//      return (isAtEnd() && other.isAtEnd());
//    }

    // equal when finished
    bool operator==(CounterConcatIteratorEnd &other) const {
      // cout << "called == for CounterConcatIteratorEnd" << endl;
      return isAtEnd();
    }
    bool operator!=(CounterConcatIteratorEnd &other) const {
      // cout << "called != for CounterConcatIteratorEnd" << endl;
      return !isAtEnd();
    }

    bool isAtEnd() const {
      return containerItersIndex >= containerIters.size() && valueIndex >= values.size();
    }

  private:
    vector<T> values;
    vector<containerIter<T>> containerIters;
    T currentValue;
    int valueIndex = 0;
    int containerItersIndex = 0;
};


/**
 * Create iterator over values and multiple containers.
 * Usage like:
 * auto iter = makeValueContainerIter<int>(
 *     {1, 2},{
 *     makeContainerIter<int>(vec1),
 *     makeContainerIter<int>(vec2)
 * });
 * for (int el : iter)
 *   cout << el;
 *
 * or:
 *   IntIteratorRange iter = makeValueContainerIter<int *>({&first, &second}, {
 *     makeItVec<int*>(vec1),
 *     containerIter<int *>(vec2.size(), [&, iter = vec2.begin()](int index) mutable {
 *       return &*iter++;
 *     })
 *   });
 *
 * @param values fixed value to begin iteration with, it T is a pointer type nullptr values will be skipt
 * @param containerIters after values iterate sequential over all containerIters
 */
template <typename T>
IteratorRange<CounterConcatIterator<T>, CounterConcatIteratorEnd> makeValueContainerIter(const vector<T> &values, const vector<containerIter<T>> &containerIters) {
  return IteratorRange(
      CounterConcatIterator<T>(move(values), move(containerIters)),
      CounterConcatIteratorEnd()
  );
}

/* NOT WORKING
template <class VALUE, class CONTAINER_VALUE, class CONTAINER>
containerIter<VALUE> makeContainerIter(CONTAINER &vec, function<VALUE(CONTAINER_VALUE&)> mapValues) {
  return containerIter<VALUE>(vec.size(), [&, vecIter = vec.begin()](int index) mutable {
    return mapValues(*vecIter++);
  });
}
*/
template <class VALUE, class CONTAINER>
containerIter<VALUE> makeContainerIter(CONTAINER &vec) {
  return containerIter<VALUE>(vec.size(), [&, vecIter = vec.begin()](int index) mutable {
    return *vecIter++;
  });
}

/**
 * Iterate over given contain and map smart_ptr<VALUE> to smart_ptr<VALUE>.get()
 */
template <class VALUE, class CONTAINER>
containerIter<VALUE*> makeContainerIter_SmartPtrToPtr(CONTAINER &vec) {
  return containerIter<VALUE*>(vec.size(), [&, vecIter = vec.begin()](int index) mutable {
    auto val = vecIter->get();
    vecIter++;
    return val;
  });
}

/**
 * Iterate over given contain and map VALUE to &VALUE
 */
template <class VALUE, class CONTAINER>
containerIter<VALUE*> makeContainerIter_ValueToPtr(CONTAINER &vec) {
  return containerIter<VALUE*>(vec.size(), [&, vecIter = vec.begin()](int index) mutable {
    return &*vecIter++;
  });
}


/* NOT WORKING
template <class VALUE>
function<VALUE*(VALUE&)> toPointer() {
  return [](VALUE& value) {return &value;};
}
*/


/// ast children range iterator

using AstIterator = CounterConcatIterator<ASTNode*>;
using AstChildRange = IteratorRange<CounterConcatIterator<ASTNode*>, CounterConcatIteratorEnd>;
using astIterGen = containerIter<ASTNode*>;

/**
 * Create iterator over values and multiple containers.
 * Usage like:
 * auto iter = makeAstRange(
 *     {1, 2},{
 *     makeContainerIter(statements, uPtrToAstNodePointer<Statement>())
 * });
 *
 * @param values fixed value to begin iteration with, it T is a pointer type nullptr values will be skiped
 * @param containerIters after values iterate sequential over all containerIters
 */
static AstChildRange makeAstRange(const vector<ASTNode*>& values, const vector<astIterGen>& containerIters) {
  return IteratorRange(
      CounterConcatIterator<ASTNode*>(values, containerIters),
      CounterConcatIteratorEnd()
  );
}

static AstChildRange makeAstRange(const vector<ASTNode*>& values) {
  return IteratorRange(
      CounterConcatIterator<ASTNode*>(values, {}),
      CounterConcatIteratorEnd()
  );
}


/* NOT WORKING
template <class VALUE>
function<ASTNode*(VALUE&)> toAstNodePointer() {
  return [](VALUE& value) {return &value;};
}


template <class VALUE>
function<ASTNode*(unique_ptr<VALUE>& value)> uPtrToAstNodePointer() {
  return [](unique_ptr<VALUE>& value) {return value.get();};
}


template <class VALUE>
ASTNode* uPtrToAstNodePointer(unique_ptr<VALUE>& value) {
  return value.get();
}
*/
