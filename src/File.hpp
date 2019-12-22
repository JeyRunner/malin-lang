#pragma once

#include<iostream>
#include <experimental/filesystem>
#include <fstream>

using namespace std;
namespace fs = std::experimental::filesystem;



/*
 * File class
 */
class File
{
  public:
    static string readFile(fs::path filePath) {
      ifstream stream(filePath);
      if (!stream) {
        throw runtime_error("can't open file '" + filePath.string() + "'");
      }

      ostringstream oss;
      oss << stream.rdbuf();
      return oss.str();
    }
};


