#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup.hpp>

#include "configuration.h"

static void init_log() {
  boost::log::add_console_log(std::cout,
    boost::log::keywords::format = "[%TimeStamp%]: %Message%"
  );

    // Only output message with INFO or higher severity in Release
#ifndef _DEBUG
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::info
    );
#endif

}

int main() {
  init_log();

  int a[10];

  for(int i = 0; i < 10; ++i) {
    a[i] = i * 2;
  }

  char string[] = "holaaa";

  return 0;    
}
