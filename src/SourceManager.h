#pragma once


class SourceManager {
  public:
    SourceManager(string &srcFile)
    {
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
      return lines[index - 1];
    }

  private:
    vector<string> lines;


};