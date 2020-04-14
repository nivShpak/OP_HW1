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

#define FAIL -1
#define SUCCESS 0

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
        args[i] = (char*)malloc(s.length()+1); ///todo free malloc at D'stractor
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

///==========================================================================================
///   Smash
SmallShell::SmallShell() :prompt("smash>"),lastPwdSmash(get_current_dir_name()),commandVectorSmash(nullptr){
// TODO: add your implementation
jobsListSmash=new JobsList();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
delete jobsListSmash;
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
    char* command_args[COMMAND_MAX_ARGS+1];
  
    if(_parseCommandLine(cmd_line,command_args)==0){
        return nullptr;//empty command
    }
    if (strcmp(command_args[0],"pwd")==0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (strcmp(command_args[0],"cd")==0) {
        cmd_line=cmd_line;
        //char** platPwdp=&this->lastPwdSmash;
        return new ChangeDirCommand(cmd_line,(*this));
    }
    else if (strcmp(command_args[0],"chprompt")==0)  {
      return new chprompt((*this),cmd_line);
    }
    else if (strcmp(command_args[0],"showpid")==0) {
       return new ShowPidCommand(cmd_line);
    }
    else if (strcmp(command_args[0],"jobs")==0) {
        if(jobsListSmash->GetMaxJobid()==0)
            return nullptr;
        jobsListSmash->sortAndDelete();
        return new JobsCommand(cmd_line,jobsListSmash);
    }
    else if (strcmp(command_args[0],"kill")==0){
        return new KillCommand(cmd_line, (this->jl));
    }
    else if (strcmp(command_args[0],"fg")==0) {
        return new ForegroundCommand(cmd_line,jobsListSmash);
    }
    else if (strcmp(command_args[0],"bg")==0) {
        return new BackgroundCommand(cmd_line,jobsListSmash);
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

void SmallShell::setLastPwd(char*& dir) {
    if(strcmp(strcpy(lastPwdSmash,dir),dir)!=0);
}

///==========================================================================================
///   Command

Command::Command(const char* cmd_line) {
    cmd_line=cmd_line;
    int check=_parseCommandLine(cmd_line,args);
}

Command::Command(const char *cmd_line, SmallShell &smash) {
    cmd_line=cmd_line;
    int check=_parseCommandLine(cmd_line,args);
    //if(check==0) there was no command
    cmd_smash=&smash;
}

Command::~Command(){
}


string Command::GetCmd_line() {
    return cmd_line;
}


BuiltInCommand::BuiltInCommand(const char *cmdLine) : Command(cmdLine) {
}

BuiltInCommand::BuiltInCommand(const char *cmd_line, SmallShell &smash) : Command(cmd_line, smash){
}


///==========================================================================================
///   GWD
GetCurrDirCommand::GetCurrDirCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {
}


void GetCurrDirCommand::execute() {
    cout<<get_current_dir_name()<<endl;
}

///==========================================================================================
///   CD
ChangeDirCommand::ChangeDirCommand(const char *cmdLine,SmallShell& smash) : BuiltInCommand(cmdLine,smash) {
}

ChangeDirCommand::ChangeDirCommand(const char *cmdLine,  char** plastPwd= nullptr) : BuiltInCommand(cmdLine) {
    last_pwd=*plastPwd; //fix it, problem when cd isnt legal
    char* current_dir=get_current_dir_name();
    if(strcmp(*plastPwd,current_dir)!=0) //we cant come back to the same dir
        *plastPwd=current_dir;
}

void ChangeDirCommand::execute() {
    char* current_dir=get_current_dir_name();
    if(args[2]!= nullptr){
        cout<<"smash error: cd: too many arguments"<<endl;
        return;
    }
    if(strcmp(args[1],"-")==0) {
        if(strcmp(cmd_smash->GetLastPwd(),current_dir)==0){
           cout<<"smash error: cd: OLDPWD not set"<<endl;
            return;
        }
        if(chdir(cmd_smash->GetLastPwd())==FAIL){
            ///perror
            return;
        }
        cmd_smash->setLastPwd(current_dir);
        return;
    }

    if(chdir(args[1])==FAIL){
        ///perror
        return;
    }
    cmd_smash->setLastPwd(current_dir);
}

///==========================================================================================
///      Crprompt

Chprompt::Chprompt(SmallShell* s, const char *cmd_line):BuiltInCommand(cmd_line) {
    this->smash = s;
    if (this->num_of_arg==1) {

chprompt::chprompt(SmallShell &s, const char *cmd_line):BuiltInCommand(cmd_line), smash(s) {
    char* args[20];
    int num = _parseCommandLine(cmd_line, args);
    if (num==1) {
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
        
///==========================================================================================
///      
  
unsigned int FgBgCheck(char** args, JobsList* jobs, const char *s, JobsList::JobEntry* job){
    unsigned int jid=0;
    if(args[1]== nullptr)
        jid=jobs->GetMaxJobid();
    else {
        try {
            jid = stoul(args[1]);
        }
        catch(invalid_argument){
            cout<<"smash error: "<<s<<": invalid arguments"<<endl;
            throw FgBgException();
        }
        //error massages
    }
    if((strcmp(s,"fg")==0)&& jobs->GetMaxJobid()==0){
        cout<<"smash error: "<<s<<": jobs list is empty"<<endl;
        throw FgBgException();
    }
    if((strcmp(s,"bg")==0)&& jobs->GetLastStoppedJobId()==0){
        cout<<"smash error: "<<s<<": there is no stopped jobs to resume"<<endl;
        throw FgBgException();
    }

    job=jobs->getJobById(jid);
    if(job== nullptr) {
        cout << "smash error: "<<s<<": job-id " << jid << " does not exist" << endl;
        throw FgBgException();
    }
    if((strcmp(s,"bg")==0)&&job->GetJobState()==BgState){
        cout << "smash error: "<<s<<": job-id " << jid << " dis already running in the background" << endl;
        throw FgBgException();
    }
    cout<<job->GetJobCmdLine()<<" : "<<job->GetJobId()<<endl;
    unsigned int jPid=job->GetJobPid();
    return jPid;
}

///==========================================================================================
///     Fg   
ForegroundCommand::ForegroundCommand(const char *cmdLine, JobsList* jobs) : BuiltInCommand(cmdLine),jobsList_fgCommand(jobs) {


}


void ForegroundCommand::execute() {
    unsigned int jPid=0;
    unsigned int jid;
    JobsList::JobEntry* job;
    try{
        jPid=FgBgCheck(args,jobsList_fgCommand,"fg",job);
    }
    catch(FgBgException& e){
        return;
    }
    jid=job->GetJobId();
    jobsList_fgCommand->removeJobById(jid);
    jobsList_fgCommand->sortOnly();
    int checkKill=kill(jPid, SIGCONT);
    int wstatus;
    if(checkKill==SUCCESS)
        waitpid(jPid, &wstatus,0 ); //maybe we want wstatus?




}
///==============================================================================
///   Bg
BackgroundCommand::BackgroundCommand(const char *cmdLine, JobsList* jobs) : BuiltInCommand(cmdLine),jobsList_bgCommand(jobs) {
}


void BackgroundCommand::execute() {
    unsigned int jPid=0;
    unsigned int jid;
    JobsList::JobEntry* job;
    try{
        jPid=FgBgCheck(args,jobsList_bgCommand,"bg",job);
    }
    catch(FgBgException& e){
        return;
    }
    jid=job->GetJobId();
    jobsList_bgCommand->getJobById(jid)->SetJobState(BgState);
    int checkKill=kill(jPid, SIGCONT);
    //if(checkKill==SUCCESS)
    //maybe we want wstatus?
    jobsList_bgCommand->removeJobById(jid);

}
      
///==========================================================================================
///   Jobs
      
JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobsList_jobsCommand(jobs){
}


void JobsCommand::execute() {
    jobsList_jobsCommand->printJobsList();
}

///==========================================================================================
///   jobs list

JobsList::JobsList():maxJobId(0) {
}

void JobsList::removeFinishedJobs() {
    int size = jobsVector.size();
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it) {
        if (it->GetJobState() == FinishState) {
            if (maxJobId < it->GetJobId()) {
                this->SetMaxJobid(jobsVector[size - 1].GetJobId());
                size--;
            }
            jobsVector.erase(it);
        }
    }
}
      
void JobsList::sortOnly(){
    sort(jobsVector.begin(), jobsVector.end());
}
      
void JobsList::sortAndDelete(){
    removeFinishedJobs(); //vector neede alaways to be sorted
    this->sortOnly();
}

unsigned int JobsList::GetMaxJobid() {
    return maxJobId;
}
      
unsigned int JobsList::GetLastStoppedJobId() {
    int jid=0;
    getLastStoppedJob(&jid);
    return jid;
}
      
void JobsList::SetMaxJobid(unsigned int new_maxid) {
    maxJobId=new_maxid;
}

unsigned int JobsList::GetPidByJid(unsigned int Jid) {
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it) {
        if (it->GetJobId() == Jid) {
            return it->GetJobPid();
        }
    }
    return FAIL;
}

void JobsList::printJobsList() {
    int size = jobsVector.size();
    for (int i=0; i<size; ++i) {
        cout<<jobsVector[i];
    }
}

ostream &operator<<(ostream &os, JobsList::JobEntry &je) {
    State je_State=je.GetJobState();
    char* s= nullptr;
    if(je_State==StoppedState){
        strcpy(s,"(stooped\n");
    } else
        strcpy(s,"\n");
    os<<"["<<je.GetJobId()<<"]"<<je.GetJobCmdLine()<<": "<<je.GetJobPid()<<je.GetJobElapsed()<<s<<endl;
    return os;
}

void JobsList::killAllJobs() {
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it)
        jobsVector.erase(it);
}
      
void JobsList::addJob(Command *cmd, bool isStopped) {
    jobsVector.push_back(JobEntry(maxJobId+1,  cmd));
    maxJobId++;
}
      
void JobsList::removeJobById(int jobId) {
    int size = jobsVector.size();
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it) {
        if (it->GetJobId() == jobId) {
            if (maxJobId == it->GetJobId()) {
                this->SetMaxJobid(jobsVector[size - 1].GetJobId());
            }
            jobsVector.erase(it);
            return;
        }
    }
}
      
JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    if(jobsVector.empty())
        return nullptr;
    return &jobsVector[jobsVector.size()-1];
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    if(jobsVector.empty())
        return nullptr;
    int size = jobsVector.size();
    for (int i = size-1; i < size; --i) {
        if(jobsVector[i].GetJobState()==StoppedState)
            *jobId=jobsVector[i].GetJobId();
            return &jobsVector[i];
    }
}


///==========================================================================================
///   job entry
      
JobsList::JobEntry::JobEntry(unsigned int jid, Command* command):jobId(jid),commandJob(command) {
    jobStart=time(nullptr);
    jobPid=getpid();
    jobState=BgState;

}
bool JobsList::JobEntry::operator<(const JobsList::JobEntry &jobEntry) const {
    return this->jobId<jobEntry.jobId;
}


unsigned int JobsList::JobEntry::GetJobId() {
    return jobId;
}

unsigned int JobsList::JobEntry::GetJobPid() {
    return jobPid;
}

double JobsList::JobEntry::GetJobElapsed() {
    time_t currentTime;
    currentTime=time(nullptr);
    return difftime(jobStart,currentTime);///check about fault
}
      
State JobsList::JobEntry::GetJobState() {
    return jobState;
}
      
void JobsList::JobEntry::zeroJobStart() {
    time_t currentTime;
    currentTime=time(nullptr);
    jobStart=currentTime;

}

void JobsList::JobEntry::SetJobState(State newState) {
    jobState=newState;

}
      
string JobsList::JobEntry::GetJobCmdLine() {
    return this->commandJob->GetCmd_line();
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    int size = jobsVector.size();
    for (int i=0; i<size; ++i) {
        if(i==jobId)
            return &jobsVector[i];
    }
    return nullptr;
}

///==========================================================================================
///   