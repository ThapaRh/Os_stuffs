#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                               // so we need to define what delimits our tokens.
                               // In this case  white space
                               // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line length_l
#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define TOTAL_BLK 4226
#define LEN_BLK 8192
#define F_NUMBER  128
#define SIZE_F_MAX 10240000

FILE * file;

uint8_t blk_value[TOTAL_BLK][LEN_BLK];

struct Folder_Struct{
  uint8_t available;
  char file_n[32];
  uint32_t inn;
};

struct Inn{
  uint8_t available;
  uint8_t diff_values;
  uint32_t length_l;
  uint32_t blk_value[1250];
};

struct Folder_Struct *folder;
struct Inn *node_i;
uint8_t *block_free_variable;
uint8_t *inode_free_variable;

// void I_node_start(){
//   int cur;
//   for (cur = 0; cur < F_NUMBER; cur++){
//     node_i[cur].available    = 0;
//     node_i[cur].length_l     = 0;
//     node_i[cur].diff_values = 0;
//     int j;
//     for ( j = 0; j < 1250; j++){
//       node_i[cur].blk_value[j] = -1;
//     }

//   }
// }

// void initializeBlockList(){
//   int cur;
//   for (cur = 0; cur < TOTAL_BLK; cur++){
//     block_free_variable[cur] = 1;
//   }
// }

// void initializeInodeList(){
//   int cur;
//   for (cur = 0; cur <TOTAL_BLK; cur++){
//     inode_free_variable[cur] = 1;
//   }
// }

// void initializeDirectory(){
//   int cur;
//   for (cur = 0; cur < F_NUMBER; cur++){
//     folder[cur].available = 0;
//     folder[cur].inn = -1;

//     memset( folder[cur].file_n, 0, 32);
//   }
// }

int free_space(){
  int cur;
  int available_spc = 0;
  for (cur = 0; cur <TOTAL_BLK; cur++){
    if (block_free_variable[cur])
      available_spc+= LEN_BLK;
  }
  return available_spc;
}

int folder_entry_finder(char *file_n){
  int cur;
  int rtn = -1;
  for (cur = 0; cur <F_NUMBER; cur++){
    if ( strcmp(file_n, folder[cur].file_n) == 0)
      return cur;
  }

  for (cur = 0; cur < F_NUMBER; cur++){
    if (folder[cur].available == 0){
      folder[cur].available = 1;
      return cur;
    }
  }
  return rtn;
}

int Available_inode_finder(){
  int h_1;
  int rtn = -1;
  for (h_1 = 0; h_1 < F_NUMBER; h_1++){
    if (!node_i[h_1].available){
      node_i[h_1].available = 1;
      return h_1;
    }
  }
  return rtn;

}

int Available_block_finder(){
  int cur;
  int rtn = -1;
  for (cur = 10; cur < TOTAL_BLK; cur++){
    if (block_free_variable[cur]){
      block_free_variable[cur] = 0;
      return cur;
    }
  }
  return rtn;
}


int place( char * file_n ){

  struct stat ooz;
  int rtn;
  rtn = stat(file_n, &ooz);
  int length_l = ooz.st_size;
  if (rtn == -1){
    printf("there is no file with that name\n");
    return -1;
  }


  if ( length_l > SIZE_F_MAX )
  {
    printf("The length of the file is too big\n");
    return -1;
  }

  if ( length_l > free_space() )
  {
    printf("the file is greater than the remaining disk space\n");
    return -1;
  }
  int folder_indx = folder_entry_finder(file_n);
  int index_of_inod = Available_inode_finder();
  int bloc_f = Available_block_finder();


  memcpy(folder[folder_indx].file_n, file_n, strlen(file_n));

  int counterbalance = 0;

  file = fopen(file_n, "r");
  printf("Reading %d bytes from %s\n", length_l, file_n );
  while (length_l > 0)
  {
    fseek(file, counterbalance,  SEEK_SET );

    int bytes = fread(blk_value[bloc_f], LEN_BLK, 1, file);

    if ( bytes == 0 && !feof(file) )
    {
      printf("\nWe encountered an errror\n");
      return -1;
    }
    clearerr(file);
    length_l -= LEN_BLK;
    counterbalance += LEN_BLK;
    bloc_f++ ;
  }

  fclose(file);
  return 0;
}



