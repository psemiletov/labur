#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <sstream>

#include <iostream>
#include <vector>

#include "puresound.h"
#include "graphics.h"
#include "libretta_utils.h"
#include "game_structs.h"
#include "libretta_pairfile.h"

#include "pugixml.hpp"


using namespace std;


#define DEPTH 0


#ifdef WINDOWS
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

extern int screen_width;
extern int screen_height;

int window_width;
int window_height;

extern int fps;

extern int msecs_delay;
extern int cycle;
extern int cycles_max;


extern SDL_Color color_key;

extern CPairFile *pfile;



extern SDL_Renderer *renderer;
extern SDL_Window *screen;

string dir_data;
extern string dir_fonts;
extern string dir_game_objects; //obsolete

string fname_config;

CSpace *space;


int sdl_screen_flags;


Uint32 ticks;
Uint32 next_time;

SDL_Joystick *joystick;


inline Uint32 time_left()
{
  Uint32 now = SDL_GetTicks();
  if (next_time <= now)
     {
      next_time = now + msecs_delay;
      return 0;
     }
  else
      return next_time - now;
}


char *content;
CSample *sample;

extern std::string LEVELS_DIR;


void init_paths()
{
  dir_data = current_path() + DIR_SEPARATOR + "data" + DIR_SEPARATOR;
  dir_fonts = dir_data + "fonts" + DIR_SEPARATOR;
  LEVELS_DIR = dir_data + "levels" + DIR_SEPARATOR;
  fname_config = dir_data + "config";
}


void update_game_state()
{
  if (! space)
      return;

  space->prepare_space();
  space->render_space();
}


void init_sdl()
{

  pfile = new CPairFile (fname_config.c_str());

  fps = pfile->get_int ("fps", 60);

  msecs_delay = 1000 / fps;

  cycle = 0;

  cout << "msecs_delay = " << msecs_delay << endl;
  cout << "fps = " << fps << endl;

  cycles_max = fps * 100;

  joystick = 0;

  bool bfullscreen = pfile->get_int ("fullscreen", 0);
  bool bhwsurface = pfile->get_int ("hwsurface", 1);
  bool bdoublebuf = pfile->get_int ("doublebuf", 1);
  bool basyncblit = pfile->get_int ("asyncblit", 1);

  screen_width = pfile->get_int ("screen_width", 320);
  screen_height = pfile->get_int ("screen_height", 240);

  window_width = pfile->get_int ("window_width", 800);
  window_height = pfile->get_int ("window_height", 600);


  sdl_screen_flags = SDL_WINDOW_SHOWN;
  sdl_screen_flags = sdl_screen_flags | SDL_WINDOW_OPENGL;

  if (bfullscreen)
     sdl_screen_flags = sdl_screen_flags | SDL_WINDOW_FULLSCREEN_DESKTOP;

  screen = 0;

  SDL_Init (SDL_INIT_EVERYTHING);

  screen = SDL_CreateWindow ("Laby",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              window_width, window_height,
                              sdl_screen_flags);


  if (! screen)
     {
      SDL_Quit();
      return;
     }

  renderer = SDL_CreateRenderer (screen, -1, 0); //http://wiki.libsdl.org/SDL_RendererFlags


  if (! renderer)
     {
      SDL_Quit();
      return;
     }


  if (TTF_Init() == -1)
     {
      cout << "TTF_Init: " << TTF_GetError() << endl;
      exit(2);
     }

  SDL_RenderSetLogicalSize (renderer, 320, 240);

  joystick = SDL_JoystickOpen (0);
  SDL_JoystickEventState (SDL_ENABLE);

  IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG);
  /*

  const SDL_VideoInfo *video_info = SDL_GetVideoInfo();

  cout << "SDL_GetVideoInfo()" << endl;

  cout << "hw_available: " << video_info->hw_available << endl;
  cout << "wm_available: " << video_info->wm_available << endl;

  cout << "blit_hw: " << video_info->blit_hw << endl;
  cout << "blit_hw_CC: " << video_info->blit_hw_CC << endl;
  cout << "blit_hw_A: " << video_info->blit_hw_A << endl;;
  cout << "blit_sw: " << video_info->blit_sw << endl;
  cout << "blit_sw_CC: " << video_info->blit_sw_CC << endl;
  cout << "blit_sw_A: " << video_info->blit_sw_A << endl;
  cout << "blit_fill: " << video_info->blit_fill << endl;;
  cout << "video_mem (kbytes): " << video_info->video_mem << endl;
  cout << "current_w: " << video_info->current_w <<  endl;
  cout << "current_h: " << video_info->current_h <<  endl;

  cout << "vfmt->BitsPerPixel: " << video_info->vfmt->BitsPerPixel << endl;

  */
}


