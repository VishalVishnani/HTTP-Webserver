//Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>


//Structure in which parameters after file parsing are stored
typedef struct file_parse{
  int port_no;
  char rootDir[10];
  char defweb[5][20];
  char content_type[10][25];
}fp;

fp conf1;


//Webpage for error 400,404,501
char webpage_error_t[]=
"%s %s\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
//put on html file
"<!DOCTYPE html>\r\n"
"<body><center><h1>%s</h1><br>\r\n";


//webpage for error 500
char webpage_500[]=
"HTTP/1.1 500 Internal Server Error: cannot allocate memory";


//File parsing function which stores parameters in conf1 structure
void file_parse_fn(void){
  char buffer[1024];
  int size=0;
  FILE* picture;
  char listen_port[8]="Listen";
  char root_dir[30]="DocumentRoot";
  char* listen_ptr;
  char* root;
  char dirIndex[30]="DirectoryIndex";
  char* defpage;
  char* ptr1;
  char cont_type[50]=".html text/html";
  char* c_type;
  char* ptr4;  

  int i=0;
  for(i=0;i<5;i++){
    memset(conf1.defweb[i],0,10);
  }

  picture=fopen("ws.conf","r");

  fseek(picture,0,SEEK_END);
  size=ftell(picture);
  fseek(picture,0,SEEK_SET);

  fread(buffer,1,size,picture);

  fclose(picture);

  listen_ptr=strstr(buffer,listen_port);
  listen_ptr=listen_ptr+strlen(listen_port)+1;

  root=strstr(buffer,root_dir);
  root=root+strlen(root_dir)+1;

  defpage=strstr(buffer,dirIndex);
  defpage=defpage+strlen(dirIndex)+1;

  c_type=strstr(buffer,cont_type);

  listen_ptr=strtok(listen_ptr,"\n");\
  conf1.port_no=atoi(listen_ptr);

  root=strtok(root,"\n");
  strcpy(conf1.rootDir,root);

  ptr1=strtok(defpage," ");
  strcpy(conf1.defweb[0],ptr1);

  ptr1=strtok(NULL," ");
  strcpy(conf1.defweb[1],ptr1);

  ptr1=strtok(NULL,"\n");
  strcpy(conf1.defweb[2],ptr1);

  ptr4=strtok(c_type," ");
  strcpy(conf1.content_type[0],ptr4);

  i=1;

  while(i!=9){
    ptr4=strtok(NULL,"\n");
    ptr4=strtok(NULL," ");
    strcpy(conf1.content_type[i],ptr4);
    i++;
  }
}



