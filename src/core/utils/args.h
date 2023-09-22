#ifndef BENPU_ARGS_H_
#define BENPU_ARGS_H_

#include <map>
#include <string>

namespace benpu {

enum ArgType {
  configurationFile
};

class Args {
public:

  Args(int argc, char** argv);
  bool isCorrect() const { return areArgumentsCorrect; }
private:
  friend class ConfigurationManager;
  bool areArgumentsCorrect;
  std::map<ArgType, std::string> values;
};

} //namespace benpu

#endif