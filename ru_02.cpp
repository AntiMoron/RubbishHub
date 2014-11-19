/**
* author : AntiMoron
* date   : 2014-11-19
* topic  : Base64 Encoding.
*/

#include<iostream>
#include<string>
#include<memory>
   
struct basic_table_wrapper{
     basic_table_wrapper(){
         for(int i=0;i<26;i++){
             table[i] = 65 + i;
             }
        for(int i=26;i<52;i++){
            table[i] = 97 + i - 26;
            }
        for(int i=52;i<62;i++){
            table[i] = 48 + i - 52;
            }
        table[62] = '+';
        table[63] = '/';
        }
    char table[64];
};
#define FIRST(x)  ((x & 0xfc0000)>>18)
#define SECOND(x) ((x &  0x3f000)>>12)
#define THIRD(x)  ((x &    0xfc0)>> 6)
#define FORTH(x)  ((x &     0x3f))


const static basic_table_wrapper table;
std::string convert(const std::string& src){
    std::string result;
    unsigned long long hash = 0;
    for(int i=0;i<3;i++){
        hash *= 256;
        hash += (unsigned long long)(src[i]);
    }
    result += table.table[FIRST(hash)];
    result += table.table[SECOND(hash)];
    result += table.table[THIRD(hash)];
    result += table.table[FORTH(hash)];
    return std::move(result);
    }

int main(){
    std::string str;
    while(std::cin>>str)
    {
        if(str.length() / 3 != 0){
            for(int i=0;i<str.length() % 3;i++)
                str.push_back(0);
            }
        for(int i=0;i < str.length() / 3;i++){
            std::cout<<convert(str.substr(i * 3,3));
            }
            std::cout<<std::endl;
           std::cout<<"Completed"<<std::endl;
        }
    return 0;
}
