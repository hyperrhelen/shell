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
  cout << dir << endl;
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
  //cout << "argument: do command: " << args[0] <<"X" << endl;
  //cout << "doCommands dir: " << dir << endl; 
  if(strcmp(args[0], "history") == 0){
    if(args[1] == NULL){
      getHistory(history);
    }
    else{
      //cout << "hey stop" << endl;
      //return an error
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
  //cout << "return from doCommands: " << endl;
}


int main(){
  struct termios SavedTermAttributes;
  vector<string> history;
  char RXChar;
  SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes); 
  int histCount = 0;
  bool readHist = false;
  string readIn = "";
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
    //cout << "gmorning natty" << endl;
    //vector<string> listInput = parse(readIn);



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
    //cout << "finishes" << endl;
    //cout << "parsed String size : " <<listInput.size() << endl;
    //cout << listInput[0] << endl;


  // end of the parsing









    //vector<string> ourList = parse(readIn);

    //cout << "somethings wrong here" << endl;
    //vector<string> listInput = parse(readIn);
    //cout << listInput[0] << endl;
 
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
    int status;
    char **args;
    int count = 0; //count keeps track of how many commands are in args
    for(vector<string>::iterator it = listInput.begin(); it != listInput.end(); it++){
      if(*it == "|" || it == listInput.end() - 1){
        //cout << *it << endl;
        if(it == listInput.end() - 1){
          //cout << "enters here" << endl;
          //char* temp = new char[ (int) (*it).size() + 1];
          //strcpy(temp, (*it).c_str());
          //commands.push_back(temp);
          command.push_back(*it);
          count++;
          //cout << *it <<"X" << endl;
          //delete[] temp;
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

            //pipe(pipefd);
            pid = fork();

            if(pid == 0){
              dup2(pipefd[1], 1);
              close(pipefd[1]);
              close(pipefd[0]);
              doCommands(args, dir, history);
            }
            else if (pid < 0){
              //error
            }
            else{
              dup2(pipefd[1], 1);
              close(pipefd[1]);
              close(pipefd[0]);
              close(1);
              wait(&status);

            }//parent
/*
            for(int i = 0; i < count; i++){
              if(i == count - 1){
                //close(in);
                //close(out);
                close(pipe[1]);
                close(pipe[0]);
                bool deleteFlag = false;

                for(int j = 0; j < count; j++){
                  if(deleteFlag == false &&  (strcmp(args[j], "<") || strcmp(args[j], ">"))){
                    deleteFlag = true;
                    args[j] = NULL;
                  }
                  else if (deleteFlag == true){
                    args[j] = NULL;
                  }
                }


              }
              if(strcmp(args[i], "<")){
                //pipe[0] = args[i+1];
                pipe[0] = open(args[i+1], O_RDONLY);
                //dup2(pipe[0], 0);
              }//in
              else if(strcmp(args[i], ">")){
                pipe[1] = open(args[i+1], O_WRONLY | O_TRUNC, O_CREAT, 
                  S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                //dup2(pipe[1], 1);
              }//out
            }

            if(pid == 0){
              if(pipe[0] != -1){
                // then you dup it.
                dup2(pipe[0], 0);
              }
              if(pipe[1] != -1){
                dup2(pipe[1], 1);
              }
              doCommands(args, dir, history);
              close(pipe[0]);
              close(pipe[1]);

            }*/





            //doCommands(args, dir, history);
          }
          else{
            //cout << "other things" << endl;
            args[i] = new char[(int) command[i].size() + 1];
            strcpy(args[i], command[i].c_str());
          }
        }
        // want to delete/free/deallocate the arguments over here
        for(int i = (int) command.size() - 1; i >=0; i--){
          delete[] args[i];
        }
        delete[] args;
        //cout << "clears out the vector " << endl;
        command.clear();
        //cout << "actually. it didn't clear out " << endl;
        // do the piping stuff over here
//        write(1, "enters here\n", strlen("enters here\n"));
        //clear out the vector

      }
      else{
        /*char* temp = new char[ (int) (*it).size() + 1];
        strcpy(temp, (*it).c_str());
*/
        //command.push_back(temp);
        command.push_back(*it);
        count++;
        //cout << "temp: " << temp << endl;
        //delete[] temp;
      }
      //cout << *it<< "X" << endl;
    }
  
    listInput.clear();
    //delete dir;
    //cout << "going to restart now" << endl;
    // delete the arguments that were created
  }//continue running the program

  return 0;
}

