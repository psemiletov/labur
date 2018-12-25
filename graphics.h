/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <string>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


enum EDirection
{
  EDirection_None = 0, 
  EDirection_Left, 
  EDirection_Right,
  EDirection_Up, 
  EDirection_Down
};


enum ELayerType
    {
     LAYER_TYPE_COLOR = 0,  
     LAYER_TYPE_SOLID,  
     LAYER_TYPE_TILED  
    };


    
class CSprite
{
public:

  std::vector <SDL_Texture *> images;
  
  bool empty;
  bool keyed;
  
  bool original;

  int cycles_per_frame; //game loop cycles per frame

  int current_frame;

  int get_current_frame_number();

  CSprite (std::string fname);
  ~CSprite();
  
  void add_image (std::string fname, bool use_colorkey);
  CSprite* create_copy();
  
  void set_fps (int speed);
};


class CTextItem
{
public:
 
  int x;
  int y;
  int size; //font size
  
  SDL_Rect dest;
  SDL_Rect rect;
  
  bool text_changed;
  
  TTF_Font *font;
  SDL_Texture *text_surface;
  
  SDL_Color color;
  std::string text;
  
  CTextItem (std::string font_name, int pos_x, int pos_y, std::string text_value, SDL_Color item_color, int vsize);
  ~CTextItem();
  void set_text (std::string text_value);
  void set_text (char *text_value);

  void set_pos (int pos_x, int pos_y);
  
};


typedef std::map <int, CTextItem *> TMapTextItems;


class CTexts
{
public:
 
 //тут хэш объектов-текстов, которые надо ренредить
  //TTF_Font *font;
 
  TMapTextItems items;
   
  CTexts();
  ~CTexts();
  
  void draw_all();
};





SDL_Texture *load_image (std::string fname);
SDL_Texture *load_image_keyed (std::string fname, SDL_Color key);
SDL_Texture *load_image_keyed (std::string fname, Uint32 key);


int string_to_direction (std::string dir);

//bool sdl_rects_intersects (const SDL_Rect *a, const SDL_Rect * b);
//Uint32 string_color_to_sdl_color (std::string scolor);

void string_color_to_sdl_color (std::string scolor, SDL_Color *color);
//Uint32 string_color_to_Uint32_color (std::string scolor);

Uint32 SDL_Color_to_Uint32_color (SDL_Surface *surface, SDL_Color color);


bool sdl_rect_intersects_with (const SDL_Rect *one, const SDL_Rect *other);



#endif