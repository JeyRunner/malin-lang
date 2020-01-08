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