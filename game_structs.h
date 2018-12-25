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


enum ECollisionType
    {
     ECT_NONE = 0,
     ECT_ADD_LIFE, //1
     ECT_SUBTRACT_LIFE, //2
     ECT_FALL, //3
     ECT_BLOW, //4
     ECT_KILL //5
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

  std::vector <CSprite *> sprites;

  string objname;
  string missilename;

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

  int level_goal_pos_x;
  int width;
  int height;

  int map_width;
  int map_height;

  int tile_width;
  int tile_height;

  void load_tmx (std::string fname);

  void load_sprites_pool (std::string level_name);
  void load_game_objects_pool (std::string level_name);
  void load_settings (std::string level_name);

  void load_game_objects (std::string level_name);


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
