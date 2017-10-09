#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cerrno>

using std::string;
using std::cin;

void pathNeeded(const string & path);
void nope(const string & prefix);

int main(int argc, char * argv[]){

  if (argc == 1){
    chdir(getenv("HOME"));
    printf("~");
  }else{
    pathNeeded(argv[1]);
  }

  return EXIT_SUCCESS;
}

void pathNeeded(const string & path){
  char cwd[1024]; 
  char * home;
  home = getenv("HOME");
  string copyHome = home;
  
  if((signed)path.find("..") != -1){
    string dotdot = "..";
    char * up = new char[dotdot.length()+1];
    strcpy(up, dotdot.c_str());

    chdir(up);
    getcwd(cwd, sizeof(cwd));    
    
    string path2 = path;
    string rep = ".";
    size_t pos = 0;
    pos = path2.find(dotdot, pos);
    
    if (pos != string::npos){
      path2.replace(pos, dotdot.length(), rep);
    }
    
    if (chdir(path2.c_str()) == 0){
      chdir(path2.c_str());
    }else{
      nope(path);
    }
    
    getcwd(cwd, sizeof(cwd));    
    
    delete [] up;
  }else{
    char * change = new char[path.length()+1];
    strcpy(change, path.c_str());
    
    if (chdir(change) == 0){
      chdir(change);
    }else{
      nope(path);
    }
    
    getcwd(cwd, sizeof(cwd));
  
    
    delete [] change;
  }

  string cwd2 = cwd;
  string rep = "~";
  size_t pos = 0;
  pos = cwd2.find(copyHome, pos);
  
  // if (pos != string::npos){
  //   cwd2.replace(pos, copyHome.length(), rep);
  // }
  
  printf("CWD: %s", cwd2.c_str());
  
}//pathNeeded
  
void nope(const string & prefix) {
  printf("cd: %s: %s:\n", prefix.c_str(), strerror(errno));
  exit(EXIT_FAILURE);
} // nope
