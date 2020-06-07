#include "users.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// typedef struct {
//   char name[25];
//   char passwd[25];
//   int won_games;
//   int lost_games;
// } user_data;

// typedef struct {
//   int size;
//   user_data udata[1024];
// } user_list;

// typedef struct {
//   char action[4];
//   char name[25];
//   char passwd[25];
// } user_login;

// int read_from_file(user_list* ac_users) {
//   FILE* finfo;
//   user_data user_from_file;
//   finfo = fopen("users.dat", "r");
//   if (finfo == NULL) {
//     fprintf(stderr, "\nError to open the file\n");
//     return 1;
//   }
//   int i = 0;
//   while (fread(&user_from_file, sizeof(user_data), 1, finfo)) {
//     ac_users->udata[i] = user_from_file;
//     ac_users->size = i++;
//   }
//   return 0;
// }

int save_to_file(user_list* ac_users) {
  FILE* outfile;
  outfile = fopen("users.dat", "w");
  if (outfile == NULL) {
    fprintf(stderr, "\nError opend file\n");
    exit(1);
  }
  for (int i = 0; i < ac_users->size; i++) {
    fwrite(&ac_users->udata[i], sizeof(user_data), 1, outfile);
  }
  // if (fwrite != 0)
  //   printf("contents to file written successfully !\n");
  // else
  //   printf("error writing file !\n");

  // close file
  fclose(outfile);

  return 0;
}

user_list new_userlist_from_file() {
  FILE* finfo;
  user_data user_from_file;
  user_list ac_users;
  finfo = fopen("users.dat", "r");
  if (finfo == NULL) {
    // fprintf(stderr,
    //         "\nFile users.dat does not exist. Lets create a new one !!!\n");
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
