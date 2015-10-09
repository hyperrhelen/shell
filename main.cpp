// Helen Chac
// Writing a basic shell that takes in command line commands.
// Some commands that it can take in is:
//  - pwd
//      Prints the current directory.
//  - cd [directory]
//      Changes into the directory.
//  - ls [directory]
//      Lists all the files and attributes in the directory.
//  Also accounts for piping and redirction. 
// Example:
// cat file.txt > file2.txt
// If we diff both file.txt and file2.txt, we will find that they are both
// identical.

#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcnt1.h>
#include <iostream>
#include <list>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

using namespace std;

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
          // want to print an error with write
          write(2, "Not a terminal\n", 15);
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

// Returns the working directory.
string getPath(char* dirName){
  string pathName = "";
  string dir = (string)dirName;
  int length = strlen(dirName);
  if(length < 16){
    pathName = dir + "> \0";
  }
  else{
    for(int j = length - 1; j > 0; j--){
      if(dirName[j] == '/'){
        pathName = "/..." + dir.substr(j, length - j) + "> \0";
        break;
      }
    }
  }
  return pathName;
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

// Parses out the command the user enters into tokens separated by spaces,
// pipes or redirecting arrows.
vector<string> parse(string input){
  vector<string> parsedString;
  int prev = 0;
  int sizeInput = input.size();
  for(int i = 0; i < sizeInput + 1; i++){
    if(i == sizeInput - 1){
      string temp = input.substr(prev, sizeInput-prev);
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

// Lists all the files that are in the specified directory.
void lsDirectory(char* newDir, char** args){
  char* dir;
  int flag = false;
  if(args[0] == NULL){
    return;
  }
  if(args[1] == NULL){
    dir = newDir;
  }
  else if(args[2] == NULL){
    flag = true;
    //this is going to be the directory name
    dir = new char(strlen(newDir) + strlen(args[1])+2);
    strcpy(dir, newDir);
    strcat(dir, "/");
    strcat(dir, args[1]);
    strcat(dir, "\0");
  }
  else{
    return;
  }

  // Checks to see if we're in the correct directory.
  DIR *lsDir;
  struct dirent *pointFile;
  if((lsDir = opendir(dir)) == NULL){
    write(STDOUT_FILENO, "Cannot open the directory\n", 12);
    exit(0);
  }
  // Checks the permissions of all the files in the directory and correctly 
  // outputs them.
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
    write(STDOUT_FILENO, pointFile->d_name, strlen(pointFile->d_name));
    write(STDOUT_FILENO, "\n", 1);
  } 
  if(flag == true){
    delete[] dir;
  }
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
    // By default, if there isn't a path that the user specifies, then
    // redirects to home.
    char* home;
    home = getenv("HOME");
    if(home == NULL){
      write(1, "Error\n",6);
    }
    chdir(home);

  }
  else if (args[2] == NULL){
    char* newDir = new char[strlen(dir)+ strlen(args[1]) + 2];
    if(newDir != NULL){
      strcpy(newDir, dir);
      strcat(newDir, "/");
      strcat(newDir, args[1]);
    }
    chdir(newDir);
    delete[] newDir;
  }
  else{
    return;
  }
}

 
// Prints out the working directory. It will get the current directory
// that it's in. It's passed in because we calculate for it.
void printWorkingDirectory(){
  char *dir = get_current_dir_name();
  write(STDOUT_FILENO, dir, strlen(dir));
  write(STDOUT_FILENO, "\n", 1);
}

void terminate(){
  exit(0);
}

// Returns the last 10 commands that were entered in the command line.
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
    delete[] memHist;
  }
} //print the last 10 inputted

void backspace(){
  write(STDIN_FILENO, "\b \b", 3);
}

// Redirects the commands to the associated function to do the command.
void doCommands(char** args, char* dir, vector<string> history){
  if(strcmp(args[0], "history") == 0){
    if(args[1] == NULL){
      getHistory(history);
    }
    else{
      write(1, "ERROR\n", 6);
    }
  } 
  else if(strcmp(args[0], "exit") == 0){
    if(args[1] == NULL){
      exit(0);
    }
  }
  else if (strcmp(args[0], "pwd") == 0){
    if(args[1] == NULL){
      printWorkingDirectory();
    }
    else{
      // Print out an error.
    }
  }
  else if (strcmp(args[0], "ls") == 0){
    lsDirectory(dir, args);
  }
  else if(strcmp(args[0], "cd") == 0){
    changeDirectory(dir, args);
  }
  else{
    execvp(args[0], args);
  }
  exit(0);
}

