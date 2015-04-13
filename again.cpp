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
#include <list>
//#include <stdlib.h>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h> // /not sure what this is used for
#include <sys/wait.h> // not sure what this is used for yet
#include <errno.h> // not sure what this is ued for yet either
#include <fcntl.h>

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
  //cout <<"function" <<  name << endl;
  return name;
}

vector<string> parse(string input){

  //cout << ":( parse no work? " << endl;
  //cout << input << endl;
  vector<string> parsedString;
  int prev = 0;
  //cout << input << "X" << endl;
  //cout << input.size() << endl;
  int sizeInput = input.size();
  //cout << "input size: " << sizeInput << endl;
  for(int i = 0; i < sizeInput + 1; i++){
    //cout << "i:" << i << endl;
    if(i == sizeInput - 1){
      string temp = input.substr(prev, sizeInput-prev);
      parsedString.push_back(temp);
      //cout << temp << endl;
    }
    /*
    if(i == (int) input.size() - 1){
      cout << "parse string iff statement" << endl;
      string temp;
      if(input[i] == ' '){
        temp = input.substr(prev, (int)input.size()-prev - 1);
      }
      else{
        temp = input.substr(prev, (int)input.size() - prev);
      }
      parsedString.push_back(temp);
      cout << temp << endl;
      //cout << temp <<"4"  << endl;
    }*/
    else if(input[i] == ' '){
      if((input[i-1] == '<' || input[i-1] == '>' || input[i-1] == '|') && i > 0){
        prev = i + 1;
      }
      else{
        string temp = input.substr(prev, i-prev);
        parsedString.push_back(temp);
        //cout << temp << endl;
        prev = i+1;  
      }
    } // if space
    else if (input[i] == '<' || input[i] == '>' || input[i] == '|'){
      if(input[i-1] != ' '){
        string temp = input.substr(prev, i-prev);
        parsedString.push_back(temp);
        //cout << temp << endl;
        temp = input.substr(i, 1);
        parsedString.push_back(temp);
        //cout << temp << endl;
        prev = i+1;  
      }
      else{
        string temp = input.substr(i, 1);
        parsedString.push_back(temp);
        //cout << temp << endl;
        prev = i+1;
      }
    } // if redirect or pipe
    //i++;
  }
  //cout << "finishes" << endl;
  //cout << "parsed String size : " <<parsedString.size() << endl;
  //cout << parsedString[0] << endl;
  return parsedString; 
}

void lsDirectory(char* newDir, char** args){
  //cerr << "LS" << endl;
  //cerr << newDir << endl;
  //cerr << "args: " << args[1] << endl;
  char* dir;
  int flag = false;
  if(args[0] == NULL){
    return;
  }
  if(args[1] == NULL){
    // then you're doing it right
    dir = newDir;
    //cout << newDir << endl;
      //write(1, dir, strlen(dir));
  }
  else if(args[2] == NULL){
    flag = true;
    //this is going to be the directory name
    //dir = newDir;
    dir = new char(strlen(newDir) + strlen(args[1])+2);
    //if(dir != NULL){
    strcpy(dir, newDir);
    strcat(dir, "/");
    strcat(dir, args[1]);
    strcat(dir, "\0");
      //cout << dir << endl;
    //}
  //write(1, dir, strlen(dir));
  }
  else{
    return;
    //print out errors
  }
  //write(1, dir, strlen(dir));
  //write(1, "\n", 1);
  // checker to check if right directory
  DIR *lsDir;
  struct dirent *pointFile;
  //cout << dir << endl;
  if((lsDir = opendir(dir)) == NULL){
    write(STDOUT_FILENO, "Cannot open the directory\n", 12);
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
  if(flag == true){
    delete[] dir;
  }
  //cout << "finished" << endl;
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
  //cout << "supposed to be in here" << endl;
  if(args[1] == NULL){
    //then by default, you want to go to home
    //cout << "enters here?" << endl;
    char* home;
    home = getenv("HOME");
    if(home == NULL){
      write(1, "Error\n",6);
    }
    //cout << home << endl;
    chdir(home);

  }
  else if (args[2] == NULL){
    //cout << "here instead." << endl;
    char* newDir = new char[strlen(dir)+ strlen(args[1]) + 2];
    if(newDir != NULL){
      strcpy(newDir, dir);
      strcat(newDir, "/");
      strcat(newDir, args[1]);
      //strcat(newDir, "\0");
    }
    //cout << "newDir: " << newDir << endl;
    chdir(newDir);
    delete[] newDir;
  }
  else{
    //cout << "error" << endl;
    return;
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
}

void terminate(){
  exit(0);
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
  //cout << "History begins here" << j << endl;
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
    //print out an error
   }
  }
  else if (strcmp(args[0], "ls") == 0){
    //cout << "goes in here" << endl;
    lsDirectory(dir, args);

    // if we get another directory 
  }
  else if(strcmp(args[0], "cd") == 0){
    changeDirectory(dir, args);
  }
  else{
    execvp(args[0], args);
  }
  exit(0);

}


int main(){
  struct termios SavedTermAttributes;
  vector<string> history;
  char RXChar;
  SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes); 
  int histCount = 0;
  bool readHist = false;
  string readIn = "";
  vector<int> pids;
