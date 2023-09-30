#include <string>

#include "core/utils/args.h"

namespace benpu {

Args::Args(int argc, char* argv[]) {
  argv++; argc--;

  while (argc > 0) {

    if (argv[0][0] != '-' || argv[0][2] != '\0' ) {
      areArgumentsCorrect = false;
      return;
    }

    switch (argv[0][1]) {
    case 'c':
      argc--;
      if (argc == 0) {
        areArgumentsCorrect = false;
        return;
      }
      argv++;
      values[ArgType::configurationFile] = std::string(argv[0]);
      break;
    default:
      areArgumentsCorrect = false;
      return;
    }
    argv++;
    argc--;
  }

  areArgumentsCorrect = true;
}

std::string Args::getValue(ArgType argType) {
  return values[argType];
}

} //namespace benpu
