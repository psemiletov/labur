#include <iostream>
#include <sstream>
#include <fstream>

#include "graphics.h"
#include "libretta_pairfile.h"
#include "libretta_utils.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;


int fps;
int msecs_delay;
int cycle;
int cycles_max;


SDL_Color color_key;
SDL_Window *screen;
SDL_Renderer *renderer;

string dir_fonts;


//теперь полный путь имя вместо имени файла
SDL_Texture *load_image (std::string fname)
{
  string name = fname;
  SDL_Surface *image = IMG_Load (name.c_str());
  if (! image)
     {
      cout << "IMG_Load: " << IMG_GetError() << endl;
      return 0;
     }

  SDL_Texture *t = SDL_CreateTextureFromSurface (renderer, image);
  SDL_FreeSurface (image);

  return t;
}


SDL_Texture *load_image_keyed (std::string fname, SDL_Color key)
{
  string name = fname;
  SDL_Surface *image = IMG_Load (name.c_str());
  if (! image)
     {
      cout << "IMG_Load: " << IMG_GetError() << endl;
      return 0;
     }

  SDL_SetColorKey (image, SDL_TRUE, SDL_Color_to_Uint32_color (image, key));
  SDL_Texture *t = SDL_CreateTextureFromSurface (renderer, image);
  SDL_FreeSurface (image);

  return t;
}




//ДОБАВИТЬ, чтобы если имя файла имеет расширение PNG
//то просто грузить с keyed и скоростью по умолчанию

CSprite::CSprite (std::string fname)
{
//  cout << "CSprite::CSprite - start: " << fname <<  endl;

  current_frame = 0;
  cycles_per_frame = 1;
  empty = true;
  original = true;
  keyed = 0;

  int speed = 2;

  if (fname.empty())
     {
      cout << "CSprite::CSprite cannot load, fname is empty" << endl;
      return;
     }

  if (fname == "#COPY")
     {
      cout << "SPRITE COPY MODE" << endl;
      original = false; 
      return;
     }


  cout << "creating sprite: " << fname << endl;

  if (file_get_ext (fname) == "png")
     { 
      set_fps (speed);
      add_image (fname, keyed);
      return;
     }

  CPairFile pfile (fname);
  speed = pfile.get_int ("speed", 2);
  set_fps (speed);
  keyed = pfile.get_int ("keyed", 0);


  string fnames = pfile.get_string ("files", "null");

  istringstream iss (fnames);
  string word;
  while (getline (iss, word, ','))
       {
        add_image (file_get_path (fname) + word, keyed);
       }

  //cout << "CSprite::CSprite - end: " << fname <<  endl;
}


CSprite::~CSprite()
{
  std::vector <SDL_Texture *>::iterator it;

  if (original)
      for (it = images.begin(); it != images.end(); ++it)
          {
           if (*it)
              SDL_DestroyTexture (*it);
          }
}


//FIXME to the percent value instead of ...
void CSprite::set_fps (int speed)
{
  cycles_per_frame = fps / speed;
}


int CSprite::get_current_frame_number()
{
  if (cycle % cycles_per_frame == 0)
     current_frame++;

  if (current_frame == (int)images.size())
     current_frame = 0;

  return current_frame;
}


void CSprite::add_image (string fname, bool use_colorkey)
{
  empty = false;

  SDL_Texture *t = 0;

  if (use_colorkey)
     t = load_image_keyed (fname, color_key);
  else
     t = load_image (fname);

  if (t)
     images.push_back (t);
}


CSprite* CSprite::create_copy()
{
  CSprite *s = new CSprite (string ("#COPY"));

  s->current_frame = 0;
  s->cycles_per_frame = cycles_per_frame;
  s->keyed = keyed;

  for (int i = 0; i < (int)images.size(); i++)
      {
       SDL_Texture *sf = images[i];
       s->images.push_back (sf);
      }

  return s;
}

//FIXME заменить на хэш-таблицу
int string_to_direction (string dir)
{
  if (dir == "EDirection_None")
     return (int)EDirection_None;
  else
  if (dir == "EDirection_Left")
     return (int)EDirection_Left;
  else
  if (dir == "EDirection_Right")
     return (int)EDirection_Right;
  else
  if (dir == "EDirection_None")
     return (int)EDirection_None;
  else
  if (dir == "EDirection_Up")
     return (int)EDirection_Up;
  else
  if (dir == "EDirection_Down")
     return (int)EDirection_Down;

  return -1;
}


void CTexts::draw_all()
{
  if (items.size() > 0)
     for (int i = 0; i < items.size(); i++)
         SDL_RenderCopy (renderer, items[i]->text_surface, 0, &items[i]->dest);
}


