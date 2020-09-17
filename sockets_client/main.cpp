#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <string>
#include <signal.h>
#include <pthread.h>


using namespace std;

int socket_descriptor =0;


void* take_input(void* arg)
{
    char buff[4096];

    while(1)
    {

      int count = read(STDIN_FILENO, buff, 4096);
      buff[count-1] = ';';
      write(socket_descriptor, buff, count);


    }

}

void* give_output(void* arg)
{
    char output[4096];

    while(1)
    {

        int count = read(socket_descriptor, output, 4096);
        write(STDOUT_FILENO, output, count);

        output[count] = NULL;
        string check = string(output);

        if(check.find("Terminating")!=string::npos)
        {
            pthread_exit(NULL);
        }

    }

}

int main(int argc, char *argv[])
{

    int sock;
    struct sockaddr_in server;
    struct hostent *hp;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if (hp == 0)
    {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0)
    {
        perror("connecting stream socket");
        exit(1);
    }

    socket_descriptor = sock;

    pthread_t input_tid;
    pthread_t output_tid;

    pthread_create(&output_tid, NULL, &give_output, NULL);
    pthread_create(&input_tid, NULL, &take_input, NULL);

    pthread_detach(input_tid);

    pthread_join(output_tid, NULL);
    exit(EXIT_SUCCESS);


}
