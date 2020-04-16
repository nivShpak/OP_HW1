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

#include <sys/stat.h>
#include <fcntl.h>



#define FAIL -1
#define SUCCESS 0
#define SPACE " "
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
__pid_t front_pid = 0;

__pid_t getFrontPid (){
    return front_pid;
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

int _parseCommandLine(const char* cmd_line, char** args) {
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
}

int _parsrCommandLine_t(const char* cmd_line, vector<string>& args){
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
    cmd_line[idx] = 0;
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}





// TODO: Add your implementation for classes in Commands.h

///==========================================================================================
///   Smash
SmallShell::SmallShell() :prompt("smash>"),lastPwdSmash(get_current_dir_name()){
// TODO: add your implementation
jobsListSmash=new JobsList();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
    for (auto it = commandVectorSmash.begin(); it != commandVectorSmash.end(); ++it) {
        delete *it;
    }
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
    bool isFirst=true;
    string cmd_s = string(cmd_line);
    char* command_args[COMMAND_MAX_ARGS+1];
    if(_parseCommandLine(cmd_line,command_args)==0){
        throw EmptyCommandException();//empty command
    }
    else if((cmd_s.find(" > ")!=string::npos)){


            return new RedirectionCommand(cmd_line,*this,isFirst);

    }
    else if((cmd_s.find(" >> ")!=string::npos)){
        isFirst= false;
        return new RedirectionCommand(cmd_line,*this,isFirst);

    }
    char built_in_cmd_line[COMMAND_ARGS_MAX_LENGTH];
    strcpy(built_in_cmd_line,cmd_line);
    _removeBackgroundSign(built_in_cmd_line);
    _removeBackgroundSign(command_args[0]);
    char command[COMMAND_ARGS_MAX_LENGTH] ;
    strcpy(command, command_args[0]);
    _removeBackgroundSign(command);

    if((cmd_s.find(" | ")!=string::npos)){

        return new PipeCommand(cmd_line,*this,isFirst);

    }
    else if((cmd_s.find(" |& ")!=string::npos)){
        isFirst= false;
        return new PipeCommand(cmd_line,*this,isFirst);

    }

    else if (strcmp(command_args[0],"pwd")==0) {
        return new GetCurrDirCommand(built_in_cmd_line);
    }
    else if (strcmp(command_args[0],"cd")==0) {
        cmd_line=cmd_line;
        //char** platPwdp=&this->lastPwdSmash;
        return new ChangeDirCommand(built_in_cmd_line,*this);
    }
    else if (strcmp(command_args[0],"chprompt")==0)  {
        return new Chprompt(this,built_in_cmd_line);
    }
    else if (strcmp(command_args[0],"showpid")==0) {
       return new ShowPidCommand(built_in_cmd_line);
    }
    else if (strcmp(command_args[0],"jobs")==0) {
        jobsListSmash->sortAndDelete();
        return new JobsCommand(built_in_cmd_line,jobsListSmash);
    }
    else if (strcmp(command_args[0],"kill")==0){
        return new KillCommand(built_in_cmd_line, jobsListSmash);
    }
    else if (strcmp(command_args[0],"fg")==0) {
        return new ForegroundCommand(built_in_cmd_line,jobsListSmash);
    }
    else if (strcmp(command_args[0],"bg")==0) {
        return new BackgroundCommand(built_in_cmd_line,jobsListSmash);
    }
    else if (strcmp(command_args[0],"quit")==0) {
        return new QuitCommand(built_in_cmd_line,this->jobsListSmash);
    }
    else if (strcmp(command_args[0],"cp")==0){
        Run run = (_isBackgroundComamnd(cmd_line))? Back: Front;
        return new CpCommand(built_in_cmd_line,*this, run);
    }
    else {
        return new ExternalCommand(cmd_line, *this); ///its refernce for command
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    //for example:
    Command* cmd = CreateCommand(cmd_line);
    //jobsListSmash->addJob(cmd,false); //check
    if (cmd==NULL)
        throw SmallShellException(); //todo: maybe error
    cmd->execute();
    this->commandVectorSmash.push_back(cmd);//so we can delete all cmds at the end of program;
    //delete cmd;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::setPrompt(string new_prompt) {
    this->prompt=new_prompt;
}


char* SmallShell::GetLastPwd() {
    return this->lastPwdSmash;
}

void SmallShell::setLastPwd(char*& dir) {
    if(strcmp(strcpy(lastPwdSmash,dir),dir)!=0);
}

void SmallShell::addJob(Command *cmd,pid_t pid, State state = BgState) {
    this->jobsListSmash->addJob(cmd,pid, state);
}


///==========================================================================================
///   Command

Command::Command(const char* cmd_lineCons):cmd_line(cmd_lineCons) {
    this->num_of_arg = _parseCommandLine(cmd_line.c_str(),this->args);
    cmd_pid=getpid();
}

Command::Command(const char *cmd_lineCons, SmallShell &smash): cmd_line(cmd_lineCons) {
    this->num_of_arg = _parseCommandLine(cmd_lineCons,this->args);

    cmd_smash=&smash;
    cmd_pid=getpid();
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
    cout<<get_current_dir_name()<<endl;
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
}

void ChangeDirCommand::execute() {
    if(args[1]== nullptr){
        perror("smash error: chdir failed");
        return;
    }
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
            perror("smash error: chdir failed");
            return;
        }
        cmd_smash->setLastPwd(current_dir);
        return;
    }

    if(chdir(args[1])==FAIL){
        ///perror
        perror("smash error: chdir failed");
        return;
    }
    cmd_smash->setLastPwd(current_dir);
}

