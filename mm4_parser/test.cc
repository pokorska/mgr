#include <iostream>
#include "mm4-driver.hh"

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
    else if (!driver.parse (argv[i]))
      driver.run();
    else
      res = 1;
  return res;
}