CTexts::CTexts()
{
/*  std::string tf = dir_fonts + font_name;
  font = TTF_OpenFont (tf.c_str(), 16);
  if (! font)
     {
      cout << "TTF_OpenFont: " << TTF_GetError();
       // handle error
     }*/
}


CTexts::~CTexts()
{
  if (items.size() > 0)
     for (int i = 0; i < items.size(); i++)
         delete items[i];
}


CTextItem::~CTextItem()
{
  if (text_surface)
    SDL_DestroyTexture(text_surface);

  if (font)
     TTF_CloseFont (font);
}


void CTextItem::set_pos (int pos_x, int pos_y)
{
  dest.x = pos_x;
  dest.y = pos_y;
}


CTextItem::CTextItem (std::string font_name, int pos_x, int pos_y, std::string text_value, SDL_Color item_color, int vsize)
{
  color = item_color;
  size = vsize;
  text_changed = false;
  text_surface = 0;
  font = 0;

  rect.w = 0;
  rect.h = 0;

  rect.x = 0;
  rect.y = 0;

  dest.h = 0;
  dest.w = 0;

  set_pos (pos_x, pos_y);

  std::string tf = dir_fonts + font_name;
  font = TTF_OpenFont (tf.c_str(), size);
  if (! font)
     {
      cout << "TTF_OpenFont: " << TTF_GetError();
       // handle error
     }

  set_text (text_value);
}


void CTextItem::set_text (std::string text_value)
{
  text = text_value;
  text_changed = false;

  if (text_surface)
     SDL_DestroyTexture (text_surface);

  SDL_Surface *t = TTF_RenderUTF8_Solid (font, text.c_str(), color);

  text_surface = SDL_CreateTextureFromSurface (renderer, t);

  rect.w = t->w;
  rect.h = t->h;

  dest.w = t->w;
  dest.h = t->h;

  SDL_FreeSurface (t);
}


void CTextItem::set_text (char *text_value)
{
  text = text_value;
  text_changed = false;

  if (text_surface)
     SDL_DestroyTexture (text_surface);

  SDL_Surface *t = TTF_RenderUTF8_Solid (font, text.c_str(), color);

  text_surface = SDL_CreateTextureFromSurface (renderer, t);

  dest.w = t->w;
  dest.h = t->h;

  SDL_FreeSurface (t);

}


bool sdl_rects_intersects (const SDL_Rect *a, const SDL_Rect * b)
{
  int bx2 = b->x + b->w;
  int by2 = b->y + b->h;

 //is top left in b

  if ((a->x >= b->x && a->x <= bx2) && ((a->y >= b->y && a->y <= by2)))
      return true;

 //is bottom left in b

  if ((a->x >= b->x && a->x <= bx2) && ((a->y + a->h >= b->y && a->y + a->h <= by2)))
      return true;


  //is bottom right in b

   if ((a->x + a->w >= b->x && a->x + a->w <= bx2) && ((a->y + a->h >= b->y && a->y + a->h <= by2)))
      return true;

 //is top right in b

    if ((a->x + a->w >= b->x && a->x + a->w <= bx2) && ((a->y >= b->y && a->y <= by2)))
      return true;


  return false;
}


bool sdl_rect_intersects_with (const SDL_Rect *one, const SDL_Rect *other)
{
  int bx2 = other->x + other->w;
  int by2 = other->y + other->h;

 //is top left in b

  if ((one->x >= other->x && one->x <= bx2) && ((one->y >= other->y && one->y <= by2)))
      return true;

 //is bottom left in b

  if ((one->x >= other->x && one->x <= bx2) && ((one->y + one->h >= other->y && one->y + one->h <= by2)))
      return true;


  //is bottom right in b

   if ((one->x + one->w >= other->x && one->x + one->w <= bx2) && ((one->y + one->h >= other->y && one->y + one->h <= by2)))
      return true;

 //is top right in b

    if ((one->x + one->w >= other->x && one->x + one->w <= bx2) && ((one->y >= other->y && one->y <= by2)))
      return true;


  return false;
}


void string_color_to_sdl_color (std::string scolor, SDL_Color *color)
{
  char r[3];
  char g[3];
  char b[3];

  scolor.copy (r, 2, 0);
  scolor.copy (g, 2, 2);
  scolor.copy (b, 2, 4);

  r[2] = '\0';
  g[2] = '\0';
  b[2] = '\0';

//  cout << r << ":" << g << ":" << b << endl;

  unsigned long int ur = strtoul (r, NULL, 16);
  unsigned long int ug = strtoul (g, NULL, 16);
  unsigned long int ub = strtoul (b, NULL, 16);

  color->r = ur;
  color->g = ug;
  color->b = ub;

  color->a = 255;

}


Uint32 SDL_Color_to_Uint32_color (SDL_Surface *surface, SDL_Color color)
{
  return SDL_MapRGB (surface->format, color.r, color.g, color.b);
}

