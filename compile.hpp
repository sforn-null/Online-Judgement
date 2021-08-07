#pragma once
#include <string>
#include <atomic>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jsoncpp/json/json.h>
#include "util.hpp"

class Compiler{
public:
  //源代码文件, name表示当前请求的名字
  static std::string SrcPath(const std::string& name){
      return "./temp_files/" + name + ".cpp";
  }
  //编译错误文件
  static std::string CompileErrorPath(const std::string& name){
      return "./temp_files/" + name + ".Compile_error";
  }
  //可执行程序文件
  static std::string ExePath(const std::string& name){
      return "./temp_files/" + name + ".exe";
  }
  //标准输入文件
  static std::string StdinPath(const std::string& name){
      return "./temp_files/" + name + ".stdin";
  }
  //标准输出文件
  static std::string StdoutPath(const std::string& name){
      return "./temp_files/" + name + ".stdout";
  }
  //标准错误文件
  static std::string StderrPath(const std::string& name){
      return "./temp_files/" + name + ".stderr";
  }

  static bool CompileAndRun(const Json::Value& req,
      Json::Value* resp){
      //根据请求对象生成源代码文件
    if (req["code"].empty()){
        (*resp)["error"] = 3;
        (*resp)["reason"] = "code empty";
        LOG(ERROR) << "code empty" <<std::endl; 
        return false;
    }
    const std::string& code = req["code"].asString();
    const std::string& file_name = WriteTmpFile(code, req["stdin"].asString());

    //调用g++进行编译
    bool ret = Compile(file_name);
    if (!ret){
        //错误处理
        (*resp)["error"] = 1;
        std::string reason;
        FileUtil::Read(CompileErrorPath(file_name), &reason);
        (*resp)["reason"] = reason;
        //用户出现问题，所以日志级别为INFO
        LOG(INFO) << "Compile failed!" <<std::endl;
        return false;
    }

    //调用可执行程序
    int sig = Run(file_name);
    if(sig != 0){
        //错误处理
        (*resp)["error"] = 2;
        std::string reason;
        FileUtil::Read(StderrPath(file_name), &reason);
        (*resp)["reason"] = reason;
        LOG(INFO) << "Program exit by signo: " << std::to_string(sig) << std::endl;
        return false;
    }
    //把最终结果进行返回
    (*resp)["error"] = 0;
    (*resp)["reason"] = "";
    std::string str_stdout;
    FileUtil::Read(StdoutPath(file_name), &str_stdout);
    (*resp)["stdout"] = str_stdout;
    std::string str_stderr;
    FileUtil::Read(StderrPath(file_name), &str_stderr);
    (*resp)["stderr"] = str_stderr;
    LOG(INFO) << "Program " << file_name << " Done" << std::endl;
    return true;
  }

private:
    //把代码写到文件中，并且分配一个唯一的名字
    static std::string WriteTmpFile(const std::string& code,
        const std::string& str_stdin){
        static std::atomic_int id(0);
        ++id;
        std::string file_name = "tmp_" + std::to_string(TimeUtil::TimeStamp())
        + "." + std::to_string(id);
        FileUtil::Write(SrcPath(file_name), code);

        FileUtil::Write(StdinPath(file_name), str_stdin);
        return file_name;
    }

    static bool Compile(const std::string& file_name){
        // 构造出编译指令
        char* command[20] = {0};
        char buf[20][50] = {{0}};
        for(int i = 0; i < 20; ++i){
            command[i] = buf[i];
        }
        sprintf(command[0], "%s", "g++");
        sprintf(command[1], "%s", SrcPath(file_name).c_str());
        sprintf(command[2], "%s", "-o");
        sprintf(command[3], "%s", ExePath(file_name).c_str());
        sprintf(command[4], "%s", "-std=c++11");
        command[5] = NULL;
        //创建子进程
        int ret = fork();
        if(ret > 0){
            //父进程进行进程等待
            waitpid(ret, NULL, 0);;
        }
        else{
            //子进程进行程序替换
            int fd = open(CompileErrorPath(file_name).c_str(),
                O_WRONLY | O_CREAT, 0666);
            if (fd < 0){
                LOG(ERROR) << "open Compile file error" << std::endl;
                exit(1);
            }
            dup2(fd, 2);
            execvp(command[0], command);
            //如果子进程执行失败，就直接退出
            exit(0);
        }
        //判定可执行文件是否存在来确定编译是否成功
        struct stat st;
        ret = stat(ExePath(file_name).c_str(), &st);
        if(ret < 0){
            //说明文件不存在
            LOG(INFO) << "Compile failed!" << file_name <<  std::endl;
            return false;
        }
        LOG(INFO) << "Compile " << file_name << " OK!" << std::endl;
        return true;
    }

    static int Run(const std::string& file_name){
        //创建子进程
        int ret = fork();
        if(ret > 0){
            //父进程进行等待
            int status = 0;
            waitpid(ret, &status, 0);
            return status & 0x7f;
        }
        else{
            //进行标准输入，输出，错误的重定向
            int fd_stdout = open(StdoutPath(file_name).c_str(),
                O_WRONLY | O_CREAT, 0666);
            dup2(fd_stdout, 1);
            int fd_stderr = open(StderrPath(file_name).c_str(),
                O_WRONLY | O_CREAT, 0666);
            dup2(fd_stderr, 2);
            
            //子进程进行程序替换
            execl(ExePath(file_name).c_str(),
                ExePath(file_name).c_str(), NULL);
            exit(0);
        }
        //子进程进行程序替换
    }
};
