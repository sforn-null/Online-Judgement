#pragma once
#include <vector>
#include <string>
#include <ctemplate/template.h>
#include "oj_model.hpp"

class OjView{
public:
    //根据数据，生成html，通常叫渲染（render）
    static void RenderAllQuestions(const std::vector<Question>& all_questions,
        std::string* html) {
      //将所有的题目数据转换成题目列表页html
      //通过网页模板的方式来构造html
      //创建一个总的ctemplate对象
      ctemplate::TemplateDictionary dict("all_question");
      for(const auto& question : all_questions){
          //循环往这个对象中田间一些子对象
          ctemplate::TemplateDictionary* table_dict
            = dict.AddSectionDictionary("question");
          //每个子对象再设置一些键值对和模板中的{{}}对应
          table_dict->SetValue("id", question.id);
          table_dict->SetValue("name", question.name);
          table_dict->SetValue("diff", question.diff);
      }

      //进行数据的替换，生成最终的html
      ctemplate::Template* tpl;
      tpl = ctemplate::Template::GetTemplate(
          "./template/all_questions.html",
          ctemplate::DO_NOT_STRIP);
      tpl->Expand(html, &dict);
    }

    static void RenderQuestion(const Question& question,
        std::string* html){
      ctemplate::TemplateDictionary dict("question");
      dict.SetValue("id", question.id);
      dict.SetValue("name", question.name);
      dict.SetValue("diff", question.diff);
      dict.SetValue("desc", question.desc);
      dict.SetValue("header", question.header_cpp);
      ctemplate::Template* tpl;
      tpl = ctemplate::Template::GetTemplate(
          "./template/question.html",
          ctemplate::DO_NOT_STRIP);
      tpl->Expand(html, &dict);
    }

    static void RenderResult(const std::string& str_stdout,
        const std::string& reason, std::string* html){
      ctemplate::TemplateDictionary dict("result");
      dict.SetValue("stdout", str_stdout);
      dict.SetValue("reason", reason);

      ctemplate::Template* tpl;
      tpl = ctemplate::Template::GetTemplate(
          "./template/result.html",
          ctemplate::DO_NOT_STRIP);
      tpl->Expand(html, &dict);
    }
};

