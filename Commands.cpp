#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include "Commands.h"
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>



#define FAIL -1
#define SUCCESS 0
#define SPACE ' '
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
__pid_t smash_pid = getpid();
__pid_t front_pid = 0;
SmallShell* smash_glob= nullptr;
__pid_t getSmashPid (){
    return smash_pid;
}

__pid_t getFrontPid (){
    return front_pid;
}
SmallShell* getSmashGlob (){
    return smash_glob;
}
#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));
bool is_pid_running(pid_t pid);
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

/*int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1); ///todo free malloc at D'stractor, maby chang func to std::vector
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = nullptr;
    }
    return i;
    FUNC_EXIT()
}*/

int _parseCommandLine(const char* cmd_line, vector<string>& args){
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ++i ) {
        args.push_back(s);
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
    //cmd_line[str.find_last_not_of(WHITESPACE, idx)] = 0;
    cmd_line[str.find_last_not_of(WHITESPACE, idx)+1] = 0;
}





// TODO: Add your implementation for classes in Commands.h

///==========================================================================================
///   Smash
SmallShell::SmallShell() :prompt("smash>"){
// TODO: add your implementation
   /// char* tmp_pwd= get_current_dir_name();
    lastPwdSmash = "";
    ///free (tmp_pwd);
    jobsListSmash=new JobsList();
    timeOutListSmash=new TimeOutList();
    smash_glob=this;
}

SmallShell::~SmallShell() {
// TODO: add your implementation
    for (auto it = commandVectorSmash.begin(); it != commandVectorSmash.end(); ++it) {
        delete *it;
    }
    delete jobsListSmash;
    delete timeOutListSmash;
}

