#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

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
///SmallShell start
SmallShell::SmallShell(){
// TODO: add your implementation
    plastPwd= ""; ///check that

}

SmallShell::~SmallShell() {
// TODO: add your implementation
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
    // else {
    // return new ExternalCommand(cmd_line);
    //}

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    //for example:
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}



char *SmallShell::GetLastPwd() {
    return plastPwd;
}

///SmallShell end
///Command start
Command::Command(const char* cmd_line) {
    cmd_line=cmd_line;
    int check=_parseCommandLine(cmd_line,args);


}
Command::~Command(){
}
BuiltInCommand::BuiltInCommand(const char *cmdLine) : Command(cmdLine) {

}
///Command end
///GWD start
GetCurrDirCommand::GetCurrDirCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}


void GetCurrDirCommand::execute() {
    printf("%s\n",get_current_dir_name());
}
///GWD end
///CD Start
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
///CD end/home/student/Desktop/OP_HW1Git/CMakeLists.txt
