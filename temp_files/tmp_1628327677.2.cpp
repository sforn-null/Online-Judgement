#include <iostream>
#include <string>
#include <vector>

class Solution {
  public:
    bool isPalindrome(int x) {
  return true;
    }

};

//tail.cpp不会让用户看到
//但是编译的时候会和用户提交的合并到一起编译

void Test1(){
  Solution s;
  bool ret = s.isPalindrome(121);
  if(ret){
    std::cout << "Test1 OK" << std::endl;
  }
  else{
    std::cout << "Test1 failed" << std::endl;
  }
}

void Test1(){
  Solution s;
  bool ret = s.isPalindrome(-121);
  if(!ret){
    std::cout << "Test1 OK" << std::endl;
  }
  else{
    std::cout << "Test1 failed" << std::endl;
  }
}

int main()
{
    
    Test1();
    return 0;
}
