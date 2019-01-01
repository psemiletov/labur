#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <SDL2/SDL.h>


#include "game_structs.h"
#include "graphics.h"
#include "libretta_pairfile.h"
#include "libretta_utils.h"

#include "pugixml.hpp"



#define MAX_CHAR_SPRITES 16


std::string LEVELS_DIR;


/*
 about character_pool and character indexes
 the first one is ALWAYS == 1
 */

int screen_width;
int screen_height;


extern int fps;
extern int cycles_max;
extern int cycle;

extern SDL_Renderer *renderer;

CPairFile *pfile;



class Ctmx_walker: public pugi::xml_tree_walker
{
public:

  CLevel *lvl;

  bool begin (pugi::xml_node &node);
  bool end (pugi::xml_node &node);
  bool for_each (pugi::xml_node& node);
};


bool Ctmx_walker::begin (pugi::xml_node &node)
{
 // std::cout << "begin node name = " << node.name() << std::endl;
  return true;
}


bool Ctmx_walker::end (pugi::xml_node &node)
{
//  std::cout << "end node name = " << node.name() << std::endl;
  return true;
}


bool Ctmx_walker::for_each (pugi::xml_node &node)
{
  if (node.type() != pugi::node_element)
      return true;

  //std::cout << "for_each name = " << node.name() << std::endl;

  if (strcmp (node.name(), "map") == 0)
     {
      //read map attrs
       pugi::xml_attribute attr = node.attribute ("width");   
       lvl->map_width = attr.as_int();   
       attr = node.attribute ("height");   
       lvl->map_height = attr.as_int();   

       attr = node.attribute ("tileheight");   
       lvl->tile_height = attr.as_int();   
       attr = node.attribute ("tilewidth");   
       lvl->tile_width = attr.as_int();   
     }


  if (strcmp (node.name(), "objectgroup") == 0)
     {
      //load objects

      std::cout << "!!!!!!!!!!!!!!!!!!!! LOAD OBJECTS !!!!!!!!!!!!!!!!!!!!" << endl;    

      for (pugi::xml_node obj: node.children("object"))
          {
           std::cout << "obj.name:" << obj.name() << endl;
         
           for (pugi::xml_node child: obj.children())
               {
            //   std::cout << "child.name:" << child.name() << endl;
               }

           pugi::xml_attribute attr = obj.attribute ("type"); 
           std::cout << "attr.value():" << attr.value() << endl;
 
           string atr = attr.value();    

           if (atr == "text")
           
           //if (strcmp (attr.value(), "text" == 0))
              {
               std::cout << "TEXT!!!!" << attr.name() << "=" << attr.value() << endl;
              }
 

           if (atr == "wall")
              {
               std::cout << "WALL!!!!" << attr.name() << "=" << attr.value() << endl;
 
               SDL_Rect *r = new SDL_Rect;
 
               r->x = node.attribute ("x").as_int(); 
               r->y = node.attribute ("y").as_int(); 
               r->w = node.attribute ("width").as_int(); 
               r->h = node.attribute ("height").as_int(); 

               lvl->walls.push_back (r);

              }


//           for (pugi::xml_attribute attr: obj.attributes())
    //          {
      //          std::cout << " " << attr.name() << "=" << attr.value() << endl;
        //     }  
        }

     }


  if (strcmp (node.name(), "layer") == 0)
     {
      pugi::xml_node datanode = node.child ("data");

      //parse datanode.child_value() to tiles map
      std::stringstream data_stream (datanode.child_value());
      std::string line;

      int row = 0; 
     
//csv начинается с перевода строки, поэтому пропускаем при разборе
      //нулевой ряд, он пустой

      std::getline (data_stream, line, '\n');

      while (std::getline (data_stream, line, '\n')) 
            {

             cout << "ROW: " << row << endl;

             std::stringstream line_stream (line);
             std::string str_col;

             int col = 0;               
  
             while (std::getline (line_stream, str_col, ',')) 
                   {
                    int colval = std::stoi (str_col);  
                  //  *(lvl->map_tiles + (row * lvl->map_width + col)) = colval; 
                    lvl->map_tiles[row][col] = colval;
                    cout << colval << ",";
                    col++; 
                   }

             cout << endl;

              row++;
              if (row == lvl->map_height)
                 break; 



             }

     }


  return true;
}


