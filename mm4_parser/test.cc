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
  int res = 0;
  mm4_driver driver;
  for (int i = 1; i < argc; ++i)
    if (argv[i] == std::string ("-p"))
      driver.trace_parsing = true;
    else if (argv[i] == std::string ("-s"))
      driver.trace_scanning = true;
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
    else if (!driver.parse (argv[i]))
      driver.run();
    else
      res = 1;
  return res;
}
