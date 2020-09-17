#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <wait.h>
#include <iomanip>
#include <iostream>
#include <pthread.h>


using namespace std;

struct ProcessInfo{

public:

int child_pid = 0;
time_t start_time;
time_t end_time;
string child_name;
string child_state;
bool check = false;

};


struct ProcessInfo *process_info = (ProcessInfo*) malloc(20*sizeof(ProcessInfo));

int array_limit = 20;
int client_descriptor;
int server_read;
int client_write;

pthread_mutex_t resource_lock = PTHREAD_MUTEX_INITIALIZER;

int process_count = 0;


string printList();


void child_handler(int sig, siginfo_t *info, void *context)
{

    int term = wait(NULL);

    for(int i =0; i < process_count; i++)
    {
        if(process_info[i].check && term == process_info[i].child_pid)
        {
            process_info[i].child_state = "Not Active";
            process_info[i].end_time = time(NULL);
            process_info[i].check = false;
            return;
        }

    }

}

void kill_processes(){

for(int count = 0; count < process_count; ++count){

if(process_info[count].check){

kill(process_info[count].child_pid, SIGKILL);

}
}

}

void sigint_handler(int sig){

    sigset_t newSet;
    sigset_t oldSet;

    sigaddset(&newSet, sig);

    sigprocmask(SIG_BLOCK, &newSet, &oldSet);


}

void* server_requests(void* args)
{

    char buff[4096];

    while(1)
    {

        int count = read(server_read, buff, 4096);
        buff[count] = NULL;

        string input = buff;

        if(input.compare("%list%") == 0)
        {

            string l = "\nClient List : \n\n" + printList();

            char cl_list[l.length()];

            strcpy(cl_list, l.c_str());

            write(client_write, cl_list, l.length());

        }
        else
        {

            string message = "\nServer said: "+ input + "\n";
            strcpy(buff, message.c_str());
            write(client_descriptor, buff, message.length());


        }


    }
return NULL;
}

void* add_command(void* args)
{
    int result = 0;
    int temp = 0;


    char *space_pointer = (char*)args;
    char *token;

    token = strtok_r(space_pointer," ", &space_pointer);


    if(token != NULL)
    {
        sscanf(token, "%d", &temp);
        result = temp;
    }
    else
    {
        char *message = "\nAddition Error : No Numbers entered!!\n";
        write(client_descriptor, message, strlen(message));
        return NULL;

    }

    while(token != NULL)
    {
        token = strtok_r(space_pointer," ", &space_pointer);

        if(token!= NULL)
        {

            sscanf(token, "%d", &temp);
            result += temp;

        }

    }

    char output[1024];

    string r = to_string(result);
    string o = "\nAddition Output : "+r+"\n";

    strcpy(output, o.c_str());

    write(client_descriptor, output, o.length());

return NULL;
}

void* sub_command(void* args)
{

    int result = 0;
    int temp = 0;


    char *space_pointer = (char*)args;
    char *token;

    token = strtok_r(space_pointer," ", &space_pointer);


    if(token != NULL)
    {
        sscanf(token, "%d", &temp);
        result = temp;
    }
    else
    {
    char *message = "\nSubtraction Error : No Numbers entered!!\n";
        write(client_descriptor, message, strlen(message));
        return NULL;

    }

    while(token != NULL)
    {
        token = strtok_r(space_pointer," ", &space_pointer);

        if(token!= NULL)
        {

            sscanf(token, "%d", &temp);
            result -= temp;

        }

    }

    char output[1024];

    string r = to_string(result);
    string o = "\nSubtraction Output : "+r+"\n";

    strcpy(output, o.c_str());

    write(client_descriptor, output, o.length());
return NULL;
}

void* mul_command(void* args)
{

      int result = 0;
    int temp = 0;


    char *space_pointer = (char*)args;
    char *token;

    token = strtok_r(space_pointer," ", &space_pointer);


    if(token != NULL)
    {
        sscanf(token, "%d", &temp);
        result = temp;
    }
    else
    {
        char *message = "\nMultiplication Error : No Numbers entered!!\n";
        write(client_descriptor, message, strlen(message));
        return NULL;

    }

    while(token != NULL)
    {
        token = strtok_r(space_pointer," ", &space_pointer);

        if(token!= NULL)
        {

            sscanf(token, "%d", &temp);
            result *= temp;

        }

    }

    char output[1024];

    string r = to_string(result);
    string o = "\nMultiplication Output : "+r+"\n";

    strcpy(output, o.c_str());

    write(client_descriptor, output, o.length());
return NULL;
}

