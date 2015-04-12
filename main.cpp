// Helen Chac && Nathan Truong
// Writing a basic shell. 


#include <iostream>
// This is a unix library and can get current directory??
#include <unistd.h>
// This library allows us to get the directories
#include <dirent.h>
#include <cstring>
#include <string>
#include <termios.h>
#include <ctype.h>
#include <vector>
//#include <list>
//#include <stdlib.h>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h> // /not sure what this is used for
#include <sys/wait.h> // not sure what this is used for yet
#include <errno.h> // not sure what this is ued for yet either
#include <fcntl.h>  //for opening the file

using namespace std;

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
 //   char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
// want to print an error with write
          write(2, "Not a terminal\n", 15);
//        fprintf (stderr, "Not a terminal.\n");
        return;
    }
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

char* path(char* dirName){
  vector<string> first;
  int length = strlen(dirName);
  string dir = (string) dirName;
  string currDir = "";

  if (length < 16){
    currDir = dir + "> \0";
  }
  else{
    for(int j = length - 1; j >= 0; j--){
      if(dir[j] == '/'){
        currDir = "/..." + dir.substr(j, length - j) + "> \0";
        break;
      }//for the first '/' that we see.
    }//starts at the end of the string (directory) and goes left
  }
  char* name = new char[currDir.length() + 1];
  strcpy(name, currDir.c_str());
  return name;
}

vector<string> parse(string input){
  vector<string> parsedString;
  int prev = 0;
  for(int i = 0; i < (int) input.size(); i++){
    if(i == (int) input.size() - 1){
      string temp = input.substr(prev, (int)input.size()-prev );
      parsedString.push_back(temp);
    }
    else if(input[i] == ' '){
      if((input[i-1] == '<' || input[i-1] == '>' || input[i-1] == '|') && i > 0){
        prev = i + 1;
      }
      else{
        string temp = input.substr(prev, i-prev);
        parsedString.push_back(temp);
        prev = i+1;  
      }
    } // if space
    else if (input[i] == '<' || input[i] == '>' || input[i] == '|'){
      if(input[i-1] != ' '){
        string temp = input.substr(prev, i-prev);
        parsedString.push_back(temp);
        temp = input.substr(i, 1);
        parsedString.push_back(temp);
        prev = i+1;  
      }
      else{
        string temp = input.substr(i, 1);
        parsedString.push_back(temp);
        prev = i+1;
      }
    } // if redirect or pipe
  }
  return parsedString; 
}

