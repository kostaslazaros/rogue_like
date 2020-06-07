#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char name[25];
  char passwd[25];
  int won_games;
  int lost_games;
} user_data;

user_data user_data_input() {
  user_data new_user;
  //   system("clear");
  printf("\n Enter the user name : ");
  scanf("%s", new_user.name);
  printf("\n Enter the password : ");
  scanf("%s", new_user.passwd);
  new_user.won_games = 0;
  new_user.lost_games = 0;
  return new_user;
}

int user_exists(user_data* user, user_data users[], int user_number) {
  for (int i = 0; i <= user_number; i++) {
    // printf("%s %s\n", user->name, users[i].name);
    if (strncmp(user->name, users[i].name, 20) == 0)
      return 1;
  }
  return 0;
}

void add_user_input(user_data users[], int* user_number) {
  user_data new_user = user_data_input();
  if (!user_exists(&new_user, users, *user_number)) {
    users[*user_number] = new_user;
    (*user_number)++;
  } else {
    printf("user already exists!!! \n");
  }
}

void add_user(user_data* user, user_data users[], int* user_number) {
  if (!user_exists(user, users, *user_number)) {
    users[*user_number] = *user;
    (*user_number)++;
  }
}

int read_from_file(user_data users[], int* user_number) {
  FILE* inf;
  user_data inp;
  inf = fopen("users.dat", "r");
  if (inf == NULL) {
    fprintf(stderr, "\nError to open the file\n");
    return 1;
  }
  while (fread(&inp, sizeof(user_data), 1, inf))
    add_user(&inp, users, user_number);
  fclose(inf);
  return 0;
}

int save_to_file(user_data users[], int user_number) {
  FILE* outfile;
  // open file for writing
  outfile = fopen("users.dat", "w");
  if (outfile == NULL) {
    fprintf(stderr, "\nError opend file\n");
    exit(1);
  }

  for (int i = 0; i < user_number; i++) {
    fwrite(&users[i], sizeof(user_data), 1, outfile);
  }

  if (fwrite != 0)
    printf("contents to file written successfully !\n");
  else
    printf("error writing file !\n");

  // close file
  fclose(outfile);

  return 0;
}

void main() {
  int usr_num = 0;
  user_data users[256];
  read_from_file(users, &usr_num);
  add_user_input(users, &usr_num);
  add_user_input(users, &usr_num);
  save_to_file(users, usr_num);
  for (int j = 0; j < usr_num; j++) {
    printf("%s \t %s \t %d \t %d \n", users[j].name, users[j].passwd,
           users[j].won_games, users[j].lost_games);
  }
  printf("User number %d \n", usr_num);
}