void SmallShell::DeleteAll() {
// TODO: add your implementation
    for (auto it = commandVectorSmash.begin(); it != commandVectorSmash.end(); ++it) {
        delete *it;
    }
    delete jobsListSmash;
    delete timeOutListSmash;
}
const string SmallShell::getPrompt() const{
  return prompt;
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/


bool emptyCommand(const char* cmd){
    int len = strlen(cmd);
    for(int i=0 ; i<len ; ++i){
        if (cmd[i]!=' ')
            return false;
    }
    return true;
}



Command * SmallShell::CreateCommand(const char* cmd_line,RedPipOther redPipOther=OtherCmd,Command* realCmd= nullptr, bool isTimeOut=false,
                                    time_t duration=0) {

    // For example:

    if(emptyCommand(cmd_line)){
        throw EmptyCommandException();//empty command
    }
    bool isFirst=true;
    string cmd_s = string(cmd_line);
    vector<string> command_args;
    char built_in_cmd_line[COMMAND_ARGS_MAX_LENGTH];
    strcpy(built_in_cmd_line,cmd_line);
    _removeBackgroundSign(built_in_cmd_line);
    _parseCommandLine(built_in_cmd_line,command_args);
    if((cmd_s.find(">")!=string::npos) && cmd_s.find(">>") == string::npos&&(command_args[0]!="timeout")){
        return new RedirectionCommand(cmd_line,*this,isFirst,isTimeOut,duration);
    }
    else if((cmd_s.find(">>")!=string::npos)&&(command_args[0]!="timeout")){
        isFirst= false;
        return new RedirectionCommand(cmd_line,*this,isFirst,isTimeOut,duration);

    }
    else if((cmd_s.find("|")!=string::npos) && cmd_s.find("|&") == string::npos&&(command_args[0]!="timeout")){
        return new PipeCommand(cmd_line,*this,isFirst,isTimeOut,duration);
    }
    else if((cmd_s.find("|&")!=string::npos)&&(command_args[0]!="timeout")){
        isFirst= false;
        return new PipeCommand(cmd_line,*this,isFirst,isTimeOut,duration);
    }

    if (command_args[0]=="pwd") {
        return new GetCurrDirCommand(built_in_cmd_line);
    }
    else if (command_args[0]=="cd") {
        return new ChangeDirCommand(built_in_cmd_line,*this);
    }
    else if (command_args[0]=="chprompt")  {
        return new Chprompt(this,built_in_cmd_line);
    }
    else if (command_args[0]=="showpid") {
       return new ShowPidCommand(built_in_cmd_line);
    }
    else if (command_args[0]=="jobs") {
        jobsListSmash->sortAndDelete();
        return new JobsCommand(built_in_cmd_line,jobsListSmash);
    }
    else if (command_args[0]=="kill"){
        return new KillCommand(built_in_cmd_line, jobsListSmash);
    }
    else if (command_args[0]=="fg") {
        return new ForegroundCommand(built_in_cmd_line,jobsListSmash);
    }
    else if (command_args[0]=="bg") {
        return new BackgroundCommand(built_in_cmd_line,jobsListSmash);
    }
    else if (command_args[0]=="quit") {
        return new QuitCommand(built_in_cmd_line,this->jobsListSmash);
    }
    else if (command_args[0]=="cp"){
        Run run = (_isBackgroundComamnd(cmd_line))? Back: Front;
        return new CpCommand(built_in_cmd_line,*this, run);
    }
    else if (command_args[0]=="timeout") {
        timeOutListSmash->sortAndDelete();
        return new TimeOutCommand(cmd_line,timeOutListSmash,*this);
    }
    else {
        return new ExternalCommand(cmd_line, *this,redPipOther,realCmd,isTimeOut,duration); ///its refernce for command
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line,RedPipOther redPipConst, Command* realCmd, bool isTimeOut, time_t duration) {
    // TODO: Add your implementation here
    Command* cmd = CreateCommand(cmd_line,redPipConst,realCmd,isTimeOut,duration);

    if (cmd==NULL)
        throw SmallShellException(); //todo: maybe error
    this->commandVectorSmash.push_back(cmd);//so we can delete all cmds at the end of program;
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::setPrompt(string new_prompt) {
    this->prompt=new_prompt;
}


string SmallShell::GetLastPwd() {
    return this->lastPwdSmash;
}

void SmallShell::setLastPwd(string dir) {
    lastPwdSmash = dir;
}

void SmallShell::addJob(Command *cmd,pid_t pid, State state = BgState,RedPipOther redPipOther=OtherCmd,Command* realCmd= nullptr) {
    this->jobsListSmash->addJob(cmd,pid, state,redPipOther,realCmd);
}

TimeOutList *&SmallShell::GetTimeOutList() {
    return timeOutListSmash;
}

void SmallShell::addTimeOut(Command *cmd,pid_t pid, time_t duration) {
    this->timeOutListSmash->addTimeOut(cmd,pid,duration);
}





///==========================================================================================
///   Command

Command::Command(const char* cmd_lineCons):cmd_line(cmd_lineCons) {
    this->num_of_arg = _parseCommandLine(cmd_line.c_str(),this->args);
    cmd_pid=getpid();
}

Command::Command(const char *cmd_lineCons, SmallShell &smash,bool isTimedOutConst, time_t durationConst): cmd_line(cmd_lineCons) {
    this->num_of_arg = _parseCommandLine(cmd_lineCons,this->args);
    cmd_smash=&smash;
    cmd_pid=getpid();
    isTimeOut=isTimedOutConst;
    duration=durationConst;
}

Command::~Command(){
}


string Command::GetCmd_line() {
    return cmd_line;
}

unsigned int Command::GetCmd_pid() {
    return cmd_pid;
}


BuiltInCommand::BuiltInCommand(const char *cmdLine) : Command(cmdLine) {
}

BuiltInCommand::BuiltInCommand(const char *cmd_line, SmallShell &smash) : Command(cmd_line, smash){
}


///==========================================================================================
///   GWDCommand
GetCurrDirCommand::GetCurrDirCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {
}


void GetCurrDirCommand::execute() {
    char* tmp_pwd= get_current_dir_name();
    cout<<tmp_pwd<<endl;
    free (tmp_pwd);

}

///==========================================================================================
///   CDCommand
ChangeDirCommand::ChangeDirCommand(const char *cmdLine,SmallShell& smash) : BuiltInCommand(cmdLine,smash) {
    //this->cmd_smash = smash;
}

ChangeDirCommand::ChangeDirCommand(const char *cmdLine,  char** plastPwd= nullptr) : BuiltInCommand(cmdLine) {
    last_pwd=*plastPwd; //fix it, problem when cd isnt legal
    char* current_dir=get_current_dir_name();
    if(strcmp(*plastPwd,current_dir)!=0) //we cant come back to the same dir
        *plastPwd=current_dir;
    free (current_dir);
}

void ChangeDirCommand::execute() {
    char* current_dir=get_current_dir_name();
    if(args.size() == 1){
        cmd_smash->setLastPwd(string(current_dir));
        free (current_dir);
        ///perror("smash error: chdir failed");
        return;
    }
    if(args.size() > 2){
        cerr<<"smash error: cd: too many arguments"<<endl;
        free (current_dir);
        return;
    }
    if(args[1]=="-") {
        if(cmd_smash->GetLastPwd()==""){
           cerr<<"smash error: cd: OLDPWD not set"<<endl;
            free (current_dir);
            return;
        }
        if(chdir(cmd_smash->GetLastPwd().c_str())==FAIL){
            ///perror
            perror("smash error: chdir failed");
            free (current_dir);
            return;
        }
        cmd_smash->setLastPwd(current_dir);
        free (current_dir);
        return;
    }

    if(chdir(args[1].c_str())==FAIL){
        ///perror
        perror("smash error: chdir failed");
        free (current_dir);
        return;
    }
    cmd_smash->setLastPwd(current_dir);
    free (current_dir);
}

///==========================================================================================
///      CrpromptCommand

Chprompt::Chprompt(SmallShell* s, const char *cmd_line):BuiltInCommand(cmd_line) {
    this->cmd_smash = s;
    if (this->num_of_arg==1)
        this->prompt ="smash> ";
    else {
        this->prompt = string(args[1])+=">";
    }
    ///todo: delete _args
}



void Chprompt::execute() {
        cmd_smash->setPrompt(this->prompt);
}
///==========================================================================================
///      ShowpidCommand

void ShowPidCommand::execute(){
    cout<<"smash pid is " << getSmashPid() <<endl;
}

///==========================================================================================
///      KillCommand

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line),jl(jobs){
    this->signal=0;
    if (this->num_of_arg==3) {
        if (args[1][0]=='-' /*& args[2][0]!='-'*/) {
            int id;
            try {id = stoi(args[2]);}
                catch (...){
                    id=0;
                }
                string id_s = to_string(id);
            int signal = stoi(args[1].c_str() + 1);
            string sig_string = to_string(signal);
            if ((id_s == args[2]) & (sig_string == (args[1].c_str() + 1)) & (signal >= 1 )& (signal <= 31)) {
                this->jobID = id;
                this->signal = signal;
            }
        }
    }
}

void KillCommand::execute() {
    if (this->signal==0){
        cerr<<"smash error: kill: invalid argument"<<endl;
        return;
    }
    int pid = this->jl->GetPidByJid(this->jobID);
    if (pid==0) {
        cerr<<"smash error: kill: job-id "<<this->jobID<<" does not exist"<<endl;
        return;
    }
    if (kill(pid,this->signal)!=0){
        perror("smash error: kill failed");
        //cout<<"here2"<<endl;// its a a mistake?
    }
    cout<<"signal number "<< this->signal <<" has sent to pid "<< pid <<endl;
}

///==========================================================================================
///      QuitCommand

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) :BuiltInCommand(cmd_line){
    this->num_of_arg = _parseCommandLine(cmd_line,this->args);
    jl = jobs;
}

void QuitCommand::execute() {
    if (this->num_of_arg>1) {
        for (int i=1 ; i<num_of_arg; ++i){
            if (args[i]=="kill") {
                jl->killAllJobs();
            }
        }
    }
    ///todo: free all allocate not include smash which be free at the end of main.
    throw string("killme"); ///todo: throw something more logical and destroy this command.
    }
        
///==========================================================================================
///      FgBgCheck
  
unsigned int FgBgCheck(vector<string> args, JobsList* jobs, const char *s, unsigned int* jobId,RedPipOther* redPipOther){
    jobs->removeFinishedJobs();
    bool isFg=true;
    if(strcmp(s,"bg")==0)
        isFg= false;
    unsigned int jid=0;
    bool enterid= false;
    if(args.size() == 1){
        if(isFg)
            jid=jobs->GetMaxJobid();
        else
            jid=jobs->GetLastStoppedJobId();
    }else {
        try {
            string::size_type sz;
            jid = stoi(args[1],&sz);//or stoul
            string sub=string(args[1]).substr(sz);
            if(sub.compare("")!=0||args.size() > 2||args[1][0]=='-'){ //which kind of error "fg 0" returns?
                cerr<<"smash error: "<<s<<": invalid arguments"<<endl;
                throw FgBgException();
            }

            enterid= true;
        }
        catch(invalid_argument){
            cerr<<"smash error: "<<s<<": invalid arguments"<<endl;
            throw FgBgException();
        }
        //error massages
    }
    if(isFg&& jobs->GetMaxJobid()==0  && !enterid){
        cerr<<"smash error: "<<s<<": jobs list is empty"<<endl;
        throw FgBgException();
    }
    if(!isFg&& jobs->GetLastStoppedJobId()==0 && !enterid){
        cerr<<"smash error: "<<s<<": there is no stopped jobs to resume"<<endl;
        throw FgBgException();
    }
    JobsList::JobEntry* job=jobs->getJobById(jid);

    (*jobId)=job->GetJobId();
    (*redPipOther)=job->GetRedPipOther();

    if(*jobId== 0) {
        cerr << "smash error: "<<s<<": job-id " << jid << " does not exist" << endl;
        throw FgBgException();
    }
    if(!isFg&&(job)->GetJobState()==BgState) {

        cerr << "smash error: " << s << ": job-id " << jid << " dis already running in the background" << endl;
        throw FgBgException();
    }

    cout<<(job)->GetJobCmdLine()<<" : "<<(job)->GetJobId()<<endl; //dont know if we shuld print jobid or procid

    unsigned int jPid=(job)->GetJobPid();
    return jPid;
}

///==========================================================================================
///     FgCommand
ForegroundCommand::ForegroundCommand(const char *cmdLine, JobsList* jobs) : BuiltInCommand(cmdLine),jobsList_fgCommand(jobs) {


}


void ForegroundCommand::execute() {
    unsigned int jPid=0;
    unsigned int jid;
    RedPipOther redPipOther;

    try{
        jPid=FgBgCheck(args,jobsList_fgCommand,"fg",&jid,&redPipOther);
    }
    catch(FgBgException& e){
        return;
    }
    /*if(redPipOther==PipCmd){
        pipe_pid=jPid;
        pipe_pid_grp=getpgid(jPid);
        pid_t grpPid=pipe_pid_grp;
        if(signal(SIGTSTP , ctrlZHandlerPipe)==SIG_ERR) {
            perror("smash error: failed to set ctrl-Z handler");
        }
        if(signal(SIGINT , ctrlCHandlerPipe)==SIG_ERR) {
            perror("smash error: failed to set ctrl-C handler");
        }
        int checKill = killpg(grpPid, SIGCONT);
        int wstatus;
        if (checKill == SUCCESS) {
            front_pid = jPid;
            waitpid(jPid, &wstatus, WUNTRACED);
            if (WIFSTOPPED(wstatus)) {
                //this->jobsList_fgCommand->addJob(this,jPid, StoppedState);
                this->jobsList_fgCommand->getJobById(jid)->SetJobState(StoppedState);
                this->jobsList_fgCommand->getJobById(jid)->zeroJobStart();
            } else {
                jobsList_fgCommand->removeJobById(jid);
                jobsList_fgCommand->sortOnly();
            }
            front_pid = 0;
        }
        if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
            perror("smash error: failed to set ctrl-Z handler");
        }
        if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
            perror("smash error: failed to set ctrl-C handler");
        }
    } else {*/
        int checkKill = kill(jPid, SIGCONT);
        int wstatus;
        if (checkKill == SUCCESS) {
            front_pid = jPid;
            waitpid(jPid, &wstatus, WUNTRACED);
            if (WIFSTOPPED(wstatus)) {
                //this->jobsList_fgCommand->addJob(this,jPid, StoppedState);
                this->jobsList_fgCommand->getJobById(jid)->SetJobState(StoppedState);
                this->jobsList_fgCommand->getJobById(jid)->zeroJobStart();
            } else {
                jobsList_fgCommand->removeJobById(jid);//what happend when ctrl z send?
                jobsList_fgCommand->sortOnly();
            }
            front_pid = 0;
        } else
            perror("smash error: kill failed");
    //}




}
///==============================================================================
///   BgCommand
BackgroundCommand::BackgroundCommand(const char *cmdLine, JobsList* jobs) : BuiltInCommand(cmdLine),jobsList_bgCommand(jobs) {
}


void BackgroundCommand::execute() {
    unsigned int jPid=0;
    unsigned int jid;
    RedPipOther redPipOther;

    try{
        jPid=FgBgCheck(args,jobsList_bgCommand,"bg",&jid,&redPipOther);
    }
    catch(FgBgException& e){
        return;
    }
    jobsList_bgCommand->getJobById(jid)->SetJobState(BgState);
   /* if(redPipOther==PipCmd) {
        pipe_pid = jPid;
        pipe_pid_grp = getpgid(jPid);
        pid_t grpPid = pipe_pid_grp;
        if (signal(SIGTSTP, ctrlZHandlerPipe) == SIG_ERR) {
            perror("smash error: failed to set ctrl-Z handler");
        }
        if (signal(SIGINT, ctrlCHandlerPipe) == SIG_ERR) {
            perror("smash error: failed to set ctrl-C handler");
        }
        int checkill = killpg(grpPid, SIGCONT);
        if(checkill==FAIL)
            perror("smash error: kill failed");
        if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR) {
            perror("smash error: failed to set ctrl-Z handler");
        }
        if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
            perror("smash error: failed to set ctrl-C handler");
        }

    } else{*/
        int checkill=kill(jPid, SIGCONT);
        //should we zero the time?
        if(checkill==FAIL)
            perror("smash error: kill failed");
    //}



}
      
///==========================================================================================
///   JobsCommand
      
JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobsList_jobsCommand(jobs){
}


void JobsCommand::execute() {
    jobsList_jobsCommand->printJobsList();
}

///==========================================================================================
///   jobs list
bool is_pid_running(pid_t pid) {

    while(waitpid(-1, 0, WNOHANG) > 0) {
        // Wait for defunct....
    }

    if (kill(pid, 0)==0)
        return 1; // Process exists

    return 0;
}
JobsList::JobsList():maxJobId(0) {
}

void JobsList::removeFinishedJobs() {
    int size = jobsVector.size();
    for (int i = 0; i < size;) {
        if (jobsVector[i].GetJobState()==FinishState||!is_pid_running(jobsVector[i].GetJobPid())) {
            if (maxJobId <= jobsVector[i].GetJobId()) {
                if(size==1){
                    this->SetMaxJobid(0);
                } else{
                    this->SetMaxJobid(jobsVector[size - 2].GetJobId());
                }

            }
            //maybe delete commands?
            jobsVector.erase(jobsVector.begin()+i);
            --size;
        } else
            ++i;

    }

}
      
void JobsList::sortOnly(){
    if(jobsVector.empty())
        return;
    sort(jobsVector.begin(), jobsVector.end());
}
      
void JobsList::sortAndDelete(){
    if(jobsVector.empty())
        return;
    removeFinishedJobs(); //vector neede alaways to be sorted
    this->sortOnly();
}

unsigned int JobsList::GetMaxJobid() {
    return maxJobId;
}
      
unsigned int JobsList::GetLastStoppedJobId() {
    this->removeFinishedJobs();
    int jid=0;
    getLastStoppedJob(&jid);
    return jid;
}
      
void JobsList::SetMaxJobid(unsigned int new_maxid) {
    maxJobId=new_maxid;
}

unsigned int JobsList::GetPidByJid(unsigned int Jid) {
    this->removeFinishedJobs();
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it) {
        if (it->GetJobId() == Jid) {
            return it->GetJobPid();
        }
    }
    return 0;
}