///==========================================================================================
///      CrpromptCommand

Chprompt::Chprompt(SmallShell* s, const char *cmd_line):BuiltInCommand(cmd_line) {
    this->cmd_smash = s;
    if (this->num_of_arg==1)
        this->prompt ="smash>";
    else {
        this->prompt = string(args[1])+=">";
    }
    ///todo: delete _args
}

Chprompt::~Chprompt(){};

void Chprompt::execute() {
        cmd_smash->setPrompt(this->prompt);
}
///==========================================================================================
///      ShowpidCommand

void ShowPidCommand::execute(){
    cout<<"smash pid is " << getpid() <<endl;
}

///==========================================================================================
///      KillCommand

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line),jl(jobs){
      if (this->num_of_arg==3) {
        if (args[1][0]=='-') {
            string id = to_string(stoi(args[2]));
            int signal = stoi(args[1] + 1);
            string sig_string = to_string(signal);
            if (id == args[2] & sig_string == (args[1] + 1) & signal >= 1 & signal <= 31) {
                this->jobID = stoi(id);
                this->signal = signal;
            }
        }
    } else this->signal=(0);
}

void KillCommand::execute() {
    if (this->signal==0){
        cout<<"smash error: kill: invalid argument"<<endl;
        return;
    }
    int pid = this->jl->GetPidByJid(this->jobID);
    if (pid==0) {
        cout<<"smash error: kill: job-id "<<this->jobID<<" does not exist"<<endl;
        return;
    }
    if (kill(pid,this->signal)!=0){
        perror("smash error: kill failed");
        cout<<"here2"<<endl;
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
        if (strcmp(args[1], "kill") == 0) {
            jl->killAllJobs();
        }
    }
    ///todo: free all allocate not include smash which be free at the end of main.
    throw "killme"; ///todo: throw something more logical and destroy this command.
    }
        
///==========================================================================================
///      FgBgCheck
  
