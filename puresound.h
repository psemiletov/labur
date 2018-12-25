/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef PURESOUND_H
#define PURESOUND_H

#include <vector>

#include "SDL2/SDL_mixer.h"


class CSample
{
public:

  Mix_Chunk *chunk;
  int loop; //-1 = infinity, 0 = once, 1 - 2 loops, 2 - 3 loops, et cetera.

  CSample (const char *fname, int loopval);
  bool load (const char *fname);
};


class CSampleList
{
public:

  std::vector<CSample *> samples;

  ~CSampleList();
  CSample *get(); //returns random sample from the list
};


class CMusic
{
public:

  Mix_Music *music;
  int loop; //-1 = infinity, 0 = once, 1 - 2 loops, 2 - 3 loops, et cetera.

  CMusic (const char *fname, int loopval);
  ~CMusic();
  bool load (const char *fname);
};


class CSoundEngine
{
public:

  CSoundEngine();
  ~CSoundEngine();

  void play_sample (CSample *s);
  void play_music (CMusic *m);
};


#endif // PURESOUND_H
