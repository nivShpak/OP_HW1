#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include "time.h"
#include <algorithm>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

enum State
{
    FinishState,
    BgState,
    StoppedState
};

enum Run{
    Front,
    Back
};
enum RedPipOther
{
    RedCmd,
    PipCmd,
    OtherCmd
};

using namespace std;


int _parsrCommandLine_t(const char* cmd_line, vector<string>& args);

class SmallShell;
class Command;
class JobsList;
class JobsList;
class EmptyCommandException : public exception {};
__pid_t getFrontPid ();
__pid_t getPipePid ();
__pid_t getSmashPid ();
__pid_t getPipePidGrp ();

///==========================================================================================
///   Command
class Command {
// TODO: Add your data members
protected:
    string cmd_line;
    int num_of_arg;
    vector<string> args;
    SmallShell* cmd_smash;
    unsigned int cmd_pid=0;
public:
    Command(const char* cmd_line);
    Command(const char* cmd_line,SmallShell& smash);
    virtual ~Command();
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
    string GetCmd_line();

    unsigned int GetCmd_pid();
};
///==========================================================================================
///   BuiltInCommand
class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmdLine);
    BuiltInCommand(const char* cmd_line,SmallShell& smash);
    virtual ~BuiltInCommand() {};
};
///==========================================================================================
///   ExternalCommand
class ExternalCommand : public Command {
    Run run = Front;
    RedPipOther isRedPipeOther;
    Command* realCmd;
public:
    ExternalCommand(const char* cmd_line,SmallShell& smash,RedPipOther isRedPipe,Command* realCmd);
    virtual ~ExternalCommand() {}
    void execute() override;
};
///==========================================================================================
///   PipeCommand
class PipeCommand : public Command {
    // TODO: Add your data members
    bool firstOption;//first | second |&
public:
    PipeCommand(const char* cmd_line);
    explicit PipeCommand(const char *cmdLine,SmallShell& smash, bool isFirst);
    virtual ~PipeCommand() {}
    void execute() override;
};
///==========================================================================================
///   RedirectionCommand
class RedirectionCommand : public Command {
    // TODO: Add your data members
    bool firstOption;
public:
    explicit RedirectionCommand(const char *cmdLine);
    explicit RedirectionCommand(const char *cmdLine,SmallShell& smash,bool isFirst);

    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};
///==========================================================================================
///   Chprompt
class Chprompt : public BuiltInCommand {
    string prompt;
public:
    Chprompt( SmallShell* smash, const char* new_prompt = "smash>");
    virtual ~Chprompt()= default;
    void execute() override;

};

///==========================================================================================
///   Cd
class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
    char* last_pwd;
public:
    ChangeDirCommand(const char* cmd_line, SmallShell& smash);
    ChangeDirCommand(const char* cmd_line, char** plastPwd);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};
///==========================================================================================
///   Pwd
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmdLine);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};
///==========================================================================================
///   ShowPid
class ShowPidCommand : public BuiltInCommand {
public:
  ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {};
  void execute() override;
};



///==========================================================================================
///   Quit
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    JobsList* jl;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() {}
    void execute() override;
};

///==========================================================================================
///   CP

class CpCommand : public BuiltInCommand {
// TODO: Add your data members public:
    int src;
    int dst;
    Run run;
    RedPipOther isRedPipeOther;
    Command* realCmd;
public:
    CpCommand(const char* cmd_line, SmallShell& smash, Run run,RedPipOther redPipConst= OtherCmd,Command* realCmdConst= nullptr);
    virtual ~CpCommand() {};
    void execute() override;
};

///==========================================================================================
///   History not relevant
class CommandsHistory {
protected:
    class CommandHistoryEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
public:
    CommandsHistory();
    ~CommandsHistory() {}
    void addRecord(const char* cmd_line);
    void printHistory();
};

class HistoryCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    HistoryCommand(const char* cmd_line, CommandsHistory* history);
    virtual ~HistoryCommand() {}
    void execute() override;
};

///==========================================================================================
///   JobsList
class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
        unsigned int jobId;
        Command* commandJob;
        time_t jobStart;
        unsigned int jobPid;
        State jobState;
        RedPipOther isRedPipeOther;
    public:
        //JobEntry &operator==(const JobEntry &jobEntry);
        //JobEntry &operator!=(const JobEntry &jobEntry)= default;
        //JobEntry &operator++();
        JobEntry(unsigned int jid,Command* command,pid_t pid, State state,RedPipOther isRedPipeOther);
        JobEntry(const JobEntry& jobEntry)= default;
        bool operator<(const JobEntry &jobEntry) const;
        friend ostream& operator<<( ostream& os,JobEntry& je);
        RedPipOther GetRedPipOther();
        unsigned int GetJobId();
        unsigned int GetJobPid();
        double GetJobElapsed();
        State  GetJobState();
        string  GetJobCmdLine();
        void  zeroJobStart();
        void  SetJobState(State newState);
    };
    // TODO: Add your data members
    vector<JobEntry> jobsVector;
    unsigned int maxJobId;
public:
    JobsList();
    JobsList(const JobsList& jobsList)= delete;
    void operator=(const JobsList& jobsList)= delete;
    ~JobsList()= default;
    void addJob(Command* cmd,pid_t pid=0,State state=BgState,RedPipOther isRedPipeOther=OtherCmd,Command* realCmd=nullptr);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(unsigned int jobId);
    void removeJobById(unsigned int jobId);
    JobEntry * getLastJob(int* lastJobId);
    unsigned int GetLastStoppedJobId();
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
    void sortAndDelete();
    void sortOnly();
    unsigned int GetPidByJid(unsigned int Jid);
    unsigned int GetMaxJobid();
    void SetMaxJobid(unsigned int new_maxid);

};
///==========================================================================================
///   JobsCommand
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobsList_jobsCommand;
public:
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};
///==========================================================================================
///   Kill
class KillCommand : public BuiltInCommand {
    KillCommand(const char *cmd_line, JobsList& jobs);
    JobsList* jl;
    int jobID;
    int signal;
public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() {}
    void execute() override;
};
///==========================================================================================
///   Fg
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobsList_fgCommand;
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};
///==========================================================================================
///   Bg
class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobsList_bgCommand;
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~BackgroundCommand() {}
    void execute() override;
};
///==========================================================================================
///   FgBgExeption
class FgBgException : public exception {
    const char * what () const throw () {
        return "FgBgExeption";
    }
};

// TODO: should it really inhirit from BuiltInCommand ?
class CopyCommand : public BuiltInCommand {
public:
    CopyCommand(const char* cmd_line);
    virtual ~CopyCommand() {}
    void execute() override;
};
///==========================================================================================
///   SmallShell

class SmallShell {
private:
    // TODO: Add your data members
    string prompt;
    string lastPwdSmash;
    JobsList* jobsListSmash;
    vector<Command*> commandVectorSmash;
    SmallShell();
public:
    const string getPrompt()const;
    void setPrompt(string new_prompt);
    Command *CreateCommand(const char* cmd_line,RedPipOther redPipOther,Command* realCmd);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void DeleteAll();
    void executeCommand(const char* cmd_line, RedPipOther redPipOther=OtherCmd,Command* realCmd= nullptr);
    // TODO: add extra methods as needed
    string GetLastPwd();
    void setLastPwd( string dir);
    void addJob(Command* cmd,pid_t pid, State state,RedPipOther redPipOther=OtherCmd,Command* realCmd= nullptr);
};

///==========================================================================================
///   SmallShellExeption
class SmallShellException : public exception {
    const char * what () const throw () {
        return "SmallShellExeption";
    }
};
///==========================================================================================
///
#endif //SMASH_COMMAND_H_
