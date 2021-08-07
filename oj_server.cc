#include <string>
#include <vector>
#include "cpphttplib/httplib.h"
#include <jsoncpp/json/json.h>
#include "compile.hpp"
#include "util.hpp"
#include "oj_model.hpp"
#include "oj_view.hpp"

// controller 作为服务器核心逻辑
// 创建服务器框架，组织逻辑
int main()
{
    OjModel model;
    model.Load();

    using namespace httplib;
    Server server;
    server.Get("/all_questions", [&model](const Request& req,
          Response& resp){
          std::vector<Question> all_questions;
          model.GetAllQuestions(&all_questions);
          std::string html;
          OjView::RenderAllQuestions(all_questions, &html);
          resp.set_content(html, "text/html");
        });
    server.Get(R"(/question/(\d+))", [&model](const Request& req,
          Response& resp){
          Question question;
          model.GetQuestion(req.matches[1].str(), &question);
          std::string html;
          OjView::RenderQuestion(question, &html);
          resp.set_content(html, "text/html");

        });
    server.Post(R"(/compile/(\d+))", [&model](const Request& req,
          Response& resp){
          //根据id获取到题目信息
          Question question;
          model.GetQuestion(req.matches[1].str(), &question);
          
          //解析body，获取到用户提交的代码
          std::unordered_map<std::string, std::string> body_kv;
          UrlUtil::ParseBody(req.body, &body_kv);
          const std::string& user_code = body_kv["code"];

          //构造json结构的参数
          Json::Value req_json;
          req_json["code"] = user_code + question.tail_cpp;
          req_json["stdin"] = user_code;
          Json::Value resp_json;
          //调用编译模块进行编译
          Compiler::CompileAndRun(req_json, &resp_json);
          //根据编译结果构造网页
          std::string html;
          OjView::RenderResult(resp_json["stdout"].asString(),
              resp_json["reason"].asString(), &html);
          resp.set_content(html, "text/html");
        });
    server.set_mount_point("/", "./wwwshy");
    server.listen("0.0.0.0", 8081);
    return 0;
}