void lsDirectory(char* newDir, char** args){
  char* dir;
  if(args[1] == NULL){
    // then you're doing it right
    dir = newDir;
  }
  else if(args[2] == NULL){
    //this is going to be the directory name
    //dir = newDir;
    dir = new char(strlen(newDir) + strlen(args[1])+2);
    if(dir != NULL){
      strcpy(dir, newDir);
      strcat(dir, "/");
      strcat(dir, args[1]);
      strcat(dir, "\0");
      //cout << dir << endl;
    }
  }
  else{
    return;
    //print out errors
  }
  DIR *lsDir;
  struct dirent *pointFile;
  if((lsDir = opendir(dir)) == NULL){
    write(STDOUT_FILENO, "Cannot open the directory", 11);
    exit(0);
  }
  while((pointFile = readdir(lsDir)) != NULL){
    struct stat statFile;
    stat(pointFile->d_name, &statFile);
    mode_t mode = statFile.st_mode;
    switch(mode & S_IFMT){
      case S_IFDIR: 
        write(STDOUT_FILENO, "d", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;
    }
    switch(mode & S_IRUSR){
      case S_IRUSR:
        write(STDOUT_FILENO, "r", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IWUSR){
      case S_IWUSR:
        write(STDOUT_FILENO, "w", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IXUSR){
      case S_IXUSR:
        write(STDOUT_FILENO, "x", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IRGRP){
      case S_IRGRP:
        write(STDOUT_FILENO, "r", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IWGRP){
      case S_IWGRP:
        write(STDOUT_FILENO, "w", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IXGRP){
      case S_IXGRP:
        write(STDOUT_FILENO, "x", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IROTH){
      case S_IROTH:
        write(STDOUT_FILENO, "r", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IWOTH){
      case S_IWOTH:
        write(STDOUT_FILENO, "w", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }
    switch(mode & S_IXOTH){
      case S_IXOTH:
        write(STDOUT_FILENO, "x", 1);
        break;
      default:
        write(STDOUT_FILENO, "-", 1);
        break;   
    }

    write(STDOUT_FILENO, " ", 1);
    //prints out the files name
    write(STDOUT_FILENO, pointFile->d_name, strlen(pointFile->d_name));
    write(STDOUT_FILENO, "\n", 1);
  }//while reading the files/directories inside the directory
}

/*
  This function helps you change the diretory.
  You only want to take in two parameters at most

        cd [directory]

  Change directory brings you home if you only have one parameter,
  if you have two parameters, then you're going to change into that
  directory.

  If it has more than two parameters, you want to detect it and return
  an exception/error and tell the user that it wasn't a valid action
  */

void changeDirectory(char *dir, char **args){
  if(args[1] == NULL){
    //then by default, you want to go to home
    chdir("/home");
  }
  else if (args[2] == NULL){
    char* newDir = new char[strlen(dir)+ strlen(args[1]) + 2];
    if(newDir != NULL){
      strcpy(newDir, dir);
      strcat(newDir, "/");
      strcat(newDir, args[1]);
      //strcat(newDir, "\0");
    }
    chdir(newDir);
    free(newDir);
  }
}

/* 
  Prints out the working directory. It will get the current directory
  that it's in. It's passed in because we calculate for it.
  */
void printWorkingDirectory(){
  char *dir = get_current_dir_name();
  write(STDOUT_FILENO, dir, strlen(dir));
  write(STDOUT_FILENO, "\n", 1);
  free(dir);
}

void getHistory(vector<string> history){
  int sHistory = history.size();
  int tenLess; 
  if(sHistory > 10){
    tenLess = sHistory - 10;
  }
  else{
    tenLess = 0;
  }
  char firstNext = '0';
  for (int i = tenLess; i < sHistory; i++){
      char* memHist = new char[sHistory +1];
      strcpy(memHist, history[i].c_str());
      
      write(STDOUT_FILENO, &firstNext, 1);
      write(STDOUT_FILENO, " ", 1);
      write(STDOUT_FILENO, memHist, strlen(memHist));
      write(STDOUT_FILENO, "\n", 1);
      firstNext++;
  }
} //print the last 10 inputted

void backspace(){
  write(STDIN_FILENO, "\b \b", 3);
}

void doCommands(char** args, char* dir, vector<string> history){
  cout << "in the command function" << endl;
  cout << args[0] << endl;
  if(strcmp(args[0], "history") == 0){
    if(args[1] == NULL){
      getHistory(history);
    }
    else{
      write(STDOUT_FILENO, "error", 5);
      //cout << "hey stop" << endl;
      //return an error
    }
  } 
  /*
  else if(strcmp(args[0], "exit") == 0){
    if(args[1] == NULL){
      exit(0);
    }
  }*/
  else if (strcmp(args[0], "pwd") == 0){
   if(args[1] == NULL){
     printWorkingDirectory();
   }
   else{
    //print out an error
   }
  }
  
  else if (strcmp(args[0], "ls") == 0){
    //cout << "in ls" << endl;
    lsDirectory(dir, args);

    // if we get another directory 
  }
  /*
  else if(strcmp(args[0], "cd") == 0){
    changeDirectory(dir, args);
  }*/
  else{
    execvp(args[0], args);
  }
  //cout << "end of the commands " << endl;
  //exit(1);
}


int main(){
  struct termios SavedTermAttributes;
  vector<string> history;
  char RXChar;
  SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes); 
  int histCount = 0;
  bool readHist = false;
  string readIn = "";

  while(1){
    char *dir = get_current_dir_name(); 
    char *pathName = path(dir); 
    write(STDOUT_FILENO, pathName, strlen(pathName)); 
    bool prevClickedUp = false;
    while(1){
      read(STDIN_FILENO, &RXChar, 1);
      if(0x04 == RXChar){
        exit(0);
      }// if ctrl+C or ctrl+D then you end the program
      else{
        //cout << "RXChar: " << RXChar << endl; 
        if(0x1B == RXChar){
          read(STDIN_FILENO, &RXChar, 1);
          if(0x5B == RXChar){
            read(STDIN_FILENO, &RXChar, 1);
            if(0x41 == RXChar){
              if(prevClickedUp == false){
                histCount = (int) history.size();
                prevClickedUp = true;
              }
              for(int i = 0; i < (int) readIn.size(); i++){
               write(STDOUT_FILENO, "\b \b", 3); 
              } 
              readIn = "";
              if(readHist == true){
                if(histCount == 0){
                  // we don't have to do anything here
                }
                else{ 
                  histCount--;
                }
              }
              else{
                readHist = true;
                if(histCount != 0){
                  histCount = (int) history.size() - 1;
                }
              }
              readIn = history[histCount];
              char* printOut = new char[ (int) readIn.size() + 1];
              strcpy(printOut, history[histCount].c_str());
              write(STDOUT_FILENO, printOut, strlen(printOut));  
              //clears out the readIn string completely
            }
            else if (0x42 == RXChar){
              if(prevClickedUp == true){
                if((int) readIn.size() > 0){
                  for(int i = 0; i < (int) readIn.size(); i++){
                    write(STDOUT_FILENO, "\b \b", 3); 
                  }   
                }
                readIn = "";
                if(histCount == (int)history.size() ){
                  //clear it out because no need to go further
                }
                else{

                  histCount++;
                  if(histCount !=  (int)history.size()){
                    readIn = history[histCount];
                    char* printOut = new char[ (int) readIn.size() + 1];
                    strcpy(printOut, history[histCount].c_str());
                    write(STDOUT_FILENO, printOut, strlen(printOut));  
                  }                 
                }
                // then you want to go down in history
              }//this will only not be true for the first time
            }
            else if (0x43 == RXChar || 0x44 == RXChar){
              // you don't want to do anything if you're going right or left
              // right or left
              // can delete this later if you want
            }
          }
        }
        else if(isprint(RXChar)){
          write(STDOUT_FILENO, &RXChar, 1);
          readIn += RXChar;
        } //if the character is valid
        else{
          if(0x0A == RXChar){
            write(STDOUT_FILENO, &RXChar, 1);
            //prevClickedUp = false;
            break;
          } // this should be enter
          if(0x7F == RXChar){
            if(readIn != ""){
              readIn = readIn.substr(0, readIn.size()-1);
              write(STDOUT_FILENO, "\b \b", 3);
            } // if it's not an empty string then you want to delete ( backspace)
            else{
              write(STDOUT_FILENO, "\a", 1);
            } // creates the bell sound
          }//prints backspace
        }//else accounts for the esc button and the eneter key
      }// if it's not a character that matters like abc123
    }// finishes readig the stream

    vector<string> listInput = parse(readIn);
    if(readIn.size() > 0){
      history.push_back(readIn);
      // add an if statement
      //histCount++;
    }// stores this in history

    if((int) readIn.size() == 0){
      continue;
    }
    readIn = ""; // reset the string
    // check if the commands return true, if it returns true
    // then you want to add it into the history

    // want to check how many pipes to make
    // want to make sure that you are doing redirection 
    // before you do piping.

    //Piping Area
    //bool commFlag = false;
    //bool isPipe = false;
    vector<string> children;
    vector<char*> commands;
    string primBuffer = "";
    
    char **args;

    int pipe_fd[2]; // pipe and their location will get put here
    pid_t pid; // The Process ID (child or parent)

    for(vector<string>::iterator it = listInput.begin(); it != listInput.end(); it++){
      if(*it == "|" || it == listInput.end() - 1){
        if(it == listInput.end() - 1){
          char* temp = new char[ (int) (*it).size() + 1];
          strcpy(temp, (*it).c_str());
          commands.push_back(temp);
        }
        if(it != listInput.end() - 1){
          pipe(pipe_fd);
        }
  
        args = new char*[(int)commands.size() + 1];
        for(int i = 0; i < (int) commands.size() + 1; i++){
          if(i == (int) commands.size()){
            args[i] = new char[0];
            args[i] = NULL;

            if(strcmp(args[0], "cd") == 0){
              changeDirectory(dir, args);
            }
            else if (strcmp(args[0], "exit") == 0){
              exit(0);
            }
            // check to see if there's any redirection
            

            pid = fork();   
            //pipe(pipe_fd);
            if(pid == 0){
              cout << "child " << pid << endl;
              
              /*close(0);
              dup2(pipe_fd[0], 0);
              close(pipe_fd[0]);
              close(pipe_fd[1]);
              */
              /*
              dup2(STDIN_FILENO, pipe_fd[0]);
              close(pipe_fd[0]);
              close(pipe_fd[1]);
              */
              doCommands(args, dir, history); 
              //cout << "Helloooo" << endl;
              /*close(0);
              close(1); */
              //exit(1);
              exit(0);
            }
            else if(pid < 0){
              write(STDOUT_FILENO, "error\n" , 6);
            }//error
            else{
              //cout << "parent " << pid << endl;
              /*close(1);
              dup(pipe_fd[1]);
              close(pipe_fd[0]);
              close(pipe_fd[1]);
              */
              /*
              close(pipe_fd[0]);
              close(pipe_fd[1]);
              */
              int status;
              //wait(&status);
              /*do{
                
                cout << "wait-status: " << wait(&status) << endl;
                cout << "status: " << status << endl;
                cout << "pid: " << pid << endl;
              }while(status != 0);
              */

              //while(wait(&status) && status != pid);
                
              wait(&status);
              //close(1);
              //cout << "done waiting" << endl;
              //continue;
              continue;
            }//parent
            
          }
          else{
            //cout << "here ? " << endl;
            args[i] = commands[i];
          }
        }

        //clear out the vector
        for(int i = (int) commands.size()-1; i >= 0; i--){
          free(args[i]);
          //args[i].free();
        }
        free(args);
        //args.free();
        commands.clear();
        //perform 
        //break;
        //isPipe = true;
      }
      else{
        //cout << *it<< "X" << endl;
        char* temp = new char[ (int) (*it).size() + 1];
        strcpy(temp, (*it).c_str());
        //cout <<"temp, what command?: " <<  temp << endl;
        //cout << "commands: " << commands[0] << endl;
        // just want to push the commands back 
        // commands store all the commands
        //cout << "size of command" <<  commands.size() << endl;
        commands.push_back(temp);
        //cout << "size after " << commands.size() << endl;
        //cout << "pushback item: " << commands[(int) commands.size() - 1] << endl;

      }
      
    }
  }//continue running the program

  return 0;
}