int finder (char *file_n, char *newfilename)
{
 struct stat ooz;
 int level;
 level = stat(file_n, &ooz );
 int length_l = ooz.st_size;

 int folder_indx = folder_entry_finder(newfilename);
 int index_of_inod = Available_inode_finder();
 int bloc_f = Available_block_finder();
 int block_index = 0;
 if (level == -1) {
  printf("File %s does not exist\n", file_n);
  return -1;
 }

 if (file == NULL){
   printf("Could not open file: %s\n", newfilename);
   return -1;
 }

 int cur;
 for (cur = 0; cur < F_NUMBER; cur++){
   if ( strcmp(file_n, folder[cur].file_n) == 0){
     int counterbalance = 0;

     file = fopen(newfilename, "w");
     printf("Writing %d bytes from %s to %s\n", length_l, file_n, newfilename);
     while ( length_l > 0){
       int num_bytes;
       if (length_l < LEN_BLK){
         num_bytes = length_l;
       }
       else{
         num_bytes = LEN_BLK;
       }
       fwrite(blk_value[block_index], num_bytes, 1, file);

       length_l -= LEN_BLK;
       counterbalance += LEN_BLK;
       block_index++;

       fseek(file, counterbalance, SEEK_SET);
     }

     fclose(file);

     return 0;
   }
 }

 printf("!! cannot find the file\n");
 return -1;
}

void file_system_creator(char *file_n){
  file = fopen(file_n, "w");
  memset( &blk_value[0], 0, TOTAL_BLK *LEN_BLK);
  //initializeDirectory();
  int cur;
  int cu;
  int c;
  int curr;
  for (cur = 0; cur < F_NUMBER; cur++){
    folder[cur].available = 0;
    folder[cur].inn = -1;

    memset( folder[cur].file_n, 0, 32);
  }
  //initializeBlockList();
    for (cu = 0; cu < TOTAL_BLK; cu++)
    {
      block_free_variable[cu] = 1;
    }
  //initializeInodeList();
  for (c = 0; c <TOTAL_BLK; c++){
    inode_free_variable[c] = 1;
  }
  //I_node_start();
  for (curr = 0; curr < F_NUMBER; curr++){
    node_i[curr].available    = 0;
    node_i[curr].length_l     = 0;
    node_i[curr].diff_values = 0;
    int j;
    for ( j = 0; j < 1250; j++){
      node_i[curr].blk_value[j] = -1;
    }

  }
  fwrite( &blk_value[0], TOTAL_BLK, LEN_BLK, file);
  fclose(file);
}

int param(char *param, char *file_n){
  struct stat ooz;
  int level;
  level = stat(file_n, &ooz );

  if (level == -1){
    printf("File not found\n");
    return -1;
  }

  if (!strcmp(param, "+h")){
    node_i[level].diff_values += 10;
    return 1;
  }

  if (!strcmp(param, "-h")){
    if (node_i[level].diff_values == 10){
      node_i[level].diff_values -= 10;
    }
    return 1;
  }

  if (!strcmp(param, "+r")){
    node_i[level].diff_values += 20;
    return 1;
  }

  if (!strcmp(param, "-r")){
    if (node_i[level].diff_values == 20){
    node_i[level].diff_values -= 20;
    }
    return 1;
  }
  return 1;
}

int get_file(char * file_n)
{
  file = fopen(file_n, "r");
  if (file == NULL){
    perror("We cannot find the file");
    return -1;
  }
  else{
    fread(&blk_value[0], TOTAL_BLK, LEN_BLK, file);
  }
  return 0;
}



int close_file(char * file_n){
  if(file_n == NULL){
    return -1;
  }

  file = fopen(file_n, "w");
  if (file == NULL){
    perror("There is an error while opening file ");
    return -1;
  }
  else{
    fwrite(&blk_value[0], TOTAL_BLK, LEN_BLK, file);
    fclose(file);
    file = NULL;
    return 0;
  }

}


int file_listing()
{


  if (folder->available == 0)
  {
    printf("Couldnot find the files\n");
    return -1;
  }
  int cur;
  for ( cur = 0; cur < F_NUMBER; cur++)
  {
    if (folder[cur].available == 1)
    {
      struct stat ooz;
      int rtn;
      rtn = stat(folder[cur].file_n, &ooz);
      int length_l = ooz.st_size;
      time_t val = ooz.st_mtime;
      int index_of_inod = folder[cur].inn;
      if (length_l != 0 && (node_i[rtn].diff_values == 0 || node_i[rtn].diff_values == 20)) {
        printf(" %s %d %s \n", folder[cur].file_n, length_l, asctime(localtime(&ooz.st_mtime)));
      }

      if (length_l == 0){
        printf("%s has been removed \n", folder[cur].file_n);
      }
  }
  }
  return 0;
}