unsigned int FgBgCheck(char** args, JobsList* jobs, const char *s, unsigned int* jobId){
    jobs->removeFinishedJobs();
    bool isFg=true;
    if(strcmp(s,"bg")==0)
        isFg= false;
    unsigned int jid=0;
    bool enterid= false;
    if(args[1]== nullptr){
        if(isFg)
            jid=jobs->GetMaxJobid();
        else
            jid=jobs->GetLastStoppedJobId();
    }else {
        try {
            string::size_type sz;
            jid = stoi(args[1],&sz);//or stoul
            string sub=string(args[1]).substr(sz);
            if(sub.compare("")!=0||args[2]!= nullptr||args[1][0]=='-'){ //which kind of error "fg 0" returns?
                cout<<"smash error: "<<s<<": invalid arguments"<<endl;
                throw FgBgException();
            }

            enterid= true;
        }
        catch(invalid_argument){
            cout<<"smash error: "<<s<<": invalid arguments"<<endl;
            throw FgBgException();
        }
        //error massages
    }
    if(isFg&& jobs->GetMaxJobid()==0  && !enterid){
        cout<<"smash error: "<<s<<": jobs list is empty"<<endl;
        throw FgBgException();
    }
    if(!isFg&& jobs->GetLastStoppedJobId()==0 && !enterid){
        cout<<"smash error: "<<s<<": there is no stopped jobs to resume"<<endl;
        throw FgBgException();
    }
    JobsList::JobEntry* job=jobs->getJobById(jid);

    (*jobId)=job->GetJobId();

    if(*jobId== 0) {
        cout << "smash error: "<<s<<": job-id " << jid << " does not exist" << endl;
        throw FgBgException();
    }
    if(!isFg&&(job)->GetJobState()==BgState) {

        cout << "smash error: " << s << ": job-id " << jid << " dis already running in the background" << endl;
        throw FgBgException();
    }

    cout<<(job)->GetJobCmdLine()<<" : "<<(job)->GetJobId()<<endl;

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
    JobsList::JobEntry** job= nullptr;
    try{
        jPid=FgBgCheck(args,jobsList_fgCommand,"fg",&jid);
    }
    catch(FgBgException& e){
        return;
    }
    int checkKill=kill(jPid, SIGCONT);
    int wstatus;
    if(checkKill==SUCCESS){
        front_pid = jPid;
        waitpid(jPid,&wstatus,WUNTRACED);
        if(WIFSTOPPED(wstatus)){
            this->cmd_smash->addJob(this,jPid, StoppedState);
        }
        front_pid = 0;
    }
    jobsList_fgCommand->removeJobById(jid);//what happend when ctrl z send?
    jobsList_fgCommand->sortOnly();




}
///==============================================================================
///   BgCommand
BackgroundCommand::BackgroundCommand(const char *cmdLine, JobsList* jobs) : BuiltInCommand(cmdLine),jobsList_bgCommand(jobs) {
}


void BackgroundCommand::execute() {
    unsigned int jPid=0;
    unsigned int jid;
    JobsList::JobEntry* job;
    try{
        jPid=FgBgCheck(args,jobsList_bgCommand,"bg",&jid);
    }
    catch(FgBgException& e){
        return;
    }
    jobsList_bgCommand->getJobById(jid)->SetJobState(BgState);
    int checkKill=kill(jPid, SIGCONT);
    //if(checkKill==SUCCESS)
    //maybe we want wstatus?
    //jobsList_bgCommand->removeJobById(jid);

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

    if (0 == kill(pid, 0))
        return 1; // Process exists

    return 0;
}
JobsList::JobsList():maxJobId(0) {
}

void JobsList::removeFinishedJobs() {
    //bool isPidRuning;
    int size = jobsVector.size();
    for (int i = 0; i < size; ++i) {
        //isPidRuning=is_pid_running(jobsVector[i].GetJobPid());
        if (jobsVector[i].GetJobState()==FinishState||!is_pid_running(jobsVector[i].GetJobPid())) {
            if (maxJobId <= jobsVector[i].GetJobId()) {
                if(size==1){
                    this->SetMaxJobid(0);
                } else{
                    this->SetMaxJobid(jobsVector[size - 2].GetJobId());
                }

            }
            jobsVector.erase(jobsVector.begin()+i);
            size--;
        }

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
    return 0;
}

void JobsList::printJobsList() {
    int size = jobsVector.size();
    for (int i=0; i<size; ++i) {
        cout<<jobsVector[i];
    }
}


void JobsList::killAllJobs() {
    for (auto it = jobsVector.begin(); it != jobsVector.end(); ++it)
        jobsVector.erase(it);
}
      
void JobsList::addJob(Command *cmd, pid_t pid, State state) {
    jobsVector.push_back(JobEntry(maxJobId+1,cmd,pid,state));
    maxJobId++;
}
      
void JobsList::removeJobById(int jobId) {
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

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    if(jobsVector.empty())
        return nullptr;
    int size = jobsVector.size();
    for (int i = size-1; i >=0; --i) {
        if(jobsVector[i].GetJobState()==StoppedState)
            *jobId=jobsVector[i].GetJobId();
            return &jobsVector[i];
    }
}


///==========================================================================================
///   job entry
      
JobsList::JobEntry::JobEntry(unsigned int jid, Command* command,pid_t pid, State state):jobId(jid),commandJob(command) {
    jobStart=time(nullptr);
    jobPid=pid;
    jobState=state;

}
bool JobsList::JobEntry::operator<(const JobsList::JobEntry &jobEntry) const {
    return this->jobId<jobEntry.jobId;
}


unsigned int JobsList::JobEntry::GetJobId() {
    if(this== nullptr)
        return 0;
    return jobId;
}

unsigned int JobsList::JobEntry::GetJobPid() {
    if(this== nullptr)
        return 0;
    return jobPid;
}

double JobsList::JobEntry::GetJobElapsed() {
    if(this== nullptr)
        return FAIL;
    time_t currentTime;
    currentTime=time(nullptr);
    return difftime(currentTime,jobStart);///check about fault
}
      
State JobsList::JobEntry::GetJobState() {
    return jobState;
}
      
void JobsList::JobEntry::zeroJobStart() {
    if(this== nullptr)
        return;
    time_t currentTime;
    currentTime=time(nullptr);
    jobStart=currentTime;

}

void JobsList::JobEntry::SetJobState(State newState) {
    jobState=newState;

}
      
string JobsList::JobEntry::GetJobCmdLine() {
    if(this== nullptr)
        return nullptr;
    return this->commandJob->GetCmd_line();
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    if(this== nullptr)
        return nullptr;
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
        s=" (stooped)\n";
    } else
        s="\n";
    os<<"["<<je.GetJobId()<<"]"<<SPACE<<je.GetJobCmdLine()<<" : "<<je.GetJobPid()<<SPACE<<je.GetJobElapsed()<<" secs"<<s;
    return os;
}

