#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sstream>


using namespace std;

struct job{
  string jobName;
  string how = "Running";
  pid_t jid;
  
};

void handleHELP();
string handleCD(string & path);
void handleEXIT(int n);
void handleEXPORT(const string & s);
void handleKILL(const string & s, int a);
void nope(const string & prefix);
void parse(const string & commands);
string space(const string & check);
string redirect(const string & check);
void getJob(vector <job> jobs);
string checkSpecial(const string & s);
void executingCommands(const string & command, int pipes, bool bg);
void delete_c(vector<char *> & c);
void close_pipe(int pipfd[2]);
vector<string> vectConvert(const string & command);
vector<char *> charConvert(vector<string> & s);
inline void nope_out(const string & sc_name);

int exitstat;
vector <job> jobs;
int jind = 0;
bool bg = false;
bool fg = false;
//pid_t JID;

/**
 *This program parses an argument from a command line.
 *
 *@author Megan Artigues
 *@author Bejamin Burton
 *@author Devin Everage
 *@author Jalysa Garner
 */

int main(int argc, char * argv[]){
  setvbuf(stdout, NULL, _IONBF, 0);
  string commands;
  bool run = true;
  int fdin, fdout, fderr;
 
  fdin = dup(STDIN_FILENO);
  fdout = dup(STDOUT_FILENO);
  fderr = dup(STDERR_FILENO);

  do{
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    // signal(SIGCHLD, SIG_IGN);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string cwd2 = cwd;
    string location = handleCD(cwd2);
    string jobName;
 
    dup2(fdin, STDIN_FILENO);
    dup2(fdout, STDOUT_FILENO);
    dup2(fderr, STDERR_FILENO);

    printf("1730sh:%s$ ", location.c_str());

    getline(cin, commands);
    jobs.push_back(job());
    jobs.at(jind).jobName = commands;
    
   
    if ((signed)commands.find("cd") != -1){
      string path2 = commands;
      string rep = "";
      size_t pos = 0;
      pos = path2.find("cd", pos);
    
      if (pos != string::npos){
	path2.replace(pos, 3, rep);
      }
    
      handleCD(path2);
    
    }else if ((signed)commands.find("help") != -1){
      handleHELP();
    }else if ((signed)commands.find("exit") != -1){
      int n;
      commands = commands.substr(4);
      if (commands.empty() || (commands.length() == 1)){
	run = false;
      }else{
	char * num = new char [commands.length()];
	strcpy(num, commands.c_str());
	n = atoi(num);
	delete [] num;
	handleEXIT(n);
      }	
    }else if ((signed)commands.find("export ") != -1){
      string send = commands.substr(7);
      handleEXPORT(send);
    }else if ((signed)(commands.find("jobs")) != -1){
      getJob(jobs);
    }else if ((signed)(commands.find("fg")) != -1){
      /*      int n;
      commands = commands.substr(3);
     
      char * num = new char [commands.length()];
      strcpy(num, commands.c_str());
      n = atoi(num);
      */
      //     kill (JID, SIGCONT);

      //delete [] num;
    }else if ((commands != "q") && !(commands.empty())){
      parse(commands);
    }
    jind++;

    int stat;
    while((waitpid(-1, &stat, WCONTINUED | WUNTRACED | WNOHANG)) > 0){  
      if(WIFEXITED(stat)){
	if (bg == false){
	  exitstat = WEXITSTATUS(stat);
	  jobs[jind].how = "Exited(" + to_string(exitstat) + ")";
	}
      }else if(WIFSIGNALED(stat)){
	if (bg == false){
	  exitstat = WTERMSIG(stat);
	  jobs[jind].how = strsignal(stat);
	}
	printf("%s\n", strsignal(stat));
      }else if (WIFSTOPPED(stat)){
	jobs[jind].how = "Stopped";
      }else if (WIFCONTINUED(stat)){
	jobs[jind].how = "Continued";
      }
      }

  }while (run);
  return exitstat;
}//main


//##########################################################################


/**
 *Handles jobs printing
 * 
 *@param jobs list of all the processes 
 */
void getJob(vector <job> jobs){
  cout << "JID " << "\tSTATUS " << "\t\t    COMMAND" << endl;
  for(unsigned int c = 0; c < jobs.size(); c++){
    
    if(jobs[c].jobName == "jobs" || jobs[c].jobName == "fg" || jobs[c].jobName == "export" || jobs[c].jobName == "" || jobs[c].jobName == "bg" || jobs[c].jobName == "help" || jobs[c].jobName == "cd"){
    }else{
      
      cout << jobs[c].jid << "\t" << jobs[c].how << "\t\t" << jobs[c].jobName << endl;
      
    }
  }
}
 

