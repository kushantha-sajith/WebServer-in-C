#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024

//Returns the Content-type
char* get_file_type(char uri[]) {
  char *type;
  
  if(strstr(uri, ".jpg") != NULL || strstr(uri, ".jpeg") != NULL){
    type = "Content-Type: Content-Type: image/jpeg\r\n\r\n";
    return type;
  }
  if(strstr(uri, ".html") != NULL){
    type = "Content-Type: text/html\r\n\r\n";
    return type;
  }
  if(strstr(uri, ".php") != NULL){
    type = "Content-Type: application/x-httpd-php\r\n\r\n";
    return type;
  }
  if(strstr(uri, ".css") != NULL){
    type = "Content-Type: text/css\r\n\r\n";
    return type;
  }
  if(strstr(uri, ".gif") != NULL){
    type = "Content-Type: Content-Type: image/gif\r\n\r\n";
    return type;
  }
  if(strcmp(uri, "/") == 0){
    type = "Content-Type: text/html\r\n\r\n";
    return type;
  }
  return type = "Content-Type: text/html\r\n\r\n";
}

//Read the requested file
void get_requested_file(char uri[],char *buffer,int new_socket,char *type)
{
  int bytes;
  FILE *fp;
  char uri1[BUFFER_SIZE];
  
  //Remove '/' from URI
  memmove (uri, uri+1, strlen(uri+1) + 1);
  char filepath[] = "./htdocs/";
  strcat(filepath, uri);
  printf("%s\n",filepath );
  
  //Open the requested file
  fp = fopen(filepath, "r");
  if(strcmp(filepath,"./htdocs/") == 0){
    fp = fopen("./htdocs/index.html", "r");
  }
  if(fp == NULL){
    fp = fopen("./htdocs/404.html","r");
  }
  sprintf(buffer, "HTTP/1.1 200 OK \r\n");
  strcat(buffer, type);
  //Send the HTTP response headers
  send(new_socket, buffer, strlen(buffer), 0);
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
    	//Send the requested file content
        send(new_socket, buffer, bytes, 0);
    }
    fclose(fp);
}

int main(){
  char *buffer = malloc(BUFFER_SIZE);
  //Create a new TCP Socket
  int create_socket = socket(AF_INET,SOCK_STREAM,0);
  int option =1;
  setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  
  if(create_socket == -1){
    perror("webserver (socket)");
    return 1;
  }
  printf("Socket Created Successfully\n");

  struct sockaddr_in host_address;
  int host_addrlen = sizeof(host_address);

  host_address.sin_family = AF_INET;
  host_address.sin_port = htons(PORT);
  host_address.sin_addr.s_addr = htonl(INADDR_ANY);

  struct sockaddr_in client_address;
  int client_addrlen = sizeof(client_address);

  //Bind the socket
  if(bind(create_socket, (struct sockaddr *)&host_address, host_addrlen)!= 0){
    perror("webserver (bind)");
    return 1;
  }
  printf("Binding Socket\n");

  //Listen for incoming connections
  if(listen(create_socket, SOMAXCONN) != 0){
    perror("webserver (listen)");
    return 1;
  }

  printf("The Client is connected...\n");

  while (1){
    int new_socket = accept(create_socket, (struct sockaddr *)&host_address, (socklen_t *)&host_addrlen);
    if(new_socket < 0){
      perror("webserver (accept)");
      continue;
    }
    printf("Connection Accepted\n");

    //Get socket information
    int sockn = getsockname(new_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_addrlen);
    if(sockn < 0){
      perror("webserver (getsockname)");
      continue;
    }

    //Read incoming requests
    int valread = read(new_socket, buffer, BUFFER_SIZE);
    if(valread < 0){
      perror("webserver (read)");
      continue;
    }

    char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
    sscanf(buffer, "%s %s %s", method, uri, version);
    printf("[%s:%u] %s %s %s\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port),method,version,uri);
    
    char *type= get_file_type(uri);

    get_requested_file(uri,buffer,new_socket,type);
    close(new_socket);
  }
  return 0;
}
