#include <iostream>
#include <cstring>
#include "mm4-driver.hh"

void handlePartSizeParam(char* param, mm4_driver* out) {
  int param_size = strlen(param);
  for (int i = 0; i < param_size; ++i)
    if (param[i] < '0' || param[i] > '9') {
      std::cout << "Warning: Invalid chunk_size parameter - expected positive "
                << "number but got " << param << "\n";
      return;
    }
  int tmp = atoi(param);
  if (tmp <= 0) {
    std::cout << "Warning: Invalid chunk_size parameter - number is too small\n";
    return;
  }
  out->chunk_size = tmp;
}

int main (int argc, char *argv[])
{
  mm4_driver driver;
  std::string input_code;
  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string ("-p"))
      driver.trace_parsing = true;
    else if (argv[i] == std::string ("-s"))
      driver.trace_scanning = true;
    else if (argv[i] == std::string("-verbose"))
      driver.verbose = true;
    else if (argv[i] == std::string ("-minsky"))
      driver._minsky = true;
    else if (argv[i] == std::string("-no_chunks"))
      driver.enable_chunks = false;
    else if (argv[i] == std::string("-chunk_size")) {
      if (i+1 < argc)
        handlePartSizeParam(argv[++i], &driver);
    }
    else if (argv[i] == std::string("-multifile"))
      driver.multifile_input = true;
    else if (argv[i] == std::string("-o")) {
      if (i+1 == argc) {
        std::cerr << "No base for output files specified after -o parameter\n";
        return 1;
      }
      driver.translation_out = argv[++i];
    }
    else
      input_code = argv[i];
  }

  if (!input_code.empty() && !driver.parse(input_code)) {
    driver.run();
    return 0;
  }
  return 1;
}
