#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include "util.hpp"
//把文件中存贮的题目信息加载起来
//供服务器随时使用

struct Question{
    std::string id;
    std::string name;
    std::string dir; //题目对应的目录,目录包含了题目描述
                     //题目的代码框架/题目的测试用例
    std::string diff; //难度
    std::string desc; //题目的描述
    std::string header_cpp; //题目的代码框架中的代码
    std::string tail_cpp; //题目的测试用例代码
};

class OjModel{
public:
  //把文件上的数据加载到内存中
  bool Load(){
      //打开oj_config.cfg文件，按行读取，解析
      std::ifstream file("./oj_data/oj_config.cfg");
      if(!file.is_open()){
          return false;
      }
      std::string line;
      while(std::getline(file, line)){
          //根据解析结果填入结构体
          std::vector<std::string> tokens;
          UrlUtil::Split(line, "\t", &tokens);
          if(tokens.size() != 4){
              LOG(ERROR) <<  "config file format error!\n";
              continue;
          }
          Question q;
          q.id = tokens[0];
          q.name = tokens[1];
          q.diff = tokens[2];
          q.dir = tokens[3];
          FileUtil::Read(q.dir + "/desc.txt", &q.desc);
          FileUtil::Read(q.dir + "/header.cpp", &q.header_cpp);
          FileUtil::Read(q.dir + "/tail.cpp", &q.tail_cpp);
          //插入到hash表
          _model[q.id] = q;
      }
      file.close();
      LOG(INFO) << "Load" << _model.size() << " questions\n";
      return true;
  }
  
  bool GetAllQuestions(std::vector<Question>* questions) const{
      //遍历hash表
      questions->clear();
      for(const auto& kv : _model){
          questions->push_back(kv.second); 
      }
      return true;
  }

bool GetQuestion(const std::string& id, Question* q) const{
      const auto pos = _model.find(id);
      if(pos == _model.end()){
          return false;
      }
      *q = pos->second;
      return true;
  }

private:
  std::map<std::string, Question> _model;

};