//НЕ ЗАБУДЬ! ПРИ ЧТЕНИИ КАЖДОГО ТАЙЛА НАДО ID+1
void CLevel::load_tmx (std::string fname)
{
  std::cout << "CLevel::load_tmx - START" << std::endl;

  std::string filename = level_path + fname; 

  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_file (filename.c_str());

//  std::cout << "Load result: " << result.description() << ", map: " << doc.child("map").attribute("version").value() << std::endl;

  Ctmx_walker walker;
  walker.lvl = this;

  doc.traverse (walker);


  std::cout << "CLevel::load_tmx - END" << std::endl;
}


void CLevel::load_settings (std::string level_name)
{
  cout << "CLevel::load_settings: " << level_name << endl;

  string fname_settings = level_path + "settings";
 
  CPairFile pf (fname_settings);

  level_goal = (ELevelGoal) pf.get_int ("level_goal");

//  cout << "level_goal = " << (int)level_goal << endl;

//  width = pf.get_int ("width");
//  height = pf.get_int ("height");


  screen_width = pf.get_int ("width");
  screen_height = pf.get_int ("height");

  level_name = pf.get_string ("level_name");
  next_level = pf.get_string ("next_level");

  fname_tmx = pf.get_string ("fname_tmx");

 // level_goal_pos_x = pf.get_int ("level_goal_pos_x");


  //cout << "level_name = " << level_name << endl;

  cout << "CLevel::load_settings - ok" << endl;
}


void CLevel::load_game_objects_pool (std::string level_name)
{
  cout << "CLevel::load_game_objects_pool - start \n";
 
  string fname_game_objects = level_path + "game_objects.idx"; //index file

 // cout << "fname_chars = " << fname_chars << endl;

  ifstream infile (fname_game_objects.c_str());

  if (! infile)
     {
      cout << "Could not open file: " << fname_game_objects << endl;
      return;
     }

  //cout << "parse characters index file" << endl;

  string line;
  int idx = 0;

  while (getline (infile, line))
        {
         idx++;
         game_objects_pool[idx] = new CGameObject (this, level_path + line);
         map_game_objects.insert (make_pair (line, idx));
        }

  infile.close();

  cout << "CLevel::load_game_objects_pool - end \n";
}


CLevel::CLevel (std::string level_name, std::string dir_lvls)
{
  cout << "CLevel::CLevel - start" << endl;

  game_objects_pool.resize (1024, nullptr);
  sprites_pool.resize (1024, nullptr);
  game_objects.resize (1024, nullptr);

  dir_levels = dir_lvls;
  level_path = dir_levels + level_name + DIR_SEPARATOR;
  dir_pix = level_path + "pix" + DIR_SEPARATOR;

  cout << "loading level " << level_path << endl;
  cout << "dir_pix " << dir_pix << endl;

  load_settings (level_name);
  load_sprites_pool (level_name);
  load_game_objects_pool (level_name);
  load_game_objects (level_name);
  load_tmx (fname_tmx);

  cout << "CLevel() - end"  << endl;
};


CLevel::~CLevel()
{
  cout << "~CLevel() - start" << endl;

  if (sprites_pool.size() > 0)
     for (size_t i = 0; i < sprites_pool.size(); i++)
         if (sprites_pool[i])
             delete sprites_pool[i];

  if (game_objects_pool.size() > 0)
     for (size_t i = 0; i < game_objects_pool.size(); i++)
         delete game_objects_pool[i];

 
  if (game_objects.size() > 0)
     for (size_t i = 0; i < game_objects.size(); i++)
         delete game_objects[i];

  if (walls.size() > 0)
     for (size_t i = 0; i < walls.size(); i++)
         delete walls[i];


  cout << "~CLevel() - end" << endl;
}



