/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef GAMESTRUCTS_H
#define GAMESTRUCTS_H

#include <string>
#include <map>

#include "graphics.h"


using namespace std;


enum ELevelGoal
    {
     ECT_LIMIT_POS_X = 0,
     ECT_KILL_BOSS
    };


enum EObjType
{
  OBJTYPE_UTIL = 0,
  OBJTYPE_PORTAL, //1
  OBJTYPE_LEVELITEM, //2
  OBJTYPE_ENEMY, //3
  OBJTYPE_POWERUP, 
  OBJTYPE_WALL,//5
};


enum ECollisionType
    {
     ECT_NONE = 0,
     ECT_NOMOVE, //1
     ECT_ADD_LIFE, //2
     ECT_SUBTRACT_LIFE, //3
     ECT_FALL, //4
     ECT_BLOW, //5
     ECT_KILL, //6
     ECT_PORTAL //7
    };


enum EMovementType
    {
     EMT_VOID = -1,
     EMT_NONE, //0
     EMT_LEFT, //1
     EMT_RIGHT, //2
     EMT_UP, //3
     EMT_DOWN //4
    };


class CCollision
{
public:

  ECollisionType type;
  int value;

  CCollision() {type = ECT_NONE; value = 0;};
};


class CLevel;

class CGameObject
{
 public:

  CLevel *level; //uplink

  std::vector <CSprite *> sprites;

  string objname;
  string missilename;
  string linkedmap; //для портала

  int can_shot; //может ли стрелять?
  int shot_freq; //как часть? в чем выразить - выстрелы в секунду, или выстрел на таком-то цикле?

  int object_type;
  int speed;
  int speed_mult;

  EMovementType movement_type;

  EDirection dir_x;
  EDirection dir_y;

  SDL_Rect rect; //position and size

  CCollision collision; //действие, которое оказывает этот объект при столкновении с другим объектом

  CGameObject (CLevel *lvl, string name);
  CGameObject (CLevel *lvl); //для "невидимого" объекта вроде портала

  void move();

  void set_speed (int a_speed);

  CGameObject* create_copy (CLevel *lvl);
};


class CHero: public CGameObject
{
 public:

  void move();
};


class CLevel
{
 public:

  std::vector <CGameObject *> game_objects_pool; //used for "cloning" objects from there
  std::vector <CSprite *> sprites_pool;
  std::vector <CGameObject *> game_objects; //not pool
  std::vector <CGameObject *> map_objects; //not pool

  std::vector <SDL_Rect *> walls;

  map <string, int> map_sprites;
  map <string, int> map_game_objects;


  int map_tiles[15][20];


  ELevelGoal level_goal;

  string level_name;
  string level_path;
  string dir_levels;

  string dir_pix;

  string fname_tmx;

  string next_level;

 // int level_goal_pos_x;
 // int width;
//  int height;

  int map_width;
  int map_height;

  int tile_width;
  int tile_height;

  void load_tmx (std::string fname);

  void load_sprites_pool (std::string level_name);
  void load_game_objects_pool (std::string level_name);
  void load_settings (std::string level_name);

  void load_game_objects (std::string level_name);
  void reset_map_objects();

  CLevel (std::string level_name, std::string dir_lvls);
  ~CLevel();
};


class CSpace
{
 public:

  CLevel *level;
  CHero *hero;
  CTexts *texts;

  CSpace (string lvl_name, int w, int h);
  ~CSpace();

  void load_level (string lvl_name);

  void create_hero();
  void hero_shoot();

  void prepare_space();
  void render_space();
};


#endif
