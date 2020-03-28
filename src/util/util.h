#pragma once


/**
 * Remove the prefix from the string if prefix exists.
 */
void stringRemovePrefix(string &s, string prefix)
{
  auto found = s.find(prefix);
  if (found != string::npos){
    s.replace(found, prefix.length(), "");
  }
}


template<typename TO, typename FROM>
unique_ptr<TO> dynamic_unique_pointer_cast (unique_ptr<FROM>&& old){
  return unique_ptr<TO>{dynamic_cast<TO*>(old.release())};
  //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
}


std::string streamInString(std::function<void(llvm::raw_ostream&)> func) {
  std::string s;
  llvm::raw_string_ostream os(s);
  func(os);
  os.flush();
  return s;
}