CGameObject::CGameObject (CLevel *lvl, string name)
{
  cout << "CGameObject::CGameObject start: " << name << endl;

  sprites.resize (MAX_CHAR_SPRITES, nullptr);

  if (name.empty())
     return;

  CPairFile pf (name); //name for example = fullpath + hero.her

//  cout << "level_path + name: " << level_path << name << endl;

  set_speed (pf.get_int ("speed", 30));
  rect.x = pf.get_int ("x", 0);
  rect.y = pf.get_int ("y", 0);
  rect.w = pf.get_int ("w", 32);
  rect.h = pf.get_int ("h", 32);

  missilename = pf.get_string ("missilename");
  speed_mult = pf.get_int ("speed_mult", 1);

  can_shot = pf.get_int ("can_shot", 0);

  float tf = pf.get_float ("shot_freq", 0.25);

  shot_freq = fps / tf;

  //cout << "shot_freq: " << shot_freq <<  endl;


  movement_type = (EMovementType)pf.get_int ("movement_type", 0);
  collision.type = (ECollisionType)pf.get_int ("collision.type", 0);
  collision.value = pf.get_int ("collision.value", 0);


  objname = name.substr(name.find_last_of(DIR_SEPARATOR) + 1);

  cout << "NAME: " << name << endl;
  cout << "OBJNAME: " << objname << endl;

  //initial direction
  dir_x = (EDirection) string_to_direction (pf.get_string ("dir_x", "EDirection_None"));
  dir_y = (EDirection) string_to_direction (pf.get_string ("dir_y", "EDirection_None"));


  //cout << "objname = " << objname << endl;
  //cout << "missilename = " << missilename << endl;
  //cout << "movement_type = " << movement_type << endl;

  string dir_EDirection_None = pf.get_string ("EDirection_None");
  string dir_EDirection_Left = pf.get_string ("EDirection_Left");
  string dir_EDirection_Right = pf.get_string ("EDirection_Right");
  string dir_EDirection_Up = pf.get_string ("EDirection_Up");
  string dir_EDirection_Down = pf.get_string ("EDirection_Down");

  sprites[EDirection_None] = lvl->sprites_pool[lvl->map_sprites[dir_EDirection_None]]->create_copy();
  sprites[EDirection_Left] = lvl->sprites_pool[lvl->map_sprites[dir_EDirection_Left]]->create_copy();
  sprites[EDirection_Right] = lvl->sprites_pool[lvl->map_sprites[dir_EDirection_Right]]->create_copy();
  sprites[EDirection_Up] = lvl->sprites_pool[lvl->map_sprites[dir_EDirection_Up]]->create_copy();
  sprites[EDirection_Down] = lvl->sprites_pool[lvl->map_sprites[dir_EDirection_Down]]->create_copy();

  //dir_x = EXDirection_None;
  //dir_y = EYDirection_None;

  cout << "CGameObject::CGameObject end" << endl;
}


void CGameObject::CGameObject::move()
{

 //cout << "CGameObject::move()" << endl;
   if (dir_x != EDirection_None)
     {
      if (cycle % speed == 0)
         {
          if (dir_x == EDirection_Left)
             rect.x -= speed_mult;
          else
          if (dir_x == EDirection_Right)
              rect.x += speed_mult;
          }
     }

  if (dir_y != EDirection_None)
    {
     if (cycle % speed == 0)
       {
        if (dir_y == EDirection_Up && rect.y > 0)
           rect.y -= speed_mult;
        else
        if (dir_y == EDirection_Down && rect.y < (screen_height - rect.h))
           rect.y += speed_mult;

        //cout << "y = " << rect.y << endl;
       }
    }

   if (rect.y < 0)
      rect.y++;

   if (rect.x < 0)
      rect.x++;

}


void CGameObject::set_speed (int a_speed)
{
  speed = a_speed;
}


void CLevel::load_sprites_pool (std::string level_name)
{
  cout << "CLevel::load_sprites_pool\n";

  string fname_sprites = level_path + "sprites.idx"; //index file

//  cout << "fname_sprites = " << fname_sprites << endl;

  ifstream infile (fname_sprites.c_str());

  if (! infile)
     {
      cout << "Could not open file:" << fname_sprites  << endl;
      return;
     }

  cout << "parse sprites index file" << endl;

  string line;

  while (getline (infile, line))
        {
          size_t pos = line.find ("=");
         
          if (pos == string::npos)
              continue;
         
          string fname; //fname
          string b; //value
        
          fname = line.substr (0, pos);
          b = line.substr (pos + 1, line.size() - pos);

          size_t idx = atoi (b.c_str());
        
          sprites_pool[idx] = new CSprite (dir_pix + fname);
          map_sprites.insert (make_pair (fname, idx));
         }

  infile.close();
}


