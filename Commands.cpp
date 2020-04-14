#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include "signals.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

void freeArgArray(char* arg[], int n){
    for (int i=0 ; i<n ; ++i){
        free(arg[i]);
    }
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

///==========================================================================================
///   Smash

SmallShell::SmallShell() :prompt("smash>") {
// TODO: add your implementation
    plastPwd= (char*)""; ///check that

}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

const string SmallShell::getPrompt() const{
  return prompt;
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

Command * SmallShell::CreateCommand(const char* cmd_line) {

    // For example:

    string cmd_s = string(cmd_line);

    if (cmd_s.find("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (cmd_s.find("cd") == 0){
        cmd_line=cmd_line;
        char** platPwdp=&this->plastPwd;
        return new ChangeDirCommand(cmd_line,platPwdp);
    }
    else if (cmd_s.find("chprompt") == 0) {
      return new Chprompt(this,cmd_line);
    }
    else if (cmd_s.find("showpid") == 0){
       return new ShowPidCommand(cmd_line);
    }
    else if (cmd_s.find("kill")==0){
        return new KillCommand(cmd_line, (this->jl));
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    //for example:
    Command* cmd = CreateCommand(cmd_line);
    if (cmd==NULL)
        return; //todo: maybe error
    cmd->execute();
    delete cmd;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::setPrompt(string new_prompt) {
    this->prompt=new_prompt;
}


char *SmallShell::GetLastPwd() {
    return plastPwd;
}


///==========================================================================================
///   Command
Command::Command(const char* cmd_line) {
    cmd_line=cmd_line;
    int check=_parseCommandLine(cmd_line,args);
}
Command::~Command(){
}
BuiltInCommand::BuiltInCommand(const char *cmdLine) : Command(cmdLine) {

}

///==========================================================================================
///      PWD

GetCurrDirCommand::GetCurrDirCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}


void GetCurrDirCommand::execute() {
    printf("%s\n",get_current_dir_name());
}

///==========================================================================================
///      CD Class

ChangeDirCommand::ChangeDirCommand(const char *cmdLine,  char** plastPwd= nullptr) : BuiltInCommand(cmdLine) {
    last_pwd=*plastPwd; //fix it, problem when cd isnt legal
    char* current_dir=get_current_dir_name();
    if(strcmp(*plastPwd,current_dir)!=0) //we cant come back to the same dir
        *plastPwd=current_dir;

}
void ChangeDirCommand::execute() {
    if(args[2]!= nullptr){
        printf("smash error: cd: too many arguments\n");
        return;
    }
    if(strcmp(args[1],"-")==0) {
        if(last_pwd==""){
            printf("smash error: cd: OLDPWD not set\n");
            return;
        }
        chdir(last_pwd);
        return;
    }

    chdir(args[1]);
}
///==========================================================================================
///      Crprompt

Chprompt::Chprompt(SmallShell* s, const char *cmd_line):BuiltInCommand(cmd_line) {
    this->smash = s;
    if (this->num_of_arg==1) {
        this->prompt ="smash>";
    }
    else {
        this->prompt = string(args[1])+=">";
    }
}

Chprompt::~Chprompt(){};

void Chprompt::execute() {
        smash->setPrompt(this->prompt);
}
///==========================================================================================
///      Showpid

void ShowPidCommand::execute(){
    cout<<"smash pid is " << getpid() <<endl;
}
///==========================================================================================
///      Kill

KillCommand::KillCommand(const char *cmd_line, JobsList& jobs) : BuiltInCommand(cmd_line),jl(jobs){
      if (this->num_of_arg==3 & args[1][0]=='-') {
        string id = to_string(stoi(args[2]));
        int signal = stoi(args[1]+1);
        string sig_string = to_string(signal);
        if (id==args[2] & sig_string==(args[1]+1) & isLegalSignal(signal)){
            this->jobID = stoi(id);
            this->signal = signal;
        }
    } else this->signal=(0);
}

void KillCommand::execute() {
    if (this->signal==0){
        cout<<"smash error: kill: invalid argument"<<endl;
        return;
    }
    unsigned int pid = this->jl.getPidByJid(this->jobID);
    if (pid==0) {
        cout<<"smash error: kill: job-id "<<this->jobID<<" does not exist"<<endl;
        return;
    }
    if (kill(pid,this->signal)!=0){
        perror("smash error: kill failed");
    }
    cout<<"signal number "<< this->signal <<" has sent to pid "<< pid <<endl;
}

///==========================================================================================
///      Quit

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) :BuiltInCommand(cmd_line){
    jl = jobs;
}

void QuitCommand::execute() {
    if (strcmp(args[1],"kill")==0)
        jl->killAllJobs();
    delete jl;
    throw "Quit smash";
}