//  write(STDOUT_FILENO, dir, strlen(dir));

  while(1){
    //cout << "New while loop" << endl;
    char *dir = get_current_dir_name(); 
    //char *dir = "/home/hechac/ecs150";
    //string pathName = getPath(dir);
//    write(1, "hi there", strlen("hi there")); 
    string pathString = getPath(dir);
    char* pathName = new char[(int) pathString.size() + 1];
    strcpy(pathName, pathString.c_str());
    //write(1, "here", strlen("here"));
    
    write(STDOUT_FILENO, pathName, strlen(pathName));
    delete[] pathName;
//    free(pathName);
    // cout << "here?" << endl;
/*
    int pathSize = pathName.size();
    write(STDOUT_FILENO, pathName, pathName.size());
    */ 
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
              if(prevClickedUp == false && (int) history.size() == 0){

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

    //cout << "adding stuff to history!  :) " << endl;
    //cout << readIn << endl;
    if(readIn.size() > 0){
      history.push_back(readIn);
      // add an if statement
      //histCount++;
    }// stores this in history
    //cout << "there's a problem in ListInput " << endl;
    if((int) readIn.size() == 0){
      continue;
    }

    // this is going to the parsing for us
    vector<string> listInput;
    //cout << "listInputsize: " << listInput.size() << endl;
    int prev = 0;
    //cout << input << "X" << endl;
    //cout << input.size() << endl;
    int sizeInput = readIn.size();
    //cout << "input size: " << sizeInput << endl;
    for(int i = 0; i < sizeInput + 1; i++){
      //cout << "i:" << i << endl;
      if(i == sizeInput - 1){
        string temp = readIn.substr(prev, sizeInput-prev);
        listInput.push_back(temp);
        //cout << temp << endl;
      }
      else if(readIn[i] == ' '){
        if((readIn[i-1] == '<' || readIn[i-1] == '>' || readIn[i-1] == '|') && i > 0){
          prev = i + 1;
        }
        else{
          string temp = readIn.substr(prev, i-prev);
          listInput.push_back(temp);
          //cout << temp << endl;
          prev = i+1;  
        }
      } // if space
      else if (readIn[i] == '<' || readIn[i] == '>' || readIn[i] == '|'){
        if(readIn[i-1] != ' '){
          string temp = readIn.substr(prev, i-prev);
          listInput.push_back(temp);
          //cout << temp << endl;
          temp = readIn.substr(i, 1);
          listInput.push_back(temp);
          //cout << temp << endl;
          prev = i+1;  
        }
        else{
          string temp = readIn.substr(i, 1);
          listInput.push_back(temp);
          //cout << temp << endl;
          prev = i+1;
        }
      } // if redirect or pipe
      //i++;
    }

 
    readIn = ""; // reset the string

    // check if the commands return true, if it returns true
    // then you want to add it into the history

    // want to check how many pipes to make
    // want to make sure that you are doing redirection 
    // before you do piping.

    
  //Piping Area
    vector<string> command;
    int pipefd[2];// 0 for in 1 for out
    pid_t pid;
    //int status;
    char **args;
    /*bool first = false;
    bool middle = false;
    bool end = false;
    */
    //bool alMouth = false;
    int prevInPipe = -1;
    bool start = true;
    bool end = false;
    //int numOfPipes = 0;
    int count = 0; //count keeps track of how many commands are in args
    int in, out;
    char** nPtr;
    //int j = 0;
    for(vector<string>::iterator it = listInput.begin(); it != listInput.end(); it++){
      if(*it == "|" || it == listInput.end() - 1){

        //cout << *it << endl;
        if(it == listInput.end() - 1){
          command.push_back(*it);
          count++;
          end = true;
        } // if it's the end, thenw e want to make sure we account for the command

        // check if it's exit or cd 

        if(it != listInput.end() - 1){
          pipe(pipefd);
        } // want to pipe every time it's not the end
  
        //cout << "size of commands: " << command.size() << endl;
        args = new char*[(int)command.size() + 1];
        int sizeOfCommands = (int) command.size();
        for(int i = 0; i < sizeOfCommands + 1; i++){
          //cout << "print out I: " << i << endl;
          if(i == sizeOfCommands){
            //cout << "stuff" << endl;
            args[i] = new char;
            args[i] = NULL;
            //cout << "main directory: "<<dir << endl;
            if(strcmp(args[0], "cd") == 0){
                changeDirectory(dir, args);
                //doCommands(args, dir, history);
              }
            else if (strcmp(args[0], "exit") == 0){
              exit(0);
            }
            else{

              pid = fork();
              //cerr << "HERE?OIjfoeiwjpaf" << endl;
              if (pid < 0){
                write(1, "Error\n", 6);
              }
              else if(pid == 0){
                //cerr << "CHILD" << endl;
                nPtr = new char*[(int)command.size() + 1];
                int counting = 0;
                for(int j = 0; j < count + 1; j++){
                  //cerr << "J: " << j << endl;
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
                    //cerr << nPtr[counting] << endl;
                    //j++;
                    counting++;
                  }

                }

                //cerr << "???" << endl;
                
                
                if(end == true){
                  //cerr << "HERE" << endl;
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
                //cerr << "before do command" << endl;
                doCommands(nPtr, dir, history);
                //cerr << "inside the child " << endl;
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

          //delete[] nPtr[i];
          //delete[] nPtr[i];
        }
        delete[] args;
        //delete[] nPtr;
        //for(int i = (int) counting -)
        //delete[] nPtr;

        command.clear();
        count = 0;
        //j = 0;
      }
      else{

        //vector
        command.push_back(*it);
        count++;
      }
      //cout << *it<< "X" << endl;
    }
    
    for(int ch = 0; ch < (int) pids.size(); ch++){
      waitpid(pids[ch], NULL, 0);
      //wait(&pids[i]);
    }

    listInput.clear();
  }//continue running the program

  return 0;
}