CGameObject* CGameObject::create_copy (CLevel *lvl)
{
  cout << "CCharacter::create_copy() - start" << endl;

  CGameObject *obj = new CGameObject (lvl, "");

  for (size_t i = 0; i < sprites.size(); i++)
      if (sprites[i])
          obj->sprites[i] = sprites[i]->create_copy();

  obj->speed = speed;

  obj->can_shot = can_shot;
  obj->shot_freq = shot_freq;

  obj->objname = objname;
  obj->collision = collision;
  obj->object_type = object_type;
  obj->movement_type = movement_type;
  obj->missilename = missilename;
  obj->speed_mult = speed_mult;

  obj->dir_x = dir_x;
  obj->dir_y = dir_y;

  obj->rect.x = rect.x;
  obj->rect.y = rect.y;
  obj->rect.w = rect.w;
  obj->rect.h = rect.h;

  cout << "CCharacter::create_copy() - end" << endl;

  return obj;
}


inline bool rect_in_view (SDL_Rect view, SDL_Rect rect)
{
  bool result = false;

  if (rect.x >= view.x && rect.y >= view.y
      && rect.x <= view.x + view.w && rect.y <= view.h
      && rect.x + rect.w <= view.x + view.w
      && rect.y + rect.h <= view.y + view.h
     )
     result = true;

  return result;
}


void CHero::move()
{
  //cout << "CHero::move()" << endl;


//  cout << "x = " << rect.x << endl;
  //      cout << "y = " << rect.y << endl;

  if (dir_x != EDirection_None)
     {
      if (cycle % speed == 0)
         {
          if (dir_x == EDirection_Left)
             rect.x--;
          else
              if (dir_x == EDirection_Right)
                rect.x++;
         }
     }
 
  if (dir_y != EDirection_None)
     {
      if (cycle % speed == 0)
        {
         if (dir_y == EDirection_Up && rect.y > 0)
            rect.y--;
         else
             if (dir_y == EDirection_Down && rect.y < (screen_height - rect.h))
                 rect.y++;

       }
    }


   if (rect.y < 0)
      rect.y++;

   if (rect.x < 0)
      rect.x++;


   if ((rect.y + rect.h) > screen_height)
      rect.y--;

   if ((rect.y + rect.w) > screen_width)
      rect.x--;

}


void CSpace::create_hero()
{
  int idx = level->map_game_objects["hero.chr"];

  hero = (CHero*)level->game_objects_pool[idx]->create_copy (level);
  if (! hero)
     return;

 

  cout << "CSpace::create_hero() = ok" << endl;
}


//load objects
//ЗАМЕНИТЬ, БУДЕТ ИЗ ФАЙЛА КАРТЫ
void CLevel::load_game_objects (std::string level_name)
{
  cout << "CLevel::load_game_objects\n";

  string fname_objmap = level_path + "objmap";

  ifstream infile (fname_objmap.c_str());

  if (! infile)
     {
  //   cout << "Could not open file " << fname_objmap << endl;
      return;
     }

  string line;

  while (getline (infile, line))
        {
         //parse line

         size_t pos1 = line.find (",");

         if (pos1 == string::npos)
            continue;

         size_t pos2 = line.find (",", pos1 + 1);

         if (pos2 == string::npos)
            continue;

    //     cout << "pos1 = " << pos1 << " pos2 = " << pos2 << endl;

         string objname;
         string b;
         string c;

         objname = line.substr (0, pos1);
         b = line.substr (pos1 + 1, pos2 - pos1);
         c = line.substr (pos2 + 1, line.size() - pos2);

//         cout << "objname = " << objname << " b = " << b << " c = " << c << endl;

         int idx = map_game_objects[objname];

  //       cout << "idx = " << idx << endl;

         CGameObject *temp_obj = game_objects_pool[idx];

         if (! temp_obj)
	    {
	     cout << "! temp_obj" << endl;
	     continue;
	    }

         CGameObject *obj = temp_obj->create_copy (this);
         if (! obj)
            continue;

         obj->rect.x = atoi (b.c_str());
         obj->rect.y = atoi (c.c_str());

         game_objects.push_back (obj);
        }


  infile.close();

  cout << "CLevel::load_game_objects - ok" << endl;
}


void CSpace::hero_shoot()
{
 cout << "CHero::shoot() - start" << endl;

  int idx = level->map_game_objects[hero->missilename];
  CGameObject *temp_obj = level->game_objects_pool[idx];

  if (! temp_obj)
     {
      cout << "! temp_obj" << endl;
      return;
     }

  CGameObject *obj = temp_obj->create_copy (level);
  if (! obj)
     return;

  obj->rect.x = hero->rect.x + hero->rect.w;
  obj->rect.y = hero->rect.y + hero->rect.h / 2;

  level->game_objects.push_back (obj);

  cout << "CSpace::shoot() - end" << endl;
}



