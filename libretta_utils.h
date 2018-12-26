/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef LIBRETTA_UTILS_H
#define LIBRETTA_UTILS_H


#include <iostream>
#include <locale> 

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>



#ifdef WINDOWS
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif


using namespace std;

string get_2char_locale();
string get_home_dir();
string current_path();
string file_get_path (const string& fname); 
string file_get_ext (const string& fname); 
 


inline float get_fvalue (float total, float perc)
{
  return (total * perc / 100);
}


inline int get_value (int total, int perc)
{
  return static_cast <int> (total * perc / 100);
}


inline double get_percent (double total, double value)
{
  return (value / total) * 100;
}


vector <string> files_get_list (const string &path, const string &ext);


#endif