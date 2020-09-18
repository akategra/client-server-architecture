# Client-Server Architecture:

**" In order to execute the exe files on Windows OS, you need to change the linux APIs to Windows."**

There are 3 directories inside the repository consisting of code for the server, client and the clienthandler.

The 3 executables are present in the repository.

Open the Terminal and execute the **server exe** file.

The **process exe** will be required by the server to execute and create client handlers upon the connection of a client.

Open another Terminal and execute the **client exe** file. 

# Input format for the Client:

The client can use 8 commands namely add, sub, mul, div, run, kill, list and exit.

1. **add, sub, mul, div** : To apply arithmetic operation on numbers, type the operator <space> and then space separated integer values. If only written the operator followed by no numbers, output will be an error otherwise the result will be displayed.

2. **run**: to execute a program, write "run" <space> and then the name of the exe file. If file is not present in the "/usr/bin" directory or the exe name is incorrect or the name field is left empty, output will be an error otherwise the program will be successfully executed.

3. **kill**: to kill a program, write "kill" <space> and then the program to be killed. Output will be an error if the provided program name field is not executed by the server or the program was already killed. Otherwise the program will be terminated.

4. **list**: to view process list of your client, write "list". On successful execution, the process list containing killed and active processes will be displayed. 

5. **exit**: to quit, write "exit". The client will be terminated and the programs that the client executed but were not killed before exiting will alse be terminated.

**Any other command other than these will be considered Invalid!!**

The client supports multi-command input. Use **semi-colon** to execute multiple commands. When using multiple commands, results may not be in order as threading has been implemented.

# Input format for the Server: 

There are 2 commands namely print and list.

1. **print**: If you want to send a message to all your connected clients, write "print" and press enter. After pressing enter, you will be required to type your message that will be delivered to all the clients. If message field is left empty, Output will be an error otherise the message will be successfully delivered.

2. **list**: If you want to view the process list of all active clients, write "list". On success, the process lists of all client will be displayed on the server window.

In both of the commands above, output will be an error if no clients are connected!

Any other command other than these will be considered Invalid!!

**Commands are case sensitive!!!**