void CSpace::load_level (string lvl_name)
{
 cout << "CSpace::load_level: " << lvl_name << endl;

  if (level)
      {
       delete level;
       level = nullptr;
      }  

  if (hero)
     {
      delete hero;
      hero = nullptr; 
     } 

  level = new CLevel (lvl_name, LEVELS_DIR);

  create_hero();

  cout << "CSpace::load_level - end" << endl;
}


CSpace::CSpace (string lvl_name, int w, int h)
{
  cout << "CSpace::CSpace - start" << endl;
  level = 0;
  hero = 0;

  texts = new CTexts;

  load_level (lvl_name);

  cout << "CSpace::CSpace - end" << endl;
}


CSpace::~CSpace()
{
  cout << "CSpace::~CSpace() - start" << endl;

  if (texts)
     delete texts;

  if (hero)
    delete hero;

  if (level)
     delete level;
  
   cout << "CSpace::~CSpace() - end" << endl;
}



void CSpace::prepare_space()
{
 // if (! hero)

   // cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
        

  if (hero)
     {
      }


 // cout << "OBJECT COLLISIONS/MOVE - START" << endl;
/*
  for (size_t i = 0; i < level->game_objects.size(); i++)
      {
       CGameObject *obj = level->game_objects[i];

       if (obj)
          {
 //COLLISIONS
            if (sdl_rect_intersects_with (&hero->rect, &obj->rect))
                if (obj->collision.type == ECT_FALL)
                   {
                    obj->dir_x = EDirection_None;
                    obj->dir_y = EDirection_Down;
                   }


              //for each object check collision
               //if (ch->rect.x)

            for (size_t x = 0; x < level->game_objects.size(); x++)
                {
                 CGameObject *t = level->game_objects[x];

                 if (sdl_rect_intersects_with (&obj->rect, &t->rect))
                 if (t->collision.type == ECT_FALL && obj->collision.type == ECT_KILL) //тестовая проверка, надо убрать будет
                    {
                     obj->dir_x = EDirection_None;
                     obj->dir_y = EDirection_Down;
                    }
                }

 //MOVEMENT
         obj->move();
       }


       	

  }
*/
 //cout << "prepare_space() - end" << endl;
}



void CSpace::render_space()
{
  cycle++;

  if (cycle == cycles_max)
     cycle = 0;


//рисуем карту из карты тайлов

   SDL_Rect tilerect;
   tilerect.x = 0;
   tilerect.y = 0;
   tilerect.w = level->tile_width;
   tilerect.h = level->tile_height;


   for (int row = 0; row < level->map_height; row++)
       {
     //  cout << "ROW: " << row << endl;
    
       for (int col = 0; col < level->map_width; col++)
           {
            int id = level->map_tiles[row][col];
            //int id = *(level->map_tiles + (row * level->width + col));

    
            CSprite *sprite = level->sprites_pool[id];
            if (sprite)
               {
                SDL_Texture *tile = sprite->images[0];
                //SDL_Texture *im = sprite->images[sprite->get_current_frame_number()];

                 SDL_Rect dstrect; 
                 dstrect.w = level->tile_width;
                 dstrect.h = level->tile_height;
                 dstrect.y = level->tile_height * row;
                 dstrect.x = level->tile_width * col;
  
                 if (tile)
                     SDL_RenderCopy (renderer, tile, &tilerect, &dstrect);
                 }
           }
         }

//рисуем героя
  hero->move();

  CSprite *sprite = hero->sprites[(int)hero->dir_x]; //FIXME: ИСПОЛЬЗОВАТЬ ТЕКУЩЕЕ DIRECTION

  SDL_Texture *im = sprite->images[sprite->get_current_frame_number()];
  if (im)
     SDL_RenderCopy (renderer, im, 0, &hero->rect);

//рисуем объекты 
  for (size_t i = 0; i < level->game_objects.size(); i++)
     {
      CGameObject *obj = level->game_objects[i];
      if (obj)
         {
          CSprite *sprite = obj->sprites[(int)obj->dir_x]; //FIXME: ИСПОЛЬЗОВАТЬ ТЕКУЩЕЕ DIRECTION
          SDL_Texture *im = sprite->images[sprite->get_current_frame_number()];
          if (im)
              SDL_RenderCopy (renderer, im, 0, &obj->rect);
         }
     }


  texts->draw_all();

  SDL_RenderPresent (renderer);
}