int main(int argc, char* argv[])
{

  //local variables used
  struct sockaddr_in server_addr,client_addr;
  socklen_t sin_len=sizeof(client_addr);
  int fd_server, fd_client;
  char buf[2048];
  char buf_og[2048];
  int fdimg;
  FILE* filesize;
  int size;
  int on=1;
  char* command;
  char* filename_t;
  char* protocol;
  char filename[100];
  char filename1[100];
  int file_cmp;
  char* p;
  char ext[5];
  int size1=-1;
  char protocol_header[35];
  char header_type[25];
  char header_length[15];
  char header_content[25];
  char webpage_error[200]; 
  int ret=-1;
  char header[100];
  int fnf=-1;
  char ext_temp[10];
  int ret_ext=-1;
  int prot1=-1;
  int prot0=-1;
  int ret_def=-1;
  int check_get=-1;
  int check_post=-1;
  char post_t[]="<h1>%s</h1>\r\n";
  char post[25];

  char method_buffer[5][10]={
    {"GET"},
    {"HEAD"},
    {"POST"},
    {"DELETE"},
    {"OPTIONS"}
  };

  int check_method=-1;
  char* data;


  //call the file parse function to get the parameters in conf structure
  file_parse_fn();

  //create socket
  fd_server=socket(AF_INET, SOCK_STREAM,0);
  if(fd_server<0){
    perror("socket");
    exit(1);
  }

  setsockopt(fd_server,SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family=AF_INET;
  server_addr.sin_addr.s_addr=INADDR_ANY;
  server_addr.sin_port=htons(conf1.port_no);

  //bind
  if(bind(fd_server,(struct sockaddr *) &server_addr,sizeof(server_addr))==-1){
    perror("bind");
    close(fd_server);
    exit(1);
  }

  //listen
  if (listen(fd_server,10)==-1){
    perror("listen");
    close(fd_server);
    exit(1);
  }

  while(1){

    //Intialize all variables
    ret=-1;
    ret_def=-1;
    fnf=-1;
    prot1=-1;
    prot0=-1;
    ret_ext=-1;
    check_get=-1;
    check_post=-1;
    check_method=-1;

    //Clear all arrays
    memset(header_length,0,sizeof(header_length));
    memset(ext,0,sizeof(ext));
    memset(header,0,sizeof(header));
    memset(protocol_header,0,sizeof(protocol_header));
    memset(header_type,0,sizeof(header_type));    
    memset(header_length,0,sizeof(header_length));
    memset(header_content,0,sizeof(header_content));    
    memset(ext_temp,0,sizeof(ext_temp));
    memset(webpage_error,0,sizeof(webpage_error));
    memset(buf,0,sizeof(buf));
    memset(buf_og,0,sizeof(buf));

   //accept
   fd_client= accept (fd_server,(struct sockaddr *)&client_addr,&sin_len);

    if(fd_client == -1){
      write(fd_client,webpage_error,sizeof(webpage_error)-1);
      printf("%s\n",webpage_error);
      perror("Connection failed...................\n");
      continue;
    }

    printf("Got client connection.............\n\n\n\n");
    

    if(!fork()){

      //child
      close(fd_server);
      memset(buf,0,2048);
      read(fd_client,buf,2047);
      printf("\n-----------------------------REQUEST FROM WEBPAGE-----------------------------\n");
      printf("%s\n",buf);

      //get the filename, method and protocol from the request
      strcpy(buf_og,buf);
      command=strtok(buf," ");
      filename_t=strtok(NULL," ");
      protocol=strtok(NULL,"\n");   
      
      //Check if the method is valid
      int k=0;
      for(k=0;k<5;k++){
        check_method=strcmp(command,method_buffer[k]);
        if(check_method==0){
          break;
        }

      }

      //check if the method is get or post
      if(check_method==0){
        check_get=strncmp(command,"GET",3);
        check_post=strncmp(command,"POST",4);
      }

      //append the root directory
      strcpy(filename,filename_t);
      strcpy(filename1,conf1.rootDir);
      strcat(filename1,filename);

      //copy protocol in protocol header, which will further be copied to webpage
      strcpy(protocol_header,protocol);

      //check if protocol is HTTP/1.0 or HTTP/1.1
      prot0=strncmp(protocol,"HTTP/1.0",8);
      prot1=strncmp(protocol,"HTTP/1.1",8);

     
      //when protocol is invalid,send 400 Bad Request
      if(prot0!=0 && prot1!=0){
        sprintf(webpage_error,webpage_error_t,"HTTP/1.1","400 Bad Request","400 Bad Request Reason: Invalid HTTP Version");
        write(fd_client,webpage_error,sizeof(webpage_error)-1);
        close(fd_client);
        printf("Closing.....\n");
        exit(0);
        continue;

      }

      //when method is invalid, send 400 Bad Request with correct Protocol
      if(check_method!=0){
         if(prot0==0){
           sprintf(webpage_error,webpage_error_t,"HTTP/1.0","400 Bad Request","400 Bad Request Reason: Invalid Method");
         }
         else{
           sprintf(webpage_error,webpage_error_t,"HTTP/1.1","400 Bad Request","400 Bad Request Reason: Invalid Method");
 
         }
         write(fd_client,webpage_error,sizeof(webpage_error)-1);

         check_method=-1;
         close(fd_client);
         printf("Closing.....\n");
         exit(0);
         continue;
      }


      //when method is not get or post, send 501 Not Implemented with correct protocol
      if(check_get!=0 && check_post!=0){
        if(prot0==0){
          sprintf(webpage_error,webpage_error_t,"HTTP/1.0","501 Not Implemented","Not Implemented");
        }
        else{
          sprintf(webpage_error,webpage_error_t,"HTTP/1.1","501 Not Implemented","Not Implemented");

        }
        write(fd_client,webpage_error,sizeof(webpage_error)-1);


        close(fd_client);
        printf("Closing.....\n");
        exit(0);
        continue;
      }
      
     
      char temp[10]=" 200 OK\r\n";
      int j=0;
      for(j=0;j<9;j++){
        protocol_header[j+8]=temp[j];
      }      

      //check if index.html is requested
      for(j=0;j<3;j++){
        ret_def=strcmp(filename,conf1.defweb[j]);
        if(ret_def==0){
          break;
        }

      }

      //when POST method
      if(check_post==0){
        data=strstr(buf_og,"\r\n\r\n");
        data=data+4;
        sprintf(post,post_t,data);
      }

     
      ret=strcmp(filename_t+1,"\0");
      if(ret==0){
        strcat(filename1,"index.html");
        strcat(filename,"index.html");
      }

      //index.html requested
      if(ret_def==0){
        memset(filename,0,sizeof(filename));
        memset(filename1,0,sizeof(filename1));
        strcpy(filename1,conf1.rootDir);
        strcat(filename1,"/index.html");
        strcpy(filename,"/index.html");
      }

      size1=strlen(filename);
      p=filename+size1-1;
     
      //get the extension
      while(*p!='.'){
        p--;
      }

      int i=0;
      p++;
      while(*p!='\0'){
        ext[i]=*p;
        p++;
        i++;
      }
  
      strcpy(header_type,"Content_Type: ");
      strcat(header_type,ext);
      strcat(header_type,";\r\n");

      strcpy(ext_temp,".");
      strcat(ext_temp,ext);

      //check if the extension is supported
      for(j=0;j<9;j++){
        ret_ext=strcmp(conf1.content_type[j],ext_temp);
        if(ret_ext==0){
          break;
        }

      }

      //if extension is not supported,send 400 Bad Request
      if(ret_ext!=0){
         if(prot0==0){
           sprintf(webpage_error,webpage_error_t,"HTTP/1.0","400 Bad Request","400 Bad Request Reason: Invalid URL");
         }
         else{
           sprintf(webpage_error,webpage_error_t,"HTTP/1.1","400 Bad Request","400 Bad Request Reason: Invalid URL");
         }
         write(fd_client,webpage_error,sizeof(webpage_error)-1);

         ret_ext=-1;
         close(fd_client);
         printf("Closing.....\n");
         exit(0);
         continue;
      }

      //check if the file which is requested exists
      filesize=fopen(filename1,"r");
      if(filesize==NULL){
        fnf=1;
      }


      //if the file doesn't exist, send 404 Not found
      if(fnf==1){
        if(prot0==0){
          sprintf(webpage_error,webpage_error_t,"HTTP/1.0","404 Not Found","404 Not Found Reason URL does not exist");
        }
        else{
          sprintf(webpage_error,webpage_error_t,"HTTP/1.1","404 Not Found","404 Not Found Reason URL does not exist");
        }
        write(fd_client,webpage_error,sizeof(webpage_error)-1);

        fnf=-1;
        close(fd_client);
        printf("Closing.....\n");
        exit(0);
        continue;
      }

      //calculate file size
      fseek(filesize,0,SEEK_END);
      size=ftell(filesize);
      fseek(filesize,0,SEEK_SET);
      fclose(filesize);

      sprintf(header_length,"%i",size);
      strcat(header_length,"\r\n\r\n");
      strcpy(header_content,"Content-Length: ");
      strcat(header_content,header_length);  

      //send the webpage header first
      strcpy(header,protocol_header);
      strcat(header,header_type);
      strcat(header,header_content);

      if(check_post==0){
        strcat(header,post);
      }

      write(fd_client,header,strlen(header));

      //send file
      fdimg=open(filename1,O_RDONLY);      
      
      sendfile(fd_client,fdimg,NULL,size);
      
      close(fdimg);
     

      close(fd_client);
      printf("Closing.....\n");
      exit(0);
      
    }
    //parent
    close(fd_client);

  }
  return 0;
}
