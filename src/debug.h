#include <ctime>
#include <stdio.h>
#include <string>

template<class... T>
void dbgln(std::string str, T&&... args) 
{
  std::time_t t = std::time(0);
  std::tm* now = std::localtime(&t);

  std::string date_template = "%d-%02d-%02d, %02d:%02d:%02d [Debug] ";
  std::string print_template = date_template + str + '\n';

  printf(print_template.c_str(), now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, std::forward<T>(args)...);
}