///==========================================================================================
///   External

ExternalCommand::ExternalCommand(const char *cmd_line, SmallShell& smash) :Command(cmd_line,smash) {
    //this->cmd_smash = smash; its in Command constractur
    this->num_of_arg = _parseCommandLine(cmd_line, this->args);
    if (_isBackgroundComamnd(cmd_line)) this->run = Back;
    else this->run = Front;
}

void ExternalCommand::execute() {
    int status;
    char cmd[COMMAND_ARGS_MAX_LENGTH];
    strcpy(cmd, cmd_line.c_str());
    if (run==Back) _removeBackgroundSign(cmd);
    _trim(cmd);
    char* _args[4] = {(char*)"/bin/bash", (char*)"-c", cmd, NULL};
    pid_t pid = fork();
    if (pid<0){//pid not good
        perror("smash error: fork failed");
    }
    if (pid>0) {//father=smash
        if (this->run == Front){
            front_pid = pid;
            waitpid(pid,&status,WUNTRACED);
            if(WIFSTOPPED(status)){
                this->cmd_smash->addJob(this,pid,StoppedState);
            }
            front_pid = 0;
        }else{//this is smash pid
            this->cmd_smash->addJob(this,pid);
        }
    }
    else {//son
        setpgrp(); // we have to do it for the son
        int flag = execv(_args[0],_args);



        //todo: add checks
        if (flag== -1)
                perror("smash error: execv failed");
        //exit(0);//should we exit after we finish the proc?

    }
}
///
///==========================================================================================
///   RedirectionCommand

RedirectionCommand::RedirectionCommand(const char *cmdLine,SmallShell& smash,bool isFirst) : Command(cmdLine,smash) {
    firstOption=isFirst;

}

void RedirectionCommand::execute() {
    string fullCmd_line=(string)cmd_line;
    string cmdLine1;
    string cmdLine2;
    int bufStart=0;
    int bufEnd=0;
    if(firstOption){
        bufStart=fullCmd_line.find(" > ");
        bufEnd=bufStart+3;
    } else{
        bufStart=fullCmd_line.find(" >> ");
        bufEnd=bufStart+4;
    }

    cmdLine1=fullCmd_line.substr(0,bufStart+1);
    cmdLine2=fullCmd_line.substr(bufEnd);
    //pid_t p = fork();

    if(_isBackgroundComamnd(cmd_line.c_str())){
        cmdLine1.push_back('&');
    }
    int n1 = cmdLine1.length();
    // declaring character array
    char charCmdLine1[n1 + 1];
    int n2 = cmdLine2.length();
    // declaring character array
    char charCmdLine2[n2 + 1];
    strcpy(charCmdLine1, cmdLine1.c_str());
    strcpy(charCmdLine2, cmdLine2.c_str());


    //if (p == 0) { //the son do the rediraction
        //setpgrp()
    int fd1=dup(1);
    if(fd1==FAIL){
        //exeption
    }

    close(1); //closes stdout

    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *filename =charCmdLine2;
    if(firstOption){
        fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, mode);

    } else{
        fd = open(filename, O_WRONLY|O_CREAT|O_APPEND, mode);
    }

    try{
        cmd_smash->executeCommand(charCmdLine1);//open file no matter

    }
    catch(SmallShellException& e){
        int checkDelete=remove(filename);//if the cmd is illigal delete file, but if it opend befor?
    }

    close(fd);
    int fd2=dup2(fd1,1); //stdout back to screen
    if(fd2==FAIL){
        //exeption
    }
        //exit(0);


    /*}

     if (p<0){
         perror("smash error: fork failed");
     }else{
         wait(NULL);
     }
     */

}
///==========================================================================================
///   RedirectionCommand

