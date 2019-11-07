/*
UNIX_SHELL

Andrew Cisneros
Operating Systems course
 */

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h> // provides access to POSIX api
#include <sys/wait.h>
#include <stdlib.h>

using namespace std;


//Global Program Variables
const int MAX_LINE = 80; 
const int MAX_PARAMS = 8;
const int H_MAX =10;  //max num of cmds in historybuffer
bool should_run = 1; //flag to determine when to exit program
char* historyBuff[MAX_LINE/2 +1]; // arry to hold history
int historyCount = 0;
int elemI = -1; //keeps track of current history index in the array buffer

//function Prototypes             (Purpose and parameters dicussed in defintion)
int getArgNum(char **params);
void displayHistory();
void parse(char* cmd, char **params);
void execShellCmd(char* args[]);
void execUserCmd(char* args[]);
bool isCommand(string cmd);
void addHistory(char* cmdtoAdd);
void popCmd(char cmd[]);

int main() 
{
  cout << "------------------------------------" << endl
       << "UNIX Shell" << endl
       << "Andrew Cisneros" << endl 
       << "------------------------------------" << endl << endl;


  //loop begins
  while(should_run)
    {
      char* args[MAX_LINE/2 +1]; //array for cmd words
      char cmd[1024];

      cout << "\nosh> ";     //"orchestrate shell"
      cin.getline(cmd, 1024);       //take in user entered command and add to histiry buffer

      addHistory(cmd);
      
      parse(cmd, args);  //parse the cmd
      
      string command(args[0]);   //stores the first word of the line entered by the user (the command) in a  string
      
      //execute a user command (see def) or a shell command (see def)
      if(isCommand(command) == true)
	execUserCmd(args);
      else 
	execShellCmd(args);
    }
  //loop ends

  cout << endl << "exiting ... " << endl; //exiting statement

  return 0;

}

//returns if command is a user command or needs system call
//returns true for user cmd
bool isCommand(string cmd)
{
  if(cmd == "exit" || cmd == "!!" || cmd == "history")
    return true;
  else
    return false;
}

//PURPOSE: Carry out user commands such as "exit", "history", and "!!"
//PARAMETERS: charter array of ptrs to character string of arguments
void execUserCmd(char* args[])
{
  string command(args[0]); //get the command of the args str

  if(command == "exit")    //exit
    should_run = false;

  else if (command == "history")  //entering "history" returns last 10 commands entered by the user
    displayHistory();            //no previous commands returns appropriate msg   
  
  else if(command == "!!")  //gets the last cmd and executes it
    {
      if(historyCount == 1)
	{
	  cout << "No previous commands" <<  endl;
	}      //error checking, if no previoud commands are avo
      else      //get last command, add it to history and execute it 
	{
	  char lastCmd[1024];
	  popCmd(lastCmd);
	  
	  parse(lastCmd, args);
	  
	  string command(args[0]);
	  cout << "Calling last command ... " << endl
	       << "\nosh> " ;
	  int q = 0;
	  while(args[q] != NULL)
	    {
	      cout << args[q] << " ";
		  q++;
	    }
	  cout << endl;
	  addHistory(lastCmd);

	  if(isCommand(command) ==1)
	    execUserCmd(args);	  
	  else
	    {
	      execShellCmd(args);
	    }	      
	}
    }   
}


//PURPOSE: carrys our systems calls such as fork and execpv
//PARAMETERS:  charter array of ptrs to character string of arguments 
void execShellCmd(char* args[])
{
  //fork a child process
  pid_t pid;
  pid = fork();  
  
  if(pid == 0)
    {
      execvp(args[0], args);// step 2
      //iff process successfull it will not return here
      //if the process fails it will return and output this msg
      cout << "execvp: command not found" << endl;
      exit(1); 
    }  
  // parent process
  else if (pid > 0) {
    if (*args[getArgNum(args)-1] != '&') {
      wait(NULL);// step 3
    }
  }
  // fork failed
  else {
    cout << "fork: failed to fork process" << endl;
    exit(1);
  }
}


//PURPOSE: displays history buffer
//PARAMETERS: none
void displayHistory()
{
  cout << "Displaying History: " << endl;
  
  if (historyCount == H_MAX)    //when the buffer is full
    {
      for(int i = elemI + 1; i < H_MAX; i++)     //start looping where the front of the buffer is
	cout << historyBuff[i] << endl;
      
      for (int q = 0; q <= elemI; q++)       //finish showing the other part of the array
	cout << historyBuff[q] << endl;
    }
  
  else if(historyCount < 10)   //whe theres less than 10 cmd's in the buffer -- show all cmds
    {
      for(int i = 0; i < historyCount; i++)
	cout << historyBuff[i] << endl;
    }
}

//PURPOSE: add command to history buffer
//The history buffer is an array. If the array if full the oldest index gets overwritten
//PARAMETERS: array of chars
void addHistory(char* cmdtoAdd)
{
  if(historyCount < H_MAX)  //buffer is not full yet
    {
      if(elemI == -1)               //buffer empty
	{
	  elemI = 0;
     	  historyBuff[elemI] = strdup(cmdtoAdd);
	  historyCount++;
	}
      else               //buffer not empty
	{
	  elemI++;
	  historyBuff[elemI] = strdup(cmdtoAdd);
	  historyCount++;
	}
    }
  else
    {//history buff is full
       
      if(elemI == 9)      //we're reached the end the array
	{
	  elemI = 0;
	  historyBuff[elemI] =  strdup(cmdtoAdd);
	}
      else                //have not reached the end yet
	{
	  elemI++;
	  historyBuff[elemI] =  strdup(cmdtoAdd);
	}
    }
}

//PURPOSE: get the last cmd in the buffer
//Parameters: an array of chars to get the last cmd, arrays are always pass by refrence
void popCmd(char cmd[])
{
  if(historyCount == 10 && elemI == 0)  //if array is full and we've begun added cmds to the front(elem ==0),
    //then take from the back
    strcpy(cmd, historyBuff[H_MAX-1]);

  else
    {
      strcpy(cmd, historyBuff[elemI-1]);    //minus one because the command "!!" should have been added to history already
    }
}



//PARAMETER: char array command entered by the user, char pointer array to store arguments
//PURPOSE: make a char pointer point to an argument
void parse(char *cmd, char **params)
{
  
  for(int i =0; i < MAX_PARAMS; i++)
    {
      params[i] = strsep(&cmd, " ");
      
      if(params[i] == NULL)
	  break;
    }  
}

//PURPOSE: gets how many arguments there are in the parsed string entered by the user
//PARAMETRS: array of arguments (args[])     
int getArgNum(char **params)      //double pointer used to manipulate pointers
{
  int count = 0;

  while(params[count] != NULL && params[count] != '\0')
    count++;

  return count;
}



