#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#define TRUE 1

using namespace std;

struct process_info
{

public:

    int server_write_fd = 0;
    int server_read_fd = 0;
    int client_pid = 0;
    bool check = false;

};


pthread_t input_tid, output_tid;
int active_processes = 0;
int max_request_queue = 10;


struct process_info clients[10];
void* give_output(void* args);


void* take_input(void* args)
{

    char buff[4096];
    bool flag = false;

    while(1)
    {

        int count = read(STDIN_FILENO, buff, 4096);
        buff[count-1] = NULL;

        string input = buff;

        if(input.compare("print") == 0)
        {
            write(STDOUT_FILENO, "\nType your message : ", 20);
            int count = read(STDIN_FILENO, buff, 4096);

            if(count == 1)
            {
                write(STDOUT_FILENO,"\nTry Again!!\n", 13);
                continue;
            }
            else
            {

                buff[count-1] = NULL;
                flag = false;

                for(int i =0; i < max_request_queue; ++i)
                {

                    if(clients[i].check)
                    {

                        write(clients[i].server_write_fd, buff, count);
                        flag = true;

                    }

                }

                if(!flag)
                {
                    write(STDOUT_FILENO, "\nNo Active clients present!!\n", 29);
                }
                else
                {
                    write(STDOUT_FILENO, "\nDone!!!\n", 9);
                    pthread_create(&output_tid, NULL, &give_output, NULL);
                    pthread_detach(output_tid);
                }

            }

        }
        else if(input.compare("list") == 0)
        {
            flag = false;

            for(int i = 0; i < max_request_queue; ++i)
            {

                if(clients[i].check)
                {

                    write(clients[i].server_write_fd, "%list%", 6);
                    flag = true;

                }
            }

            if(!flag)
            {
                write(STDOUT_FILENO, "\nNo Active clients present!!\n", 29);
            }
            else
            {
                write(STDOUT_FILENO, "\nDone!!!\n", 9);
                pthread_create(&output_tid, NULL, &give_output, NULL);
                pthread_detach(output_tid);
            }

        }
        else if(count > 1)
        {
            write(STDOUT_FILENO, "\nInvalid Instruction...Try Again!!\n", 35);
            continue;
        }

    }

}


void* give_output(void* args)
{

    char buff[8000];
    int c = 0;


    for(int i =0; i < max_request_queue; i++)
    {

        if(clients[i].check)
        {
            int count = read(clients[i].server_read_fd, buff, 10000);
            write(STDOUT_FILENO, buff, count);
            write(STDOUT_FILENO, "\n\n", 2);

        }

    }

}


void child_handler(int sig)
{

    pid_t term = wait(NULL);

    for(int i=0; i < max_request_queue; i++)
    {

        if(clients[i].check && term == clients[i].client_pid)
        {

            clients[i].check = false;
            --active_processes;
            close(clients[i].server_read_fd);
            close(clients[i].server_write_fd);
            write(STDOUT_FILENO, "\nClient disconnected...\n", 24);

            return;
        }

    }

}


void sigint_handler(int sig){

 sigset_t newSet;
    sigset_t oldSet;

    sigaddset(&newSet, sig);

    if(active_processes == 0){
    write(STDOUT_FILENO, "\nClosing server as no clients connected!!!\n", 43);
    exit(EXIT_SUCCESS);
    }
    else{
    write(STDOUT_FILENO, "\nCant terminate!! Clients still connected...\n",45);
    sigprocmask(SIG_BLOCK, &newSet, &oldSet);
    }



}


int main(void)
{
    int sock, length;
    struct sockaddr_in server;
    int msgsock;

    signal(SIGCHLD, child_handler);
    signal(SIGINT, sigint_handler);


    pthread_create(&input_tid, NULL, &take_input, NULL);
    pthread_detach(input_tid);


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = 0;
    if (bind(sock, (struct sockaddr *) &server, sizeof(server)))
    {
        perror("binding stream socket");
        exit(1);
    }

    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length))
    {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket has port #%d\n", ntohs(server.sin_port));
    write(STDOUT_FILENO, "Use commands like \"print\" and \"list\" to send message or get stats\n\n", 67);
    fflush(stdout);


    listen(sock, max_request_queue);

    do
    {
        msgsock = accept(sock, 0, 0);

        if (msgsock == -1)
            perror("accept");
        else
        {

            int fd[2];
            int server_fd[2];
            int client_fd[2];

            if(pipe2(fd, O_CLOEXEC) == -1)
            {
                perror("Pipe Failed");
                continue;
            }

            if(pipe(server_fd) || pipe(client_fd) == -1)
            {
                perror("Pipe Failed!!");
                continue;
            }

            pid_t child = fork();

            if(child == 0)
            {

                close(fd[0]);
                close(server_fd[1]);
                close(client_fd[0]);

                char client_socket[10];
                char server_read[10];
                char client_write[10];

                sprintf(client_socket,"%d", msgsock);
                sprintf(server_read, "%d", server_fd[0]);
                sprintf(client_write, "%d", client_fd[1]);


                char *args[] = {"./process", client_socket, server_read, client_write, NULL};

                execv(args[0], args);
                perror("Client creation failed");
                write(fd[1], "test", 4);
                exit(1);
            }
            else if(child > 0)
            {

                close(fd[1]);
                close(server_fd[0]);
                close(client_fd[1]);
                close(msgsock);

                char test[10];

                int count = read(fd[0], test, 10);

                if(count == 0)
                {

                    for(int i =0; i < max_request_queue; i++)
                    {

                        if(!clients[i].check)
                        {
                            clients[i].check = true;
                            ++active_processes;
                            clients[i].server_write_fd = server_fd[1];
                            clients[i].server_read_fd = client_fd[0];
                            clients[i].client_pid = child;
                            close(fd[0]);
                            write(STDOUT_FILENO, "\nNew Client connected...\n", 25);
                            break;
                        }

                    }

                }
                else
                {
                    close(fd[0]);
                    close(server_fd[0]);
                    close(server_fd[1]);
                    close(client_fd[0]);
                    close(client_fd[1]);
                }

            }
            else
            {
                close(msgsock);
                close(server_fd[0]);
                close(server_fd[1]);
                close(client_fd[0]);
                close(client_fd[1]);
                write(STDOUT_FILENO, "closing sock", 12);
                continue;
            }

        }

    }
    while(true);
}