void JobsList::printJobsList() {
    int size = jobsVector.size();
    for (int i=0; i<size; ++i) {
        cout<<jobsVector[i];
    }
}


void JobsList::killAllJobs() {
    this->removeFinishedJobs();
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it) {
        kill(it->GetJobPid(),SIGKILL);
    }
    cout<<"smash: sending SIGKILL signal to "<<jobsVector.size() <<" jobs:"<<endl;
    for (auto it = jobsVector.begin(); it != jobsVector.end(); it = jobsVector.begin()) {
        cout<<it->GetJobPid() <<": "<<it->GetJobCmdLine() <<endl;
        jobsVector.erase(it);
    }
}
      
void JobsList::addJob(Command *cmd, pid_t pid=0, State state=BgState,RedPipOther isRedPipeOther=OtherCmd,Command* realCmd= nullptr) {
    this->removeFinishedJobs();
    if(realCmd!=nullptr)
        jobsVector.push_back(JobEntry(maxJobId+1,realCmd,pid,state,isRedPipeOther));
    else
        jobsVector.push_back(JobEntry(maxJobId+1,cmd,pid,state,isRedPipeOther));
    maxJobId++;
}
      
void JobsList::removeJobById(unsigned int jobId) {
    if(jobsVector.empty())
        return;
    int size = jobsVector.size();
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it) {
        if (it->GetJobId() == jobId) {
            if (maxJobId == it->GetJobId()) {
                if(size==1){
                    this->SetMaxJobid(0);
                } else{
                    this->SetMaxJobid(jobsVector[size - 2].GetJobId());
                }

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

JobsList::JobEntry *JobsList::getLastStoppedJob(int* jobId) {
    this->removeFinishedJobs();
    if(jobsVector.empty())
        return nullptr;
    int size = jobsVector.size();
    for (int i = size-1; i >=0; --i) {
        if(jobsVector[i].GetJobState()==StoppedState) {
            *jobId = jobsVector[i].GetJobId();
            return &jobsVector[i];
        }
    }
    *jobId=0;
    return nullptr;
}


///==========================================================================================
///   job entry
      
JobsList::JobEntry::JobEntry(unsigned int jid, Command* command,pid_t pid, State state ,RedPipOther isRedPipeOtherConst=OtherCmd):jobId(jid),commandJob(command),jobPid(pid) {
    jobStart=time(nullptr);
    jobState=state;
    isRedPipeOther=isRedPipeOtherConst;

}
bool JobsList::JobEntry::operator<(const JobsList::JobEntry &jobEntry) const {
    return this->jobId<jobEntry.jobId;
}

RedPipOther JobsList::JobEntry::GetRedPipOther() {
    return isRedPipeOther;
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
    return difftime(currentTime,jobStart);///check about fault
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

JobsList::JobEntry* JobsList::getJobById(unsigned int jobId) {
    int size = jobsVector.size();
    for (int i=0; i<size; ++i) {
        if(jobsVector[i].GetJobId()==jobId)
            return &jobsVector[i];
    }
    return nullptr;
}
ostream &operator<<(ostream &os, JobsList::JobEntry &je) {
    State je_State=je.GetJobState();
    string s;
    if(je_State==StoppedState){
        s=" (stopped)\n";
    } else
        s="\n";
    os<<"["<<je.GetJobId()<<"]"<<SPACE<<je.GetJobCmdLine()<<" : "<<je.GetJobPid()<<SPACE<<je.GetJobElapsed()<<" secs"<<s;
    return os;
}

///==========================================================================================
///   External

ExternalCommand::ExternalCommand(const char *cmd_line, SmallShell& smash,RedPipOther redPipConst=OtherCmd,Command* realCmdConst= nullptr,bool isTimedOutConst=false,
                                 time_t duration=0)
:Command(cmd_line,smash,isTimedOutConst,duration) {
    //this->cmd_smash = smash; its in Command constractur
    this->isRedPipeOther=redPipConst;
    this->realCmd=realCmdConst;
    this->num_of_arg = _parseCommandLine(cmd_line, this->args);
    if (_isBackgroundComamnd(cmd_line)) this->run = Back;
    else this->run = Front;
}

void ExternalCommand::execute() {
    int status;
    char cmd[COMMAND_ARGS_MAX_LENGTH];
    strcpy(cmd, cmd_line.c_str());
    if (run == Back) _removeBackgroundSign(cmd);
    _trim(cmd); //nothing happens here
    char *_args[4] = {(char *) "/bin/bash", (char *) "-c", cmd, NULL};
    pid_t pid = fork();
    if (pid < 0) {//pid not good
        perror("smash error: fork failed");
    }
    if (pid > 0) {//father=smash
        if (isRedPipeOther == PipCmd) {
            wait(NULL);
        }
        if (this->run == Front) {
            front_pid = pid;
                if (isTimeOut) {
                    if ((isRedPipeOther != OtherCmd) || realCmd != nullptr) {
                        cmd_smash->addTimeOut(realCmd, pid, duration);
                    } else
                        cmd_smash->addTimeOut(this, pid, duration);
                    alarm(duration);
                }
                waitpid(pid, &status, WUNTRACED);
                if (WIFSTOPPED(status)) {
                    this->cmd_smash->addJob(this, pid, StoppedState, isRedPipeOther, realCmd);
                }
                front_pid = 0;
            } else {//this is smash pid backround
                this->cmd_smash->addJob(this, pid, BgState, isRedPipeOther, realCmd);
                if (isTimeOut) {
                    if ((isRedPipeOther != OtherCmd) || realCmd != nullptr) {
                        cmd_smash->addTimeOut(realCmd, pid, duration);
                    } else
                        cmd_smash->addTimeOut(this, pid, duration);
                    alarm(duration);
                }

            }
        } else {//son
            front_pid = 0;
            if (isRedPipeOther != PipCmd && !isTimeOut) //pipe dosent change pgrd
                setpgrp(); // we have to do it for the son
            execv(_args[0], _args);
            perror("smash error: execv failed");
            cmd_smash->DeleteAll();//for valgrind not reachbale
            exit(0);

        }
    }

///
///==========================================================================================
///   RedirectionCommand

RedirectionCommand::RedirectionCommand(const char *cmdLine,SmallShell& smash,bool isFirst,bool isTimeOutConst= false, time_t durationConst=0)
: Command(cmdLine,smash,isTimeOutConst,durationConst) {
    firstOption=isFirst;

}

void RedirectionCommand::execute() {
    string fullCmd_line = (string) cmd_line;
    string cmdLine1;
    string cmdLine2;
    int bufStart = 0;
    int bufEnd = 0;
    bool isBack = _isBackgroundComamnd(fullCmd_line.c_str());
    _removeBackgroundSign((char *) fullCmd_line.c_str());
    if (firstOption) {
        bufStart = fullCmd_line.find(">");
        bufEnd = bufStart + 1;
    } else {
        bufStart = fullCmd_line.find(">>");
        bufEnd = bufStart + 2;
    }
    cmdLine1 = fullCmd_line.substr(0, bufStart);
    cmdLine2 = fullCmd_line.substr(bufEnd);
    cmdLine1=_trim(cmdLine1);
    cmdLine2=_trim(cmdLine2);
    if (isBack) {
        cmdLine1.push_back(SPACE);
        cmdLine1.push_back('&');
    }
    int n1 = cmdLine1.length();
    char charCmdLine1[n1 + 1];
    int n2 = cmdLine2.length();
    char charCmdLine2[n2 + 1];
    strcpy(charCmdLine1, cmdLine1.c_str());
    strcpy(charCmdLine2, cmdLine2.c_str());

    /*
    int p = fork();
    if (p < 0)
        perror("smash error: fork failed");
    if (p == 0) {//child proc
        setpgrp();
        */
        int fd1 = dup(1);
        if (fd1 == FAIL) {
            perror("smash error: dup failed");
            //cmd_smash->DeleteAll();
           // exit(0);
        }

        if (close(1) == FAIL) {
            perror("smash error: close failed");//closes stdout
            //cmd_smash->DeleteAll();
            //exit(0);
        }
        int fd;
       // mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
       mode_t mode = 0666;
        char *filename = charCmdLine2;
        if (firstOption) {
            fd = open(filename, O_WRONLY | O_TRUNC| O_CREAT , mode);//stdout to file
            if (fd == FAIL) {
                perror("smash error: open failed");//closes stdout
               // cmd_smash->DeleteAll();
                //exit(0);
            }

        } else {
            fd = open(filename, O_WRONLY | O_APPEND| O_CREAT , mode);//stdout to file
            if (fd == FAIL) {
                perror("smash error: open failed");//closes stdout
               // cmd_smash->DeleteAll();
               // exit(0);
            }
        }

        try {
            cmd_smash->executeCommand(charCmdLine1, RedCmd, this,isTimeOut,duration);//open file no matter


        }
        catch (SmallShellException &e) {
            int checkDelete = remove(filename);//if the cmd is illigal delete file, but if it opend befor?
            if (checkDelete == FAIL) {
                close(fd);
                dup2(fd1, 1);
                perror("smash error: remove failed");//closes stdout
            }
        }
        if (close(fd) == FAIL) {
            perror("smash error: close failed");
        }
        int fd2 = dup2(fd1, 1); //stdout back to screen
        if (fd2 == FAIL) {
            perror("smash error: dup2 failed");

        }
        //cmd_smash->DeleteAll();
       // exit(0);
   /* } else {//smash proc
        if(!isBack) {
            int status;
            front_pid = p;
            waitpid(p, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                this->cmd_smash->addJob(this, p, StoppedState, RedCmd);
            }
            front_pid = 0;
        }else
            this->cmd_smash->addJob(this, p, BgState, RedCmd);


    }
    */
}


///==========================================================================================
///   RedirectionCommand

PipeCommand::PipeCommand(const char *cmdLine,SmallShell& smash,bool isFirst,bool isTimeOutConst= false, time_t durationConst=0)
: Command(cmdLine,smash,isTimeOutConst,durationConst) {
    firstOption = isFirst;
}



void PipeCommand::execute() {
    string fullCmd_line=(string)cmd_line;
    string cmdLine1;
    string cmdLine2;
    int bufStart=0;
    int bufEnd=0;
    bool isBack=_isBackgroundComamnd(fullCmd_line.c_str());
    _removeBackgroundSign((char*)fullCmd_line.c_str());
    if(firstOption){
        bufStart=fullCmd_line.find("|");
        bufEnd=bufStart+1;
    } else{
        bufStart=fullCmd_line.find("|&");
        bufEnd=bufStart+2;
    }
    cmdLine1=fullCmd_line.substr(0,bufStart);
    cmdLine2=fullCmd_line.substr(bufEnd);
    cmdLine1=_trim(cmdLine1);
    cmdLine2=_trim(cmdLine2);
    if(isBack){
        cmdLine1.push_back(SPACE);
        cmdLine1.push_back('&');
        cmdLine2.push_back(SPACE);
        cmdLine2.push_back('&');
    }
    int status;
    pid_t p0 = fork();
    if(p0<0)
        perror("smash error: fork failed");
    if(p0==0) {//pipe process
        setpgrp();
        int fd[2];
        pipe(fd);
        pid_t p1 = fork();
        if (p1 < 0) {
            perror("smash error: fork failed");
        }
        if (p1 == 0) {// first child
            //setpgrp();
            if (firstOption) {
                dup2(fd[1], 1);
            }
            if (!firstOption) {
                dup2(fd[1], 2);

            }
            close(fd[0]);
            close(fd[1]);
            cmd_smash->executeCommand(cmdLine1.c_str(), PipCmd,this,isTimeOut,duration);
            exit(0);
        } else{//pipe process
            //wait(NULL);


            pid_t p2 = fork();
            if (p2 < 0) {//pid not good
                perror("smash error: fork failed");
            }
            if (p2 == 0) {// second child
                //setpgrp();
                dup2(fd[0], 0);
                close(fd[0]);
                close(fd[1]);
                cmd_smash->executeCommand(cmdLine2.c_str(), PipCmd, this,isTimeOut,duration);
                //deleteall?
                exit(0);
            }
            else { //pipe process
                //wait(NULL);
                front_pid = p1;
                waitpid(p1, &status, WUNTRACED);///we should wait to output?
                //if(WIFSTOPPED(status)){
                // kill(SIGCONT,smash_pid);
                //  }
                front_pid = 0;
                front_pid = p2;
                waitpid(p2, &status, WUNTRACED);///we should wait to output?
                //if(WIFSTOPPED(status)){
                //kill(SIGCONT,smash_pid);
                //}
                front_pid = 0;
                close(fd[0]);
                close(fd[1]);

            }
        }
        exit(0);
    }
    else{
        //smash process
        if(!isBack) {
            front_pid=p0;
            if(isTimeOut){
                cmd_smash->addTimeOut(this,p0,duration);
                alarm(duration);
            }
            waitpid(p0, &status, WUNTRACED);
            if(WIFSTOPPED(status)){
                this->cmd_smash->addJob(this,p0,StoppedState,PipCmd);
            }
            front_pid = 0;
        }
        else {
            this->cmd_smash->addJob(this, p0, BgState, PipCmd);//pipCmd
            if(isTimeOut){
                cmd_smash->addTimeOut(this,p0,duration);
                alarm(duration);
            }
        }
    }



}



///==========================================================================================
///   cp

bool isSamePath(string a, string b){
    if (a==b)
        return true;
    string full = (a.size()>b.size())? a:b ;
    string rel = (a.size()<b.size())? a:b ;
    if (rel[0]=='/'){
        rel.erase(0);
    }
    char* _curr = get_current_dir_name();
    string curr = (string)_curr;
    free (_curr);
    if (curr[curr.size()]=='/'){
        rel.erase(curr.size());
    }
    curr.append("/").append(rel);
    if (curr == full) return true;
    return false;
}

CpCommand::CpCommand(const char* cmd_line, SmallShell& smash, Run run,RedPipOther redPipConst,Command* realCmdConst,bool isTimeOutConst,time_t durationConst)
: BuiltInCommand(cmd_line,smash), run(run),isRedPipeOther(redPipConst),realCmd(realCmdConst){
    isTimeOut=isTimeOutConst;
    duration=durationConst;
}

void CpCommand::execute() {
    pid_t pid = fork();
    int status;
    //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    mode_t mode =0666;
    if (pid < 0) {
        perror("smash error: fork failed");
    } else if (pid > 0) {
        if (isTimeOut) {
            if ((isRedPipeOther != OtherCmd) || realCmd != nullptr) {
                cmd_smash->addTimeOut(realCmd, pid, duration);
            } else
                cmd_smash->addTimeOut(this, pid, duration);
            alarm(duration);
        }
        if(isRedPipeOther==PipCmd)
            wait(NULL);
        if (this->run == Front) {
            front_pid = pid;
            waitpid(pid, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                this->cmd_smash->addJob(this, pid, StoppedState,isRedPipeOther,realCmd);
            }
            front_pid = 0;
        } else {
            this->cmd_smash->addJob(this, pid,BgState,isRedPipeOther,realCmd);
        }
    } else {
            setpgrp();

            if (this->num_of_arg != 3) {
                ///invalid num of arg
                cmd_smash->DeleteAll();
                exit(0);
            }
            src = open(args[1].c_str(), O_RDONLY);
            if (src == FAIL) {
                perror("smash error: open failed");
                cmd_smash->DeleteAll();
                exit(0);
            }
            if (!isSamePath(args[1], args[2])) {
                dst = open(args[2].c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
                if (dst == FAIL) {
                    if (close(src) == FAIL)
                        perror("smash error: close failed");
                    perror("smash error: open failed");
                    cmd_smash->DeleteAll();
                    exit(0);
                }
                char buff[30];
                int read_count = 1;
                while (read_count) {
                    read_count = read(src, (void *) buff, 30);
                    if (read_count == FAIL) {
                        if (close(src) == FAIL) {
                            perror("smash error: close failed");
                        }
                        if (close(dst) == FAIL) {
                            perror("smash error: close failed");
                        }
                        perror("smash error: read failed");
                        cmd_smash->DeleteAll();
                        exit(0);
                    }
                    if (write(dst, (void *) buff, read_count) == FAIL) {
                        if (close(src) == FAIL) {
                            perror("smash error: close failed");
                        }
                        if (close(dst) == FAIL) {
                            perror("smash error: close failed");
                        }
                        perror("smash error: read failed");
                        cmd_smash->DeleteAll();
                        exit(0);
                    }
                }
                if (close(dst) == FAIL)
                    perror("smash error: close failed");
            }
            if (close(src) == FAIL)
                    perror("smash error: close failed");
            cout << "smash: " << args[1] << " was copied to " << args[2] << endl;
            cmd_smash->DeleteAll();
            exit(0);
        }
}

///==========================================================================================
///   TimeOutCommand

TimeOutCommand::TimeOutCommand(const char *cmd_line, TimeOutList *timeOutListConst,SmallShell& smash): Command(cmd_line,smash),tOList(timeOutListConst){
    smash_glob=&smash;


}


void TimeOutCommand::execute() {
    tOList->removeFinishedTimeOut();
    time_t duration=0;
    try {
        if (args.size()<=2) { //which kind of error "fg 0" returns?
            cerr << "smash error: " << "timeout: invalid arguments" << endl;
            throw FgBgException();
        }
        string::size_type sz;
        duration = stoi(args[1], &sz);//or stoul
        string sub = string(args[1]).substr(sz);
        if(sub.compare("") != 0||duration<=0 ){
            cerr << "smash error: " << "timeout: invalid arguments" << endl;
            throw FgBgException();
        }

    }
    catch (invalid_argument) {
        cerr << "smash error: " << "timeout: invalid arguments" << endl;
        throw FgBgException();
    }
    string fullCmd_line = (string)cmd_line;
    string cmdLine1;
    int bufStart = 0;
    //bool isBack = _isBackgroundComamnd(fullCmd_line.c_str());
    //_removeBackgroundSign((char *) fullCmd_line.c_str());
    string s="timeout "+to_string(duration)+SPACE;
    bufStart = fullCmd_line.find(s);

    cmdLine1 = fullCmd_line.substr(bufStart+s.size() );
    cmdLine1=_trim(cmdLine1);
   /* if (isBack) {
        cmdLine1.push_back(SPACE);
        cmdLine1.push_back('&');
    }
    */
    int n1 = cmdLine1.length();
    char charCmdLine1[n1 + 1];
    strcpy(charCmdLine1, cmdLine1.c_str());

    cmd_smash->executeCommand(charCmdLine1,OtherCmd, nullptr,true,duration); //is TimeOut
    /*
    if(!isBack) {
        front_pid=p0;
        waitpid(p0, &status, WUNTRACED);
        if(WIFSTOPPED(status)){
            this->cmd_smash->addJob(this,p0,StoppedState,PipCmd);
        }
        front_pid = 0;
    }
    else
        this->cmd_smash->addJob(this,p0,BgState,PipCmd);//pipCmd
}*/


}

///==========================================================================================
///   timeOut List
TimeOutList::TimeOutList():maxTOId(0) {
}

void TimeOutList::addTimeOut(Command *cmd, pid_t pid, time_t duration) {
    this->removeFinishedTimeOut();

    tOVector.push_back(TimeOutEntry(maxTOId+1,cmd,pid,duration));
    maxTOId++;
}
void TimeOutList::printTimeOutList() {
    int size = tOVector.size();
    for (int i=0; i<size; ++i) {
        cout<<tOVector[i];
    }
}
void TimeOutList::removeFinishedTimeOut() {
    int size = tOVector.size();
    for (int i = 0; i < size;) {
        if (!is_pid_running(tOVector[i].GetTimeOutPid())) {
            if (maxTOId <= tOVector[i].GetTimeOutId()) {
                if(size==1){
                    this->SetMaxTOid(0);
                } else{
                    this->SetMaxTOid(tOVector[size - 2].GetTimeOutId());
                }

            }
            //maybe delete commands?
            tOVector.erase(tOVector.begin()+i);
            --size;
        } else
            ++i;

    }
}

void TimeOutList::sortOnly(){
    if(tOVector.empty())
        return;
    sort(tOVector.begin(), tOVector.end());
}

void TimeOutList::sortAndDelete(){
    if(tOVector.empty())
        return;
    removeFinishedTimeOut(); //vector neede alaways to be sorted
    this->sortOnly();
}

void TimeOutList::killAllTimeOut() {
    this->removeFinishedTimeOut();
    for (auto it = tOVector.begin(); it != tOVector.end(); ++it) {
        kill(it->GetTimeOutPid(),SIGKILL);
    }
    cout<<"smash: sending SIGKILL signal to "<<tOVector.size() <<" TimeOutCommands:"<<endl;
    for (auto it = tOVector.begin(); it != tOVector.end(); it = tOVector.begin()) {
        cout<<it->GetTimeOutId() <<": "<<it->GetTimeOutCmdLine() <<endl;
        tOVector.erase(it);
    }
}


void TimeOutList::removeTimeOutById(unsigned int ToId) {
    if(tOVector.empty())
        return;
    int size = tOVector.size();
    for (auto it =tOVector.begin(); it != tOVector.end(); ++it) {
        if (it->GetTimeOutId() == ToId) {
            if (maxTOId == it->GetTimeOutId()) {
                if(size==1){
                    this->SetMaxTOid(0);
                } else{
                    this->SetMaxTOid(tOVector[size - 2].GetTimeOutId());
                }

            }
            tOVector.erase(it);
            return;
        }
    }
}

TimeOutList::TimeOutEntry *TimeOutList::getLastTimeOut(int *lastTOId) {
    if(tOVector.empty())
        return nullptr;
    return &tOVector[tOVector.size()-1];
}

unsigned int TimeOutList::GetMaxTOid() {
    return maxTOId;
}

void TimeOutList::SetMaxTOid(unsigned int new_maxid) {
    maxTOId=new_maxid;

}


unsigned int TimeOutList::GetPidByTOid(unsigned int tOid) {
    this->removeFinishedTimeOut();
    for (auto it = tOVector.begin(); it != tOVector.end(); ++it) {
        if (it->GetTimeOutId() == tOid) {
            return it->GetTimeOutPid();
        }
    }
    return 0;
}
TimeOutList::TimeOutEntry *TimeOutList::getTimeOutById(unsigned int tOId) {
    int size = tOVector.size();
    for (int i=0; i<size; ++i) {
        if(tOVector[i].GetTimeOutId()==tOId)
            return &tOVector[i];
    }
    return nullptr;
}
unsigned int TimeOutList::GetPidFinishNow() {
    this->removeFinishedTimeOut();
    time_t currentTime;
    int secArea;
    currentTime=time(nullptr);
    for (auto it = tOVector.begin(); it != tOVector.end(); ++it) {
        secArea=difftime(currentTime,it->GetTimeOutTimeStep()+it->GetTimeOuDuration());
        if (secArea>=-1&&secArea<=1) {
            return it->GetTimeOutPid();
        }
    }
    return 0;
}
TimeOutList::TimeOutEntry*  TimeOutList::GetTOFinishNow(time_t now) {
    this->removeFinishedTimeOut();
    time_t secArea;
    time_t should_finish;
    time_t timeStep;
    time_t dur;

    for (auto it = tOVector.begin(); it != tOVector.end(); ++it) {
        timeStep=it->GetTimeOutTimeStep();
        dur=it->GetTimeOuDuration();
        should_finish=timeStep+dur;
        secArea=difftime(now,should_finish);
        if (secArea>=-2&&secArea<=2) {
            return &(*it);
        }
    }
    return nullptr;
}
void  TimeOutList::SetAlarmTOFinishNext(time_t now) {
    if(tOVector.empty())
        return;
    time_t should_finish=tOVector[0].GetTimeOutTimeStep()+tOVector[0].GetTimeOuDuration();
    time_t from_now=should_finish-now;
    if(from_now==0){
        kill(smash_pid,SIGALRM);
        return;
    }

    alarm(from_now);
}


///==========================================================================================
///   timeOut entry
TimeOutList::TimeOutEntry::TimeOutEntry(unsigned int tOidConst, Command *command, pid_t pid,time_t durationConst):tOId(tOidConst),commandTO(command),tOPid(pid) {
    tOTimeStep=time(nullptr);
    duration=durationConst;


}

bool TimeOutList::TimeOutEntry::operator<(const TimeOutList::TimeOutEntry &timeOutEntry) const {
    if(tOTimeStep+duration<timeOutEntry.tOTimeStep+timeOutEntry.duration)
        return true;
    if(tOTimeStep+duration>timeOutEntry.tOTimeStep+timeOutEntry.duration)
        return true;
    return this->tOId<timeOutEntry.tOId;
}

ostream &operator<<(ostream &os, TimeOutList::TimeOutEntry &to) {
    os<<to.GetTimeOutCmdLine()<<" Timed Out!";
    return os;
}



const unsigned int TimeOutList::TimeOutEntry::GetTimeOutId() {
    return tOId;
}

pid_t TimeOutList::TimeOutEntry::GetTimeOutPid() {
    return tOPid;
}
const time_t TimeOutList::TimeOutEntry::GetTimeOutTimeStep() {
    return tOTimeStep;
}
const time_t TimeOutList::TimeOutEntry::GetTimeOuDuration() {
    return duration;
}

double TimeOutList::TimeOutEntry::GetTimeOutElapsed() {
    time_t currentTime;
    currentTime=time(nullptr);
    return difftime(currentTime,tOTimeStep);///check about fault
}

string TimeOutList::TimeOutEntry::GetTimeOutCmdLine() {
    return commandTO->GetCmd_line();
}


void TimeOutList::TimeOutEntry::zeroTimeOutStart() {
    time_t currentTime;
    currentTime=time(nullptr);
    tOTimeStep=currentTime;
}
