#ifndef PARSESTRING
#define PARSESTRING

#include <vector>
#include <string>
#include <iostream>

using namespace std;

//parse 'sep' delimited string
vector<string> parseString(string str, string sep)
{
  vector<string> parsed;
  int pos = 0;
  bool first = true;
  if (str.size() == 0) return parsed;
  if (str.find(sep) == string::npos)
  {
    parsed.push_back(str);
    return parsed;
  }
  while (true)
  {
    int newPos = str.find(sep, pos);
    if (str.find(sep, pos) == string::npos)
    {
      if (!first) parsed.push_back(str.substr(pos, newPos-pos));
      break;
    }
    string sub = str.substr(pos, newPos-pos);
    parsed.push_back(sub);
    pos = newPos+1;
    first = false;
  }
  return parsed;
}

#endif
