/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#define BUFFERLENGTH 256

/* displays error messages from system calls */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    socklen_t clilen;
    int sockfd, newsockfd, portno;
    char buffer[BUFFERLENGTH];
    struct sockaddr_in6 serv_addr, cli_addr;
    int n;
    FILE *fp;
    int line_no = 0;

    //printf("argc: %i\narg0: %s\narg1: %s\narg2: %s", argc, argv[0], argv[1], argv[2]);
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

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

    /* open the logfile */
    fp = fopen(argv[2], "a");

    /* now wait in an endless loop for connections and process them */
    while (1)
    {

        /* waiting for connections */
        newsockfd = accept(sockfd,
                           (struct sockaddr *)&cli_addr,
                           &clilen);
        if (newsockfd < 0)
        {
            /* close the file*/
            fclose(fp);

            /* close the connection */
            close(newsockfd);

            error("ERROR accepting connection");
        }

        bzero(buffer, BUFFERLENGTH);

        /* read the data */
        n = read(newsockfd, buffer, BUFFERLENGTH - 1);
        if (n < 0)
        {
            /* close the file*/
            fclose(fp);

            /* close the connection */
            close(newsockfd);

            error("ERROR reading from socket");
        }

        if (fp == NULL)
        {
            fprintf(stderr, "Cannot Open File: %s", argv[2]); /* write to the log file if valid */
        }
        else
        {
            fprintf(fp, "%d %s\n", ++line_no, buffer);
        }

        /* close the file*/
        fclose(fp);

        /* close the connection */
        close(newsockfd);
    }
    return 0;
}
