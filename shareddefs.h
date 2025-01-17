#define MAX_FILE_NAME_LENGTH 512
#define MAX_DIRECTORY_SIZE 8192

enum req_type{
   LIST,
   DOWNLOAD,
   QUIT
};


struct request{
   enum req_type type;
   char filename[MAX_FILE_NAME_LENGTH];
   char localfilename[MAX_FILE_NAME_LENGTH];
};


struct response{
   enum req_type type;
   char data[MAX_DIRECTORY_SIZE];
};


struct mq_connection{
   char cs_name[8];
   char sc_name[8];
};
