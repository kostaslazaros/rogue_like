/******************************** 
* A rogue like multiplayer game * 
*            Made by:           *
*       Nefeli Stefanatou       *
*       Konstantinos Lazaros    *
*********************************/ 

#include "users.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// save user info to .dat files
int save_to_file(user_list* ac_users) {
  FILE* outfile;
  outfile = fopen(USERS_FILE, "w");
  if (outfile == NULL) {
    fprintf(stderr, "\nError opend file\n");
    exit(1);
  }
  for (int i = 0; i < ac_users->size; i++) {
    fwrite(&ac_users->udata[i], sizeof(user_data), 1, outfile);
  }
  fclose(outfile);

  return 0;
}

user_list new_userlist_from_file() {
  FILE* finfo;
  user_data user_from_file;
  user_list ac_users;
  finfo = fopen(USERS_FILE, "r");
  if (finfo == NULL) {
    return ac_users;
  }
  int i = 0;
  while (fread(&user_from_file, sizeof(user_data), 1, finfo)) {
    ac_users.udata[i] = user_from_file;
    i++;
    ac_users.size = i;
  }
  return ac_users;
}

int user_exists(char* username, user_list* ac_users) {
  for (int i = 0; i < ac_users->size; i++) {
    if (strncmp(username, ac_users->udata[i].name, 25) == 0)
      return 1;
  }
  return 0;
}

int check_user_pass(char* username, char* passwd, user_list* ac_users) {
  for (int i = 0; i < ac_users->size; i++) {
    if (strncmp(username, ac_users->udata[i].name, 25) == 0)
      if (strncmp(passwd, ac_users->udata[i].passwd, 25) == 0)
        return 1;
  }
  return 0;
}

int add_new_user(char* username, char* passwd, user_list* ac_users) {
  if (!user_exists(username, ac_users)) {
    ac_users->udata[ac_users->size].lost_games = 0;
    strcpy(ac_users->udata[ac_users->size].name, username);
    strcpy(ac_users->udata[ac_users->size].passwd, passwd);
    ac_users->udata[ac_users->size].won_games = 0;
    ac_users->size++;
  }
  return 1;
}
