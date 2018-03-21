#include <iostream>
#include "tm-driver.hh"

int main (int argc, char *argv[])
{
  int res = 0;
  tm_driver driver;
  for (int i = 1; i < argc; ++i)
    if (argv[i] == std::string ("-p"))
      driver.trace_parsing = true;
    else if (argv[i] == std::string ("-s"))
      driver.trace_scanning = true;
    else if (argv[i] == std::string ("-2StackPDA"))
      driver._2stackPDA = true;
    else if (!driver.parse (argv[i]))
      driver.run();
    else
      res = 1;
  return res;
}
