/******************************************************************************
 * Copyright (c) 2015-2016, GraphSQL Inc.
 * All rights reserved.
 * Project: GraphSQL Query Language
 * udf.hpp: a library of user defined functions used in queries.
 *
 * - Supported type of return value and parameters
 *     - int
 *     - float
 *     - double
 *     - bool
 *     - string (don't use std::string)
 *     - accumulators
 *
 * - Function names are case sensitive, unique, and can't be conflict with
 *   built-in math functions and reserve keywords.
 *
 * - Please don't remove necessary codes in this file
 *
 * Created on: Nov 05, 2015
 * Author: Zixuan Zhuang
 ******************************************************************************/

#ifndef EXPRFUNCTIONS_HPP_
#define EXPRFUNCTIONS_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <string>
typedef std::string string; //XXX DON'T REMOVE

/****** BIULT-IN FUNCTIONS **************/
/****** XXX DON'T REMOVE ****************/
inline int str_to_int (string str) {
  return atoi(str.c_str());
}

inline int float_to_int (float val) {
  return (int) val;
}

inline double str_to_double(string str) {
  return atof(str.c_str());
}

inline string to_string (double val) {
  std::stringstream ss;
          ss << std::fixed << std::setprecision(400) << val;
              return ss.str();
}

inline string replaceStr(string &str, string src, string tgt) {
  boost::replace_all(str, src, tgt);
  
  std::cout << "std:" << str << std::endl;

  return str;
}

inline void GetKeyVal (string input, string &key, string &val) {
  std::vector<std::string> strs;
  boost::split(strs, input, boost::is_any_of(","));

  if (strs.size() != 2) {
    key = "";
    val = "0";
    return;
  }

  key = strs[0];
  val = strs[1];
}

inline int rdn(int y, int m, int d) { 
      if (m < 3)
                y--, m += 12;
          return 365*y + y/4 - y/100 + y/400 + (153*m - 457)/5 + d - 306;
}

inline int getDayDiff (int day1, int day2) {

  return rdn(day1/10000, (day1%10000)/100, day1%100) - rdn(day2/10000, (day2%10000)/100, day2%100);
} 

inline bool enrollAbove21 (int bday, int eday) {
  return bday+200000<eday;
}

/****************************************/

#endif /* EXPRFUNCTIONS_HPP_ */
