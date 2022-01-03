#pragma once
#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <termcolor/termcolor.hpp>
#include <lyra/lyra.hpp>

using namespace std;
using namespace lyra;
namespace fs = std::experimental::filesystem;


class SourceManager {
  public:
    void setSource(fs::path &filePath, string &srcFile) {
      this->filePath = filePath;

      // split lines
      // @todo ignore '\r'
      auto ss = std::stringstream(srcFile);
      for (std::string line; std::getline(ss, line);) {
        lines.push_back(line);
      }
    }

    /**
     * first line is 1
     */
    string getLine(int index) {
      if (index - 1 < lines.size()) {
        return lines[index - 1];
      }
      else {
        return "";
      }
    }

    string getFilePathString() {
      return fs::canonical(filePath).string();
    }

  private:
    vector<string> lines;
    fs::path filePath;

};

static SourceManager sourceManager;