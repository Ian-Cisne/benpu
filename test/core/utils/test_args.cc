
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "core/utils/args.h"

char executable[] = "benpu"; 

BOOST_AUTO_TEST_CASE( test_normal_execution ) {

  char c[] = "-c"; 
  char config[] = "config.json";
  char* argv[] = {
    executable,
    c,
    config
  };

  benpu::Args args(3, argv);

  BOOST_CHECK( args.isCorrect() );

}

BOOST_AUTO_TEST_CASE( test_execution_without_arg ) {

  char* argv[] = {
    executable
  };

  benpu::Args args(1, argv);

  BOOST_CHECK( args.isCorrect() );

}

BOOST_AUTO_TEST_CASE( test_execution_with_incorrect_arg_1 ) {

  char d[] = "-de"; 
  char config[] = "config.json";

  char* argv[] = {
    executable,
    d,
    config
  };

  benpu::Args args(3, argv);

  BOOST_CHECK_MESSAGE( !args.isCorrect(), "The argumens should be incorrect." );

}

BOOST_AUTO_TEST_CASE( test_execution_with_incorrect_arg_2 ) {

  char d[] = "-d"; 

  char* argv[] = {
    executable,
    d
  };

  benpu::Args args(2, argv);

  BOOST_CHECK_MESSAGE( !args.isCorrect(), "The argumens should be incorrect." );

}