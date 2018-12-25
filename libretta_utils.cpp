/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/


#include <iostream>
#include <locale> 
#include <iostream>
#include <string>

#include <stdlib.h>
#include <stdio.h>  // for FILENAME_MAX


#ifdef WINDOWS
#include <direct.h>
#define get_cur_dir _getcwd
#else
#include <unistd.h>
#define get_cur_dir getcwd
#endif


using namespace std;


string get_2char_locale()
{
  locale l("");
  string nm = l.name();
  string nn;
  nn += nm[0];
  nn += nm[1];
  return nn;  
}


string get_home_dir()
{
  string homedir = getenv ("HOME");
  return homedir;  
}


string current_path()
{
  char path [FILENAME_MAX];
  string result;
 
  if (! get_cur_dir (path, sizeof (path)))
     return result;

  result = path;
  return result;
}
