/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#include <iostream>
#include "puresound.h"

using namespace std;


CSoundEngine::CSoundEngine()
{
//  int flags = MIX_INIT_OGG | MIX_INIT_MOD;
  int flags = MIX_INIT_OGG;
  int initted = Mix_Init (flags);
  if ((initted & flags) != flags)
     {
      cout << "Mix_Init: Failed to init required ogg support!" << endl;
      cout << "Mix_Init: " << Mix_GetError() << endl;
     }

  Mix_OpenAudio (48000, MIX_DEFAULT_FORMAT, 2, 4096);
  Mix_AllocateChannels (32);
}


CSoundEngine::~CSoundEngine()
{
  Mix_CloseAudio();
  Mix_Quit();
}


void CSoundEngine::play_sample (CSample *s)
{
  if (! s)
     return;

  Mix_PlayChannel (-1, s->chunk, s->loop);
}


bool CSample::load (const char *fname)
{
  bool result = true;

  chunk = Mix_LoadWAV (fname);

  if (! chunk)
     {
      cout << "cannot load " << fname << endl;
      result = false;
     }

  return result;
}


CSample::CSample (const char *fname, int loopval)
{
  load (fname);
  loop = loopval;
}


CMusic::CMusic (const char *fname, int loopval)
{
  load (fname);
  loop = loopval;
}


bool CMusic::load (const char *fname)
{
  bool result = true;

  music = Mix_LoadMUS (fname);

  if (! music)
     {
      cout << "cannot load " << fname << endl;
      result = false;
     }

  return result;
}


CMusic::~CMusic()
{
  Mix_FreeMusic (music);
}


void CSoundEngine::play_music (CMusic *m)
{
  if (! m)
     return;

  Mix_HaltMusic();
  Mix_PlayMusic (m->music, m->loop);
}


CSampleList::~CSampleList()
{
  std::vector <CSample *>::iterator it;

  for (it = samples.begin(); it != samples.end(); ++it)
      {
       if (*it)
          delete *it; 
      }
}


CSample *CSampleList::get()
{
  int i = rand() % samples.size();
  return samples.at (i);
}  