void done_sdl()
{
  SDL_JoystickClose (joystick);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}


Uint32 lastTime = 0;
Uint32 currentTime = 0;
Uint32 before_cycle = 0;
Uint32 after_cycle = 0;




int main (int argc, char *argv[])
{
  init_paths();
  init_sdl();

  string_color_to_sdl_color (pfile->get_string ("color_key", "ffffff"), &color_key);

  CSoundEngine *sengine = new CSoundEngine;
  CMusic *music = new CMusic ("./372-02.ogg", 1);

  space = new CSpace (pfile->get_string ("first_level", "00"), screen_width, screen_height);




  SDL_Color temp_text_color = {255,0,0};
  CTextItem *temp_text_item = new CTextItem ("DejaVuSerif.ttf", 1, 50, "панкамхой", temp_text_color, 22);

  SDL_Color temp_text_color2 = {0,0,255};
  CTextItem *temp_text_item2 = new CTextItem ("DejaVuSerif.ttf", 1, 80, "ЛАБИРИНТ БЮРОКРАТИИ", temp_text_color2, 22);


  space->texts->items[0] = temp_text_item;
  space->texts->items[1] = temp_text_item2;


  //cout << space->texts->items[0]->text << endl;

   sample = new CSample ("./shot-carrot-01.ogg", 0);

   sengine->play_music (music);

   bool quit = false;


#ifdef __linux__
#endif


  SDL_Event keyevent;

  while (! quit)
        {
         before_cycle = SDL_GetTicks();

         if (SDL_PollEvent (&keyevent) != 0)
            {
             if (keyevent.type == SDL_KEYDOWN && keyevent.key.keysym.sym == SDLK_ESCAPE)
                {
                 quit = true;
                 break;
                }

             switch (keyevent.type)
                    {
                     case SDL_JOYAXISMOTION:
                         {
                          //keyevent.jaxis.axis
                          //keyevent.jaxis.value
                          break;
                         }

                     case SDL_KEYDOWN:
                          switch (keyevent.key.keysym.sym)
                                 {
                                  case SDLK_LEFT:
                                                 space->hero->dir_x = EDirection_Left;
                                                 break;

                                  case SDLK_RIGHT:
                                                  space->hero->dir_x = EDirection_Right;
                                                  break;

                                  case SDLK_UP:
                                               space->hero->dir_y = EDirection_Up;
                                              // space->level->bglayers->set_directions (EDirection_Right, EDirection_Up);
                                               break;

                                  case SDLK_DOWN:
                                                 space->hero->dir_y = EDirection_Down;
                                                // space->level->bglayers->set_directions (EDirection_Right, EDirection_Down);
                                                 break;

                                  case SDLK_SPACE:
                                                  sengine->play_sample (sample);
                                                  space->hero_shoot();
                                                  break;

                                  case SDLK_LCTRL:
                                                  sengine->play_sample (sample);
                                                  space->hero_shoot();
                                                  break;
                            //    case SDLK_ESCAPE:
                              //                 quit = true;
                                //               break;

                                  default:
                                          break;
                                }
                       break;

                     case SDL_KEYUP:
                                    switch (keyevent.key.keysym.sym)
                                           {
                                            case SDLK_LEFT:
                                                           space->hero->dir_x = EDirection_None;
                                                           break;

                                            case SDLK_RIGHT:
                                                           space->hero->dir_x = EDirection_None;
                                                           break;

                                            case SDLK_UP:
                                                         space->hero->dir_y = EDirection_None;
                                                  //       space->level->bglayers->set_directions (EDirection_Right, EDirection_None);
                                                         break;

                                            case SDLK_DOWN:
                                                           space->hero->dir_y = EDirection_None;
                                                    //       space->level->bglayers->set_directions (EDirection_Right, EDirection_None);
                                                           break;

                                            default:
                                                    break;
                                            }
                    }
                  }

  update_game_state();

  after_cycle = SDL_GetTicks();
  Uint32 ticks_diff = after_cycle - before_cycle;
  Uint32 new_delay = msecs_delay - ticks_diff;

  if (new_delay > 1000)
      new_delay = 1;

   if (new_delay > 0)
      SDL_Delay (new_delay);

   currentTime = SDL_GetTicks();

   if (currentTime > lastTime + 1000)
      {
       cout << new_delay << endl;
       lastTime = currentTime;
      }
  }

   cout << "EXIT !!!!!!!!!!!!!!!!!" << endl;

  delete space;

  delete sample;

  delete music;

  delete sengine;
  delete pfile;

  done_sdl();


  return 0;
}
