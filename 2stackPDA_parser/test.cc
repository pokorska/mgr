#include <iostream>
#include "2stackPDA-driver.hh"

int main (int argc, char *argv[])
{
  int res = 0;
  _2stackPDA_driver driver;
  for (int i = 1; i < argc; ++i)
    if (argv[i] == std::string ("-p"))
      driver.trace_parsing = true;
    else if (argv[i] == std::string ("-s"))
      driver.trace_scanning = true;
    else if (argv[i] == std::string ("--debug"))
      driver.debug = true;
    else if (argv[i] == std::string ("-minsky"))
      driver._minsky = true;
    else if (argv[i] == std::string ("-translateTo")) {
      driver._minsky = true;
      driver.direct_translation = true;
      if (i+1 < argc) driver.translation_file = argv[i+1];
      else std::cout << "Warning: No translation filename provided. "
                     << "Translating to output.mm4";
      i++;
    } else if (argv[i] == std::string ("-multifile")) {
      driver._minsky = true;
      driver.direct_multifile_mode = true;
      if (i+1 < argc) driver.multifile_base = argv[i+1];
      else std::cout << "Warning: No translation filename provided. "
                     << "Translating to output/base*";
      i++;
    }
    else if (!driver.parse (argv[i]))
      driver.run();
    else
      res = 1;
  return res;
}
