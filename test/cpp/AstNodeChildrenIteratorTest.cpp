#include <parser/AstIterator/AstNodeChildIterator.h>


int *toPtr(int &i) {
  return &i;
}

int first = 99;
int second = 9922;
vector<int> vec1 = {};//{1,2};
vector<int> vec2 = {22,33,44};

/*
struct X {
    auto retRange(bool x) {
      if (x)
      return join(boost::irange(0, 1)  | transformed([&](int i) {
        return &first;
      }), vec1 | transformed(toPtr));
    }
};
*/

using IntIteratorRange = IteratorRange<CounterConcatIterator<int*>, CounterConcatIteratorEnd>;

/*
template <class VALUE, class CONTAINER>
containerIter<VALUE> makeContainerIterPtr(CONTAINER &vec) {
  return containerIter<int *>(vec.size(), [&, vecIter = vec.begin()](int index) mutable {
    return &*vecIter++;
  });
}
*/

/**
 * Program entry point
 */
int main(int argc, const char **argv) {


  /*
  function<optional<int>()> func = [i = 0]() mutable -> optional<int> {
    cout << "  -- gen element -- " << i << endl;
    i++;
    if (i <= 10) {
      return i;
    }
    else {
      return nullopt;
    }
  };

  IteratorRange it(func);
   */
/*
  IntIteratorRange iter = makeValueContainerIter<int *>({&first, &second}, {
      makeItVec<int*>(vec1),
      containerIter<int *>(vec2.size(), [&](int index) mutable {
        cout << "vec2Iter i: " << index << endl;
        return &vec2[index];
      })
  });
  */
  int last = 2266;
  IntIteratorRange iter = makeValueContainerIter<int*>(
      {{&first, nullptr, &second, nullptr}},{
          makeContainerIter_ValueToPtr<int>(vec1),
          makeContainerIter_ValueToPtr<int>(vec2),
  });

  IntIteratorRange iter2 = makeValueContainerIter<int*>(
      {{&first, &second}},{
          //makeContainerIter_ValueToPtr<int>(vec1),
          //makeContainerIter_ValueToPtr<int>(vec2),
      });
  IntIteratorRange iter3 = makeValueContainerIter<int*>(
      {nullptr, &first, nullptr},{
          makeContainerIter_ValueToPtr<int>(vec1),
          makeContainerIter_ValueToPtr<int>(vec2),
      });

/*
  auto it = makeCountedRange<int*>([&, vec1Iter = vec1.begin(), vec2Iter = vec2.begin()](int index) mutable {
    if (index <= 0)
      return &first;
    if (index <= vec1.size())
      return &*vec1Iter++;
    if (index <= vec1.size()+vec2.size())
      return &*vec2Iter++;
  }, vec1.size()+vec2.size()+1);
*/
  /*
  auto it = join(boost::irange(0, 1)  | transformed([&](int i) {
    return &first;
  }), vec1 | transformed(toPtr));
  */

  //vector<int> v;
  //v.insert(v.end(), vec1.begin(), vec1.end());
  //v.insert(v.end(), vec2.begin(), vec2.end());

  cout << "elements: " << endl;
  for (auto el : iter3) {
    //(*el)++;
    cout << "el: " << *el << endl << endl;
  }
}