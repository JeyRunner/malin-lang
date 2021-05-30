#pragma once
#include <string>
#include <llvm/Support/raw_ostream.h>

using namespace std;


/**
 * Remove the prefix from the string if prefix exists.
 */
static void stringRemovePrefix(string &s, string prefix)
{
  auto found = s.find(prefix);
  if (found != string::npos){
    s.replace(found, prefix.length(), "");
  }
}

/**
 * This will invalidate the old ptr
 * When cast fails exception in thrown
 */
template<typename TO, typename FROM>
unique_ptr<TO> dynamic_unique_pointer_cast_throwing(unique_ptr<FROM>&& old) {
  auto to = dynamic_cast<TO*>(old.release());
  if (!to) {
    throw runtime_error(string("dynamic_unique_pointer_cast: invalid cast: try to cast ") + typeid(FROM).name() + " to " + typeid(FROM).name());
  }
  return unique_ptr<TO>{to};
}

/**
 * This will invalidate the old ptr
 */
template<typename TO, typename FROM>
unique_ptr<TO> dynamic_unique_pointer_cast(unique_ptr<FROM>&& old) {
  return unique_ptr<TO>(dynamic_cast<TO*>(old.release()));
}


static std::string streamInString(std::function<void(llvm::raw_ostream&)> func) {
  std::string s;
  llvm::raw_string_ostream os(s);
  func(os);
  os.flush();
  return s;
}


string toString(bool val) {
  return val ? "true" : "false";
}