PipeCommand::PipeCommand(const char *cmdLine,SmallShell& smash,bool isFirst) : Command(cmdLine,smash) {
    firstOption = isFirst;
}



void PipeCommand::execute() {
    string fullCmd_line=(string)cmd_line;
    string cmdLine1;
    string cmdLine2;
    int bufStart=0;
    int bufEnd=0;
    if(firstOption){
        bufStart=fullCmd_line.find(" | ");
        bufEnd=bufStart+3;
    } else{
        bufStart=fullCmd_line.find(" |& ");
        bufEnd=bufStart+4;
    }

    cmdLine1=fullCmd_line.substr(0,bufStart+1);
    cmdLine2=fullCmd_line.substr(bufEnd);
    //pid_t p = fork();

    if(_isBackgroundComamnd(cmd_line.c_str())){
        cmdLine1.push_back('&');
        cmdLine2.push_back('&');
    }

    /*
    int n1 = cmdLine1.length();
    // declaring character array
    char charCmdLine1[n1 + 1];
    int n2 = cmdLine2.length();
    // declaring character array
    char charCmdLine2[n2 + 1];
    strcpy(charCmdLine1, cmdLine1.c_str());
    strcpy(charCmdLine2, cmdLine2.c_str());
    */

    int fd[2];
    pipe(fd);
    pid_t p1 = fork();
    if (p1 == 0) {
        // first child
        setpgrp();
        if(strcmp(cmdLine1.c_str(),"|")==0){
            dup2(fd[1],1);
        }
        if(strcmp(cmdLine2.c_str(),"|&")==0){
            dup2(fd[1],2);
        }
        close(fd[0]);
        close(fd[1]);
        cmd_smash->executeCommand(cmdLine1.c_str());
        exit(0);
    }
    pid_t p2 = fork();
    if (p2 == 0) {
        // second child
        setpgrp();
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
        cmd_smash->executeCommand(cmdLine2.c_str());
        exit(0);
    }
    close(fd[0]);
    close(fd[1]);




}



///==========================================================================================
///   cp


CpCommand::CpCommand(const char* cmd_line, SmallShell& smash, Run run) : BuiltInCommand(cmd_line,smash), run(run){
}

void CpCommand::execute() {
    pid_t pid = fork();
    int status;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if (pid<0){
        perror("smash error: fork failed");
    }
    else if (pid>0) {
        if (this->run == Front){
            front_pid = pid;
            waitpid(pid,&status,WUNTRACED);
            if(WIFSTOPPED(status)){
                this->cmd_smash->addJob(this,pid,StoppedState);
            }
            front_pid = 0;
        }
        else{
            this->cmd_smash->addJob(this,pid);
        }
    }
    else {
        if(this->num_of_arg!=3){
            ///invalid num of arg
            exit(0);
        }
        src = open(args[1],O_RDONLY);
        if (src== -1)
            perror("smash error: open failed");
        dst = open(args[2],O_WRONLY | O_CREAT |O_TRUNC, mode);
        if (dst== -1)
            perror("smash error: open failed");
        char buff [10];
        int read_count =1;
        while (read_count){
            read_count = read(src,(void*)buff,10);
            if (read_count == -1){
                perror("smash error: read failed");
            }
            if (write(dst,(void*)buff, read_count)==-1){
                perror("smash error: write failed");
            }
        }
        if (close(dst)== -1)
            perror("smash error: close failed");
        if (close(src)== -1)
            perror("smash error: close failed");
        cout<<"smash: " << args[1]<<" was copied to "<< args[2]<<endl;
        exit(0);
    }
};