int main(int argc, char *argv[])
{
  folder = (struct Folder_Struct *) &blk_value[0];
  node_i = (struct Inn *)&blk_value[3];
  inode_free_variable = (uint8_t *)&blk_value[1];
  block_free_variable = (uint8_t *)&blk_value[2];

  char *cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int h = 0;
  int r = 0;
  char hidden[F_NUMBER+1][MAX_COMMAND_SIZE];
  char read_only[F_NUMBER+1][MAX_COMMAND_SIZE];

  // initializeDirectory();
  // initializeBlockList();
  // initializeInodeList();
  // I_node_start();
  int cur;
  int cu;
  int c;
  int curr;
  //we initialize the folder in here
  for (cur = 0; cur < F_NUMBER; cur++){
    folder[cur].available = 0;
    folder[cur].inn = -1;

    memset( folder[cur].file_n, 0, 32);
  }
  //we initialize the block 
    for (cu = 0; cu < TOTAL_BLK; cu++)
    {
      block_free_variable[cu] = 1;
    }
  //we initialize the inode for our code
  for (c = 0; c <TOTAL_BLK; c++){
    inode_free_variable[c] = 1;
  }
  //I_node_start();
  for (curr = 0; curr < F_NUMBER; curr++){
    node_i[curr].available    = 0;
    node_i[curr].length_l     = 0;
    node_i[curr].diff_values = 0;
    int j;
    for ( j = 0; j < 1250; j++){
      node_i[curr].blk_value[j] = -1;
    }

  }


 while( 1 )
 {
   // Print out the msh prompt
   printf ("msh> ");

   // Read the command from the commandline.  The
   // maximum command that will be read is MAX_COMMAND_SIZE
   // This while command will wait here until the user
   // inputs something since fgets returns NULL when there
   // is no input
   while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

   /* Parse input */
   char *token[MAX_NUM_ARGUMENTS];

   int   token_count = 0;

   // Pointer to point to the token
   // parsed by strsep
   char *arg_ptr;

   // save a copy of the command line since strsep will end up
   // moving the pointer head
   char *working_str  = strdup( cmd_str );

   // we are going to move the working_str pointer so
   // keep track of its original value so we can deallocate
   // the correct amount at the end
   char *working_root = working_str;

   // Tokenize the input stringswith whitespace used as the delimiter
   while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
             (token_count<MAX_NUM_ARGUMENTS))
   {
     token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
     if( strlen( token[token_count] ) == 0 )
     {
       token[token_count] = NULL;
     }
       token_count++;
   }
     // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    // int token_index  = 0;
    // for( token_index = 0; token_index < token_count; token_index ++ )
    // printf("token[%d] = %s\n", token_index, token[token_index] );

   if (token[0] != NULL)
   {
     if( !strcmp(token[0], "quit") || !strcmp(token[0], "exit")){
      return 0;
    }
     else if( !strcmp(token[0], "put")){
      place(token[1]);
    }

     else if( !strcmp(token[0], "get")){
      if (token[2] == NULL){
        finder(token[1], token[1]);
      }
      else{
        finder(token[1],token[2]);
      }
    }
     else if (!strcmp (token[0], "del")){
      struct stat qinn;
      int retqinn;
      retqinn = stat(token[1], &qinn);
      if (node_i[retqinn].diff_values == 20 || node_i[retqinn].diff_values == 30){
        printf("The read only file is unable to delete. \n");
      }
      else{

        int try;
        try = remove(token[1]);
           if(try == 0) {
              printf("File removed\n");
              struct stat del;
              int removed;
              removed = stat(token[1], &del);
              int size_del = del.st_size;
              size_del -= LEN_BLK ;
            }
            else{
              printf("There has been some error, we cannot delete the file \n");
            }
     }
   }
      else if (!strcmp (token[0], "list")) {
        file_listing();
      // if (folder->available == 0)
      // {
      //   printf("Couldnot find the files\n");
      //   return -1;
      // }
      // int cur;
      // for ( cur = 0; cur < F_NUMBER; cur++)
      // {
      //   if (folder[cur].available == 1)
      //   {
      //     struct stat ooz;
      //     int rtn;
      //     rtn = stat(folder[cur].file_n, &ooz);
      //     int length_l = ooz.st_size;
      //     time_t val = ooz.st_mtime;
      //     int index_of_inod = folder[cur].inn;
      //     if (length_l != 0 && (node_i[rtn].diff_values == 0 || node_i[rtn].diff_values == 20)) {
      //       printf(" %s %d %s \n", folder[cur].file_n, length_l, asctime(localtime(&ooz.st_mtime)));
      //     }

      //     if (length_l == 0){
      //       printf("%s has been removed \n", folder[cur].file_n);
      //     }
      // }
      // }
    }
     else if (!strcmp (token[0], "df")) {
      printf("The ramaining space in our disk is: %d bytes\n", free_space());
    }
     else if (!strcmp (token[0], "open")){
        file = fopen(token[1], "r");
        if (file == NULL){
            perror("File not found ");
          }
        else{
            fread(&blk_value[0], TOTAL_BLK, LEN_BLK, file);
          }
     }
     else if (!(strcmp(token[0], "close"))){
       if(token[1] == NULL){
         printf("close: File not found. \n");
       }
       close_file(token[1]);
     }
     else if (!(strcmp("attrib", token[0]))){
       param(token[1], token[2]);
     }
     else if (!(strcmp("createfs", token[0]))){
       if (token[1] == 0){
         printf("createfs: File not found. \n");
       }
       else{
         file_system_creator(token[1]);
         printf("New image disk has been created: %s\n", token[1]);
       }
     }
     else{
       printf("Unknown command \n");
     }

   }
   free( working_root );
 }
 return 0;
}
