#ifndef USERS_H
#define USERS_H

typedef struct {
  char name[25];
  char passwd[25];
  int won_games;
  int lost_games;
} user_data;

typedef struct {
  int size;
  user_data udata[1024];
} user_list;

typedef struct {
  char action[4];
  char name[25];
  char passwd[25];
} user_login;

user_list new_userlist_from_file();
int save_to_file(user_list* ac_users);
int user_exists(char* username, user_list* ac_users);
int check_user_pass(char* username, char* passwd, user_list* ac_users);
int add_new_user(char* username, char* passwd, user_list* ac_users);
#endif