// Main chunk of the program which it begins execution upon.
int main(){
  struct termios SavedTermAttributes;
  vector<string> history;
  char RXChar;
  SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes); 
  int histCount = 0;
  bool readHist = false;
  string readIn = "";
  vector<int> pids;

  while(1){
    char *dir = get_current_dir_name(); 
    string pathString = getPath(dir);
    char* pathName = new char[(int) pathString.size() + 1];
    strcpy(pathName, pathString.c_str());
    
    write(STDOUT_FILENO, pathName, strlen(pathName));
    delete[] pathName;

    bool prevClickedUp = false;
    while(1){
      read(STDIN_FILENO, &RXChar, 1);
      if(0x04 == RXChar){
        exit(0);
      } // if ctrl+C or ctrl+D then you end the program.
      else{
        if(0x1B == RXChar){
          read(STDIN_FILENO, &RXChar, 1);
          if(0x5B == RXChar){
            read(STDIN_FILENO, &RXChar, 1);
            if(0x41 == RXChar){
              if(prevClickedUp == false && (int) history.size() == 0){
                // Then we don't do anything.
              }
              else{
                if(prevClickedUp == false ){
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
                delete[] printOut; 
                //clears out the readIn string completely
              }
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
                    delete[] printOut; 
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
            }
            else{
              write(STDOUT_FILENO, "\a", 1);
            }
          }//prints backspace
        }//else accounts for the esc button and the eneter key
      }// if it's not a character that matters like abc123
    }// finishes readig the stream

    if(readIn.size() > 0){
      history.push_back(readIn);
    }// stores this in history
    if((int) readIn.size() == 0){
      continue;
    }

    // this is going to the parsing for us
    vector<string> listInput;
    int prev = 0;
    int sizeInput = readIn.size();
    for(int i = 0; i < sizeInput + 1; i++){
      if(i == sizeInput - 1){
        string temp = readIn.substr(prev, sizeInput-prev);
        listInput.push_back(temp);
      }
      else if(readIn[i] == ' '){
        if((readIn[i-1] == '<' || readIn[i-1] == '>' || readIn[i-1] == '|') && i > 0){
          prev = i + 1;
        }
        else{
          string temp = readIn.substr(prev, i-prev);
          listInput.push_back(temp);
          prev = i+1;  
        }
      } // if space
      else if (readIn[i] == '<' || readIn[i] == '>' || readIn[i] == '|'){
        if(readIn[i-1] != ' '){
          string temp = readIn.substr(prev, i-prev);
          listInput.push_back(temp);
          temp = readIn.substr(i, 1);
          listInput.push_back(temp);
          prev = i+1;  
        }
        else{
          string temp = readIn.substr(i, 1);
          listInput.push_back(temp);
          prev = i+1;
        }
      } // if redirect or pipe
    } // for

 
    readIn = ""; // reset the string

    // Check if the commands return true, if it returns true
    // then you want to add it into the history

    // want to check how many pipes to make
    // want to make sure that you are doing redirection 
    // before you do piping.

    
    //Piping Area
    vector<string> command;
    int pipefd[2];// 0 for in 1 for out
    pid_t pid;
    char **args;
    
    int prevInPipe = -1;
    bool start = true;
    bool end = false;
    int count = 0; //count keeps track of how many commands are in args
    int in, out;
    char** nPtr;

    for(vector<string>::iterator it = listInput.begin(); it != listInput.end(); it++){
      if(*it == "|" || it == listInput.end() - 1){
        // If it's the end, we want to make sure we account for the command.
        if(it == listInput.end() - 1){
          command.push_back(*it);
          count++;
          end = true;
        } 

        if(it != listInput.end() - 1){
          pipe(pipefd);
        } // want to pipe every time it's not the end
  
        args = new char*[(int)command.size() + 1];
        int sizeOfCommands = (int) command.size();
        for(int i = 0; i < sizeOfCommands + 1; i++){
          if(i == sizeOfCommands){
            args[i] = new char;
            args[i] = NULL;
            if(strcmp(args[0], "cd") == 0){
                changeDirectory(dir, args);
              }
            else if (strcmp(args[0], "exit") == 0){
              exit(0);
            }
            else{
              pid = fork();
              if (pid < 0){
                write(1, "Error\n", 6);
              }
              else if(pid == 0){
                nPtr = new char*[(int)command.size() + 1];
                int counting = 0;
                for(int j = 0; j < count + 1; j++){
                  if(args[j] == NULL){
                    nPtr[counting] = new char;
                    nPtr[counting] = NULL;
                    counting++;
                  }
                  else if(strcmp(args[j], "<") == 0){
                    in = open(command[j+1].c_str(), O_RDONLY);
                    dup2(in, STDIN_FILENO);
                    close(in);
                    j++;
                  }
                  else if (strcmp(args[j], ">") == 0){
                    out = open(command[j+1].c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    dup2(out, STDOUT_FILENO);
                    close(out);
                    j++;
                  }
                  else{
                    nPtr[counting] = new char[strlen(args[j]+1)];
                    nPtr[counting] = args[j];
                    counting++;
                  }

                }

                if(end == true){
                  if(prevInPipe != -1 ){
                    dup2(prevInPipe, STDIN_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);  
                  }
                } // end close

                else if(prevInPipe == -1){
                  dup2(pipefd[1], STDOUT_FILENO);
                  close(pipefd[1]);
                  close(pipefd[0]);
                } // start
                else{
                  dup2(prevInPipe, STDIN_FILENO);
                  dup2(pipefd[1], STDOUT_FILENO);
                  close(pipefd[1]);
                  close(pipefd[0]);
                } // if it's the middle one
                doCommands(nPtr, dir, history);
              }
  
              else{
                pids.push_back(pid);
              } // parent 
            
              if(start == true){
                start = false;
                close(pipefd[1]);
              } // at beginning
              else if(end == true){
                close(prevInPipe);
              } // on the right
              else{

                close(pipefd[1]);
                close(prevInPipe);
              } // in the middle
              prevInPipe = pipefd[0];

            }
          }
          else{
            args[i] = new char[(int) command[i].size() + 1];
            strcpy(args[i], command[i].c_str());

          }
        }
        // want to delete/free/deallocate the arguments over here
        for(int i = (int) command.size() - 1; i >=0; i--){
          delete[] args[i];
        }
        delete[] args;
        command.clear();
        count = 0;
      }
      else{
        command.push_back(*it);
        count++;
      }
    }
    
    for(int ch = 0; ch < (int) pids.size(); ch++){
      waitpid(pids[ch], NULL, 0);
    }

    listInput.clear();
  }//continue running the program

  return 0;
}