void* div_command(void* args)
{

 int result = 0;
    int temp = 0;


    char *space_pointer = (char*)args;
    char *token;

    token = strtok_r(space_pointer," ", &space_pointer);


    if(token != NULL)
    {
        sscanf(token, "%d", &temp);
        result = temp;
    }
    else
    {
        char *message = "\nDivision Error : No Numbers entered!!\n";
        write(client_descriptor, message, strlen(message));
        return NULL;

    }

    while(token != NULL)
    {
        token = strtok_r(space_pointer," ", &space_pointer);

        if(token!= NULL)
        {

            sscanf(token, "%d", &temp);
             if(temp !=0)
            {
                result /= temp;
            }
            else
            {
                write(client_descriptor, "\nDivision Error : Invalid Instruction, dividing by 0!!\n", 55);
                return NULL;
            }

        }

    }

    char output[1024];

    string r = to_string(result);
    string o = "\nDivision Output : "+r+"\n";

    strcpy(output, o.c_str());

    write(client_descriptor, output, o.length());

return NULL;
}

void* run_command(void* args)
{

    char *space_pointer = (char*) args;
    char *token;

    token = strtok_r(space_pointer, " ", &space_pointer);

    if(token==NULL)
    {
        write(client_descriptor,"\nRun Error : Failed to run desired process...\n", 46);
    }
    else
    {
        string c = string(token);

        int fd[2];

        if(pipe2(fd, O_CLOEXEC) == -1)
        {
            perror("Pipe Failed!!");
            return NULL;
        }

        int child1 = fork();


        if(child1 < 0)
        {
            write(client_descriptor, "\nOutput : Error doing fork, try again!!!\n",41);
            return NULL;
        }
        else if(child1 == 0)
        {

            close(fd[0]);
            string p = "/usr/bin/" + c;
            int n = p.length();

            char path[n+1];
            strcpy(path, p.c_str());

            char *path_pointer = strtok(path, " ");
            execl(path_pointer,NULL);
            write(fd[1], "test", 4);
            kill(getpid(), SIGKILL);


        }
        else if(child1 > 0)
        {

            close(fd[1]);
            char test[10];

            int count = read(fd[0], test, 10);

            if(count == 0)
            {

                close(fd[0]);

                pthread_mutex_lock(&resource_lock);

                if(process_count == array_limit){

                write(client_descriptor, "\nList limit reached!! Reallocating..\n", 38);

                process_info = (ProcessInfo*) realloc(process_info, (array_limit+20)*sizeof(ProcessInfo));
                array_limit+=20;

                }

                process_info[process_count].child_pid = child1;
                process_info[process_count].start_time = time(NULL);
                process_info[process_count].child_name = c;
                process_info[process_count].child_state = "Active";
                process_info[process_count].check = true;
                ++process_count;

                pthread_mutex_unlock(&resource_lock);
                write(client_descriptor, "\nRun Output : Run is Successful.\n", 33);

            }
            else if(count > 0)
            {
                close(fd[0]);
                write(client_descriptor, "\nRun Error : Run Failed!!\n",26);
                return NULL;
            }

        }

    }
return NULL;
}

void* kill_command(void* args)
{

    char *space_pointer = (char*)args;
    char *token;

    token = strtok_r(space_pointer, " ", &space_pointer);
    bool flag = false;

    if(token == NULL)
    {
        write(client_descriptor, "\nKill Error : Kill not successful!!\n", 36);
        return NULL;
    }
    else
    {

        string k = (string)token;

        for(int i = 0; i < process_count; i++)
        {

            if(process_info[i].check && k.compare(process_info[i].child_name) == 0)
            {

                flag = true;
                kill(process_info[i].child_pid, SIGTERM);
                break;

            }

        }

        if(!flag)
        {
            write(client_descriptor, "\nKill Error : Kill not successful!!\n", 36);
        }
        else
        {
            write(client_descriptor, "\nKill Output : Kill successful!!\n", 33);

        }

    }

return NULL;
}


void* list_command(void* args)
{

    string l = printList() + "\n";

    char print_list[l.length()];

    strcpy(print_list, l.c_str());

    write(client_descriptor, "\nClient List :\n", 15);
    write(client_descriptor, print_list, l.length());
return NULL;
}