//###########################################################################


/**
 *Parses the command line arg and handles redirection.
 *
 *@param commands the command line args
3 */
void parse(const string & commands){
  setvbuf(stdout, NULL, _IONBF, 0);
  string copy = commands;

  int pipes = count(commands.begin(), commands.end(), '|');
  int process = pipes +1;
  (void) process;

  string jobin = "STDIN_FILENO";
  string jobout = "STDOUT_FILENO";
  string joberr = "STDERR_FILENO";

  int n = (count(commands.begin(), commands.end(), ' ')) +1;
  int f =0;
  string * element = new string[n];

  //separates each part by spaces
  for (int i=0; i<n; i++){
    if ((f =(signed)copy.find(" ")) != -1){
      element[i] = copy.substr(0,f);
      copy = copy.substr(f+1);
    }else{
      element[i] = copy;
    }
  }//for

  //handles redirection output
  for (int i=0; i<n; i++){
    if (element[i][0] == '<'){
      int fdin;
      jobin = element[i+1];
       fdin = open(jobin.c_str(), O_RDONLY);
       if (dup2(fdin, STDIN_FILENO) == -1) {
	 nope_out("dup2");
       } // if
    }
    
    if ((element[i][0] == '>') && (element[i][1] != '>')){
      int fdout;
      jobout = element[i+1];
      //jobout += " (truncate)";
      fdout = open(jobout.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (dup2(fdout, STDOUT_FILENO) == -1) {
	nope_out("dup2");
      } // if

    }
    if ((element[i][0] == '>') && (element[i][1] == '>')){
      int fdout;
      jobout = element[i+1];
      //jobout += " (append)";
      fdout = open(jobout.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
      if (dup2(fdout, STDOUT_FILENO) == -1) {
	nope_out("dup2");
      } // if

    }

    if ((element[i][0] == 'e') && (element[i][1] == '>') &&
        (element[i][2] != '>')){
      int fderr;
      joberr = element[i+1];
      //      joberr += " (truncate)";
      fderr = open(joberr.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (dup2(fderr, STDERR_FILENO) == -1) {
	nope_out("dup2");
      } // if
    }
    
    if ((element[i][0] == 'e') && (element[i][1] == '>') &&
        (element[i][2] == '>')){
      int fderr;
      joberr = element[i+1];
      // joberr += " (append)";
      fderr = open(joberr.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
      if (dup2(fderr, STDERR_FILENO) == -1) {
	nope_out("dup2");
      } // if
 
    }
  }//for

  //sends only the commands up to redirection for execution

  int loc;
  string send = commands;
  if ((loc = (signed)(commands.find("&"))) != -1){
    bg = true;
    send.erase(loc-1, 2);
  }
  string sending2 = checkSpecial(send);
  string sending = redirect(sending2);
  executingCommands(sending, pipes, bg);

  /*printf("\nJob STDIN = %s\n", jobin.c_str());
    printf("Job STDOUT = %s\n", jobout.c_str());
    printf("Job STDERR = %s\n", joberr.c_str());

    printf("\n%d pipe(s)\n", pipes);
    printf("%d process(es)\n\n", process);
  */
  //printProcess(element, n, process);
  delete [] element;
}


//###########################################################################


/**
 *Returns only the command line arg up to a redirection
 *
 *@param check the command line args
 *
 *@return check substring to the necessary position
 */
string redirect(const string & check){
  int f = check.find(">");
  int g = check.find("<");
  int h = check.find("e>");
  int temp;

  //handle which one is found first to make sure we return the string up
  //to the first occurrence of a redirection
  if (f>g){
    if (g == -1){
      temp = f;
    }else{
      temp = g;
    }
  }else if (g>f){
    if (f == -1){
      temp = g;
    }else{
      temp = f;
    }
  }else if (g == f){
    temp = g;
  }//if

  if (temp>h){
    if (h!= -1){
      temp = h;
    }else{
      temp = temp;
    }
  }

  if (temp == -1){
    return check;
  }else{
    return check.substr(0, temp);
  }//if
}//redirect 

//###########################################################################

/**
 *Returns only the command line arg up to a redirection
 *
 *@param check the command line args
 *
 *@return check substring to the necessary position
 */
string checkSpecial(const string & s){
  string inputcopy = s;


  int n = count(inputcopy.begin(),inputcopy.end(),'\"');
  int f = 0, temp = 0;
  for(int i = 0; i < n; ++i){


    if((f = (signed)inputcopy.find('\"',temp)) != -1){
      if(f == 0){
        inputcopy.erase(f,1);
      }
      else if(inputcopy[f-1] == '\\'){
        inputcopy.erase(f-1,1);
      }
      else{
        inputcopy.erase(f,1);
      }
      temp = f;
    }
  }
 
  return inputcopy;
}


//###########################################################################


/**
 *Executes the commands
 *
 *@param command the command to execute
 *@param pipes the number of pipes we have to consider
 */
void executingCommands(const string & command, int pipes, bool bg){
  setvbuf(stdout, NULL, _IONBF, 0);

  int n = pipes +1;
  int f =0;
  string * process = new string[n];
  string copy = command;

  

  //parses everything by pipe(process)
  for (int i=0; i<n; i++){
    if ((f =(signed)copy.find("|")) != -1){
      process[i] = copy.substr(0,f);
      process[i] = space(process[i]);
      copy = copy.substr(f+1);
    }else{
      process[i] = copy;
      process[i] = space(process[i]);
    }
  }//for

  int pid;

  
  if (n == 1){
    int stat;
    if ((pid = fork()) == -1) {
      nope_out("fork");
    } else if (pid == 0) {
      //      JID = getpid();

      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
      // signal(SIGCHLD, SIG_DFL);

      vector<string> s = vectConvert(process[0]);
      vector<char *> c = charConvert(s);
      execvp(c.at(0), &c.at(0)); 
      perror("execvp");
      delete_c(c);
      exit(EXIT_FAILURE);
    }

    jobs.at(jind).jid = pid;

    int options = (WCONTINUED | WUNTRACED);
    if (bg == true){
      options = (WCONTINUED | WUNTRACED | WNOHANG);
    }
    waitpid(pid, &stat, options);
    if(WIFEXITED(stat)){
      if (bg == false){
	exitstat = WEXITSTATUS(stat);
	jobs[jind].how = "Exited(" + to_string(exitstat) + ")";
      }
    }else if(WIFSIGNALED(stat)){
      if (bg == false){
	exitstat = WTERMSIG(stat);
	jobs[jind].how = strsignal(stat);
      }
    }else if (WIFSTOPPED(stat)){
      jobs[jind].how = "Stopped";
    }else if (WIFCONTINUED(stat)){
      jobs[jind].how = "Continued";
    }
  }else{
    int stat;
    int ** pipefd = new int*[n-1];
    for (int i=0; i < (n-1); i++){
      pipefd[i] = new int[2];
    }

    for (int i=0; i<(n); i++){//goes through the processes
      if (i==0){
	if (pipe(pipefd[i]) == -1) {
	  nope_out("pipe");
	} // if	
	if ((pid = fork()) == -1) {
	  nope_out("fork");
	} else if (pid == 0) {
	  
	  //  JID = getpid();
	  //tcsetpgrp(0,JID);
	  
	  signal(SIGINT, SIG_DFL);
	  signal(SIGQUIT, SIG_DFL);
	  signal(SIGTSTP, SIG_DFL);
	  signal(SIGTTIN, SIG_DFL);
	  signal(SIGTTOU, SIG_DFL);
	  // signal(SIGCHLD, SIG_DFL);
	  
	  if (dup2(pipefd[i][1], STDOUT_FILENO) == -1) {
	    nope_out("dup2");
	  } // if
	  
	  close_pipe(pipefd[i]);
	   
	  vector<string> s2 = vectConvert(process[i]);
	  vector<char *> c2 = charConvert(s2);

	  execvp(c2.at(0), &c2.at(0));
	  perror("execvp");
	  delete_c(c2);
	  exit(EXIT_FAILURE);
	}
      }//if one process
      else if (i == pipes){ 
	if ((pid = fork()) == -1) {
	  nope_out("fork");
	} else if (pid == 0) {
	  signal(SIGINT, SIG_DFL);
	  signal(SIGQUIT, SIG_DFL);
	  signal(SIGTSTP, SIG_DFL);
	  signal(SIGTTIN, SIG_DFL);
	  signal(SIGTTOU, SIG_DFL);
	  //signal(SIGCHLD, SIG_DFL);

	  if (dup2(pipefd[i-1][0], STDIN_FILENO) == -1) {
	    nope_out("dup2");
	  } // if
	  
	  close_pipe(pipefd[i-1]);
	  
	  vector<string> s4 = vectConvert(process[i]);
	  vector<char *> c4 = charConvert(s4);

	  execvp(c4.at(0), &c4.at(0));
	  perror("execvp");
	  delete_c(c4);
	  exit(EXIT_FAILURE);
	}
	close_pipe(pipefd[i-1]);
      }else{

      
      // if ((i != 0) && (i != (n-1))){//if not first or last process
	if (pipe(pipefd[i]) == -1) {
	  nope_out("pipe");
	} // if
	if ((pid = fork()) == -1) {
	  nope_out("fork");
	} else if (pid == 0) {
	  signal(SIGINT, SIG_DFL);
	  signal(SIGQUIT, SIG_DFL);
	  signal(SIGTSTP, SIG_DFL);
	  signal(SIGTTIN, SIG_DFL);
	  signal(SIGTTOU, SIG_DFL);
	  // signal(SIGCHLD, SIG_DFL);

	  if (dup2(pipefd[i-1][0], STDIN_FILENO) == -1) {
	    nope_out("dup2");
	  } // if
	    
	  if (dup2(pipefd[i][1], STDOUT_FILENO) == -1) {
	    nope_out("dup2");
	  } // if
	  
	  close_pipe(pipefd[i-1]);
	  close_pipe(pipefd[i]);

	  
	  vector<string> s3 = vectConvert(process[i]);
	  vector<char *> c3 = charConvert(s3);
	  execvp(c3.at(0), &c3.at(0));
	  perror("execvp");
	  delete_c(c3);
	  exit(EXIT_FAILURE);
	}
	close_pipe(pipefd[i-1]);
      }//if not first or last

      jobs.at(jind).jid = pid;

      int options;
      if (bg == true){
	options = (WCONTINUED | WUNTRACED | WNOHANG);
      }
      waitpid(pid, &stat, options);
      
      if (WIFEXITED(stat)){
	if (bg == false){
	  exitstat = WEXITSTATUS(stat);
	  jobs[jind].how = "Exited(" + to_string(exitstat) + ")";
	}
      }else if(WIFSIGNALED(stat)){
	if (bg == false){
	  exitstat = WTERMSIG(stat);
	  jobs[jind].how = strsignal(stat);
	}
      }else if (WIFSTOPPED(stat)){
	jobs[jind].how = "Stopped";
      }else if (WIFCONTINUED(stat)){
	jobs[jind].how = "Continued";
      }
  }//for
}
  delete [] process;
}

//###########################################################################

/*
 *Deletes vector<char *>
 *
 *@param c the vector
 */
void delete_c(vector<char *> & c) {
  for (unsigned int i = 0; i < c.size(); ++i) {
    delete[] c.at(i);
  } 
} 


//###########################################################################


/**
 *Closes a pipe to ensure we don't forget them open
 *
 *@param pipefd the pipe(s) to close
 */
void close_pipe(int pipefd [2]) {
  if (close(pipefd[0]) == -1) nope_out("close");
  if (close(pipefd[1]) == -1) nope_out("close");
} // close_pipe


//###########################################################################


/**
 *Converts a string to a vector of type string
 *Used for converting to a vector of char * later
 *Which is used for execvp
 *
 *@param command the command to convert
 *
 *@return sendback the converted vector
 */
vector<string> vectConvert(const string & command){
  vector<string> sendback;
  stringstream stream(command);
  string s;

  while(stream >> s){
    sendback.push_back(s);	
  }
  return sendback;
}//vectConvert


//###########################################################################


/**
 *Converts a vector of type string to a vector of type char *
 *Which is used for execvp
 *
 *@param s the vector to convert
 *
 *@return sendback the converted vector
 */
vector<char *> charConvert(vector<string> & s) {
  vector<char *> sendback;
  for (unsigned int i = 0; i < s.size(); ++i) {
    sendback.push_back(new char [s.at(i).size() + 1]);
    strcpy(sendback.at(i), s.at(i).c_str());
  } // for
  sendback.push_back(nullptr);
  return sendback;
} // charCovnert


//###########################################################################


/**
 *Removes all leading spaces to ensure proper reading
 *
 *@param check the element to check
 *
 *@return sendback the modified string to ensure there are no leading spaces
 */
string space(const string & check){
  string sendback = check;
  int f = sendback.find(" ");

  if (f == 0){
    sendback = sendback.substr(f+1);
  }else{
    sendback = sendback;
  }

  return sendback;
}//space


//###########################################################################


/**
 *Handles the builtin help command.
 */
void handleHELP(){
  setvbuf(stdout, NULL, _IONBF, 0);

  printf("\n-bg JID – Resume the stopped job JID in the background\n");
  printf("-cd [PATH] – Change the current directory to PATH.\n");
  printf("-exit [N] – Cause the shell to exit with a status of N.\n");
  printf("-export NAME[=WORD] – NAME is automatically included in the environment of subsequently executed jobs.\n");
  printf("-fg JID – Resume job JID in the foreground.\n");
  printf("-help – Display helpful information about builtin commands.\n");
  printf("-jobs – List current jobs.\n");
  printf("-kill [-s SIGNAL] PID - kills a specified process.\n\n");
}


//###########################################################################

/**
 *Handles export builtin
 *
 *@param s the values to pass to env
 */

void handleEXPORT(const string & s){
  putenv((char *)s.c_str());
}


//###########################################################################


/**
 *Handles kill builtin
 *
 *@param s the pid and signal
 */
void handleKILL(const string & s, int a){
  string copy = s;
  string * parts = new string[a+1];
  int f=0;

  for (int i=0; i<(a+1); i++){
    if ((f=copy.find(" ")) != -1){
      parts[i] = copy.substr(0,f);
      copy = copy.substr(f+1);
    }else{
      parts[i] = copy;
    }
  }
  
  if (a==2){
    char *x = new char[parts[1].length()];
    strcpy(x, parts[1].c_str());
    int y = atoi(x);
    pid_t z = (pid_t)y;
    
    int signal;
    if(parts[0] == "SIGNULL"){
      signal = 0;
    }else if(parts[0] == "SIGHUP"){
      signal = 1;
    }else if(parts[0] == "SIGINT"){
      signal = 2;
    }else if(parts[0] == "SIGQUIT"){
      signal = 3;
    }else if(parts[0] == "SIGKILL"){
      signal = 9;
    }else if(parts[0] == "SIGTERM"){
      signal = 15;
    }else if(parts[0] == "SIGSTOP"){
      signal = 24;
    }else if(parts[0] == "SIGTSTP"){
      signal = 25;
    }else if(parts[0] == "SIGCONT"){
      signal = 26;
    }
    kill(z, signal);
  }
  //TODO: handle -s in input    
  
}

//###########################################################################


/**
 *Handles exit builtin
 *
 *@param n the status to exit with
 */
void handleEXIT(int n){
  // if (n!= exitstat){
    exit(n);
    // }else{
    // exit(exitstat);
    //}
}


//###########################################################################


/*
*Handles builtin for cd.
*
*@param path the path to change to.
*
*@return cwd2 the current working directory to print out
*/
string handleCD(string & path){
  setvbuf(stdout, NULL, _IONBF, 0);
  char cwd[1024];
  char * home;
  home = getenv("HOME");
  string copyHome = home;

  if (path.empty()){
    if (chdir(home) == 0){
      chdir(home);
    }else{
      nope(path);
    }

    getcwd(cwd, sizeof(cwd));
    
  }else if ((signed)path.find("~") != -1){
    string p2 = path;
    string rep2 = copyHome;
    size_t pos2 = 0;
    pos2 = p2.find("~", pos2);

    if (pos2 != string::npos){
      p2.replace(pos2, 1, rep2);
    }

    if (chdir(p2.c_str()) == 0){
      chdir(p2.c_str());
    }else{
      nope(path);
    }

    getcwd(cwd, sizeof(cwd));
    
  }else if(((signed)path.find(".") != -1) && 
	   ((signed)path.find("..") == -1) ){
    string p3 = path;
    string rep3 = getcwd(cwd, sizeof(cwd));
    size_t pos3 = 0;
    pos3 = p3.find(".", pos3);

    if (pos3 != string::npos){
      p3.replace(pos3, 1, rep3);
    }

    if (chdir(p3.c_str()) == 0){
      chdir(p3.c_str());
    }else{
      nope(path);
    }

  }else if((signed)path.find("..") != -1){
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

  if (pos != string::npos){
    cwd2.replace(pos, copyHome.length(), rep);
  }

  return cwd2;
}


//###########################################################################


/**
 *Error handler.
 *
 *@param prob the prefix to diplay before the error message.
 */
inline void nope_out(const string & sc_name) {
  //  printf("hello");
  perror(sc_name.c_str());
  //exit(EXIT_FAILURE);
} // nope_out


//###########################################################################


/**
 *Other error handler.
 *
 *@param prefix the prefix to diplay before the error message.
 */
void nope(const string & prefix) {
  printf("cd: %s: %s\n", prefix.c_str(), strerror(errno));
  //  exit(EXIT_FAILURE);
} // nope
