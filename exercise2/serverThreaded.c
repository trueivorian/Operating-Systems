/* A threaded server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFERLENGTH 256

/* displays error messages from system calls */
void error(char *msg)
{
  perror(msg);
  exit(1);
}

int line_no = 0;

pthread_mutex_t mut; /* the lock */

char *file_name;

/* the procedure called for each request */
void *processRequest(void *args)
{
  int *newsockfd = (int *)args;
  char buffer[BUFFERLENGTH];

  bzero(buffer, BUFFERLENGTH);

  char msg[1024];

  msg[0] = '\0';

  int n;

  // Receive the client message
  while ((n = read(*newsockfd, buffer, BUFFERLENGTH - 1)))
  {

    strcat(msg, buffer);

    bzero(buffer, BUFFERLENGTH);
  }

  if (n < 0)
  {
    close(*newsockfd); /* important to avoid memory leak */
    free(newsockfd);
    error("ERROR reading from socket");
  }

  char *curLine = msg;
  // Write text to file line by line
  while (curLine)
  {
    char *nextLine = strchr(curLine, '\n');
    if (nextLine)
    {
      *nextLine = '\0'; // temporarily terminate the current line

      pthread_mutex_lock(&mut); /* lock exclusive access to the log file */

      /* open the logfile in append mode*/
      FILE *fp = fopen(file_name, "a+");

      if (fp == NULL)
      {
        fprintf(stderr, "Cannot Open File: %s", file_name); /* write to the log file if valid */
      }
      else
      {
        fprintf(fp, "%d %s\n", line_no++, curLine);
      }

      /* close the file*/
      fclose(fp);

      pthread_mutex_unlock(&mut); /* release the lock */

      *nextLine = '\n'; // then restore newline-char, just to be tidy
    }
    curLine = nextLine ? (nextLine + 1) : NULL;
  }

  close(*newsockfd); /* important to avoid memory leak */
  free(newsockfd);

  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  socklen_t clilen;
  int sockfd, portno;
  char buffer[BUFFERLENGTH];
  struct sockaddr_in6 serv_addr, cli_addr;
  int result;

  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /* Get file name */
  file_name = argv[2];

  /* check port number */
  portno = atoi(argv[1]);
  if ((portno < 0) || (portno > 65535))
  {
    fprintf(stderr, "%s: Illegal port number, exiting!\n", argv[0]);
    exit(1);
  }
  /* create socket */
  sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin6_family = AF_INET6;
  serv_addr.sin6_addr = in6addr_any;
  serv_addr.sin6_port = htons(portno);

  /* bind it */
  if (bind(sockfd, (struct sockaddr *)&serv_addr,
           sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* ready to accept connections */
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);

  /* now wait in an endless loop for connections and process them */
  while (1)
  {

    pthread_t server_thread;

    int *newsockfd;              /* allocate memory for each instance to avoid race condition */
    pthread_attr_t pthread_attr; /* attributes for newly created thread */

    newsockfd = malloc(sizeof(int));
    if (!newsockfd)
    {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(1);
    }

    /* waiting for connections */
    *newsockfd = accept(sockfd,
                        (struct sockaddr *)&cli_addr,
                        &clilen);
    if (*newsockfd < 0)
      error("ERROR on accept");
    bzero(buffer, BUFFERLENGTH);

    /* create separate thread for processing */
    if (pthread_attr_init(&pthread_attr))
    {
      fprintf(stderr, "Creating initial thread attributes failed!\n");
      exit(1);
    }

    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED))
    {
      fprintf(stderr, "setting thread attributes failed!\n");
      exit(1);
    }
    result = pthread_create(&server_thread, &pthread_attr, processRequest, (void *)newsockfd);
    if (result != 0)
    {
      fprintf(stderr, "Thread creation failed!\n");
      exit(1);
    }
  }
  return 0;
}