int main(int argc, char* argv[])
{
    client_descriptor = atoi(argv[1]);
    server_read = atoi(argv[2]);
    client_write = atoi(argv[3]);

    char *command_pointer, *space_pointer, *commandStore_pointer;


    pthread_t server_request_tid;
    pthread_t command_threads[7];

    pthread_create(&server_request_tid, NULL, &server_requests, NULL);
    pthread_detach(server_request_tid);

    struct sigaction child_hndlr;
    child_hndlr.sa_sigaction = child_handler;
    child_hndlr.sa_flags = 0;
    child_hndlr.sa_flags = SA_SIGINFO | SA_NODEFER | SA_RESTART;

    sigaction(SIGCHLD, &child_hndlr, 0);
    signal(SIGINT, sigint_handler);

char *message = "You can use add, sub, mul, div, run, kill and list followed by their particular arguments..\nEnter \"exit\" to terminate...\n\n";

    write(client_descriptor, message,strlen(message));

    char input[4096];


    while(true)
    {

        int bytesRead = read(client_descriptor, input, 4096);


        if(bytesRead == 0)
        {
            kill_processes();
            free(process_info);
            exit(EXIT_SUCCESS);
        }

        input[bytesRead] = NULL;

        command_pointer = strtok_r(input, ";", &commandStore_pointer);


        if(command_pointer == NULL)
        {
            continue;
        }

        while(command_pointer != NULL)
        {

            int switch_Case = 0 ;
            bool flag = true;
            string oper = "";

            space_pointer = strtok_r(command_pointer," ", &command_pointer);

            if(space_pointer!=NULL)
            {
                oper = string(space_pointer);
            }

            if(oper.compare("add")==0)
                switch_Case = 1;
            else if(oper.compare("sub")==0)
                switch_Case = 2;
            else if(oper.compare("mul")==0)
                switch_Case = 3;
            else if(oper.compare("div")==0)
                switch_Case = 4;
            else if(oper.compare("run")==0)
                switch_Case = 5;
            else if(oper.compare("kill")==0)
                switch_Case = 6;
            else if(oper.compare("list")==0)
                switch_Case = 7;
            else if(oper.find("exit")!=string::npos)
            {
                write(client_descriptor, "\nOutput : Terminating after closing all opened programs..\n", 58);
                close(client_descriptor);
                kill_processes();
                free(process_info);
                exit(EXIT_SUCCESS);
            }
            else
            {
                flag = false;
            }


            if(flag)
            {


                if(switch_Case < 5)
                {

                    for(int count = 0; count<strlen(command_pointer); count++)
                    {
                        if((command_pointer[count] < '0' || command_pointer[count] > '9') && command_pointer[count]!=NULL && command_pointer[count]!=';')
                        {
                            command_pointer[count]=' ';
                        }
                    }

                }


                switch(switch_Case)
                {

                case 1 :
                {

                    pthread_create(&command_threads[0], NULL, &add_command, (void*)command_pointer);
                    pthread_detach(command_threads[0]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);

                }
                break;

                case 2 :
                {
                    pthread_create(&command_threads[1], NULL, &sub_command, (void*)command_pointer);
                    pthread_detach(command_threads[1]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);
                }
                break;

                case 3 :
                {
                    pthread_create(&command_threads[2], NULL, &mul_command, (void*)command_pointer);
                    pthread_detach(command_threads[2]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);

                }
                break;

                case 4 :
                {
                    pthread_create(&command_threads[3], NULL, &div_command, (void*)command_pointer);
                    pthread_detach(command_threads[3]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);

                }
                break;

                case 5 :
                {
                    pthread_create(&command_threads[4], NULL, &run_command, (void*)command_pointer);
                    pthread_detach(command_threads[4]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);

                }
                break;

                case 6:
                {
                    pthread_create(&command_threads[5], NULL, &kill_command, (void*)command_pointer);
                    pthread_detach(command_threads[5]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);
                }
                break;

                case 7:
                {

                    pthread_create(&command_threads[6], NULL, &list_command, (void*)command_pointer);
                    pthread_detach(command_threads[6]);
                    command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);

                }
                break;
                default:
                    break;

                }


            }
            else
            {
                write(client_descriptor, "\nInvalid Instruction....Try Again!!\n", 36);
                command_pointer = strtok_r(commandStore_pointer, ";", &commandStore_pointer);

            }


        }

    }
}

string printList()
{

    stringstream process_list;
    process_list << "\n" << left << setw(8) << "PID" << setw(20) << "Process Name";
    process_list << setw(15) << "Start Time" << setw(13) << "End Time";
    process_list << setw(17) << "Elapsed Time" << setw(14) << "Status\n" << endl;

    for(int i =0; i < process_count; ++i)
    {
        struct ProcessInfo process = process_info[i];
        struct tm * s_time = localtime(&process.start_time);

        process_list << left << setw(8) << process.child_pid << setw(20) << process.child_name;
        process_list << setw(15) << to_string(s_time->tm_hour)+":"+to_string(s_time->tm_min)+":"+to_string(s_time->tm_sec);

        if(!process_info[i].check)
        {

            struct tm * e_time = localtime(&process.end_time);

            process_list << setw(13) << to_string(e_time->tm_hour)+":"+to_string(e_time->tm_min)+":"+to_string(e_time->tm_sec);
            process_list << setw(17);

            int diff = difftime(process.end_time, process.start_time);

            process_list << to_string(diff) + " sec";

        }
        else
        {
            process_list << setw(13) << "NaN" << setw(17) << "NaN";
        }

        process_list << setw(14) << process.child_state << endl;

    }

    string l = process_list.str();

    return l;
}
