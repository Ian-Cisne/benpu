#ifndef BENPU_SYSTEM_H_
#define BENPU_SYSTEM_H_

#include <filesystem>
#include <string>

namespace benpu {

class System {
public:
  static void printHelp();
  static std::filesystem::path getDefaultConfigurationFile();
};

}

#endif