
#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <Windows.h>
#include <stack>
using namespace std;
/*
对算符，界符，保留字表进行define
*/
#define PLUS 13
#define SUB 14
#define MULTI 15
#define DIV 16
#define EQUAL 17
#define JING 18
#define LE 19
#define LEQ 20
#define GE 21
#define GEQ 22
#define DOT 23
#define COMMA 24
#define SEMI 25
#define COLON 26
#define LEFT 27
#define RIGHT 28
#define ENDCHAR -1 
//算符，界符，保留字存储在KeyWordList中
string KeyWordList[29] = { "const","var","procedure","begin","end", "odd", "if" ,"then" ,"call","while","do", "read", "write","+","-","*","/","=","#","<","<=",">",">=",".",",",";",":=","(",")" };
int KeyWordNum = 13;//计数KeyWord个数
FILE* inputer;
const char* inputerRoute = "D:prog22.pas";//输入源程序路径
char buffer =NULL;
int lineCounter = 1;//指向第一行，计数源代码中的行数
stack<int> pointerStack;//procedure节点stack,说明procedure之间的层次关系
static int pointer = 0;//指向符号表上当前层的开始处
/*
存储等待回填的代码的信息
*/
struct innerNode
{
    int codesline;//代码行数
    int PRELEVEL;//产生代码时的层数
    int pointer1;//产生代码时的pointer值
};
/*
符号表节点
*/
struct wordNode
{
    int index;
    string word;
    string kind;
    int level;
    int adr;
    bool declare;//是否声明
    int father;//指向其父亲在符号表中的位置
    int innerPort;//说明入口地址
    int codePort;//代码跳转地址
    int constValue;
    int B;//记录静态链上的基地址
    bool isassigned;//是否赋值
    int assignedValue;//赋予的值
    innerNode codeFrom;//存储过程开始时的跳转语句的信息
    vector<innerNode> called;//说明会call它的语句的CODE地址
    int TableIndex;
    wordNode(int _index, string _word) { index = _index; word = _word; declare = false; father = -1; innerPort = -1; TableIndex = -1; codePort = -1; B = -1; isassigned = false; }
    //每个节点的父亲初始化为-1
    //只有procedure节点会写明father.
};
vector<wordNode> ID;//标识符表
int idIndex = 0;//标识符表计数
int numIndex = 0;//常数表计数
vector<wordNode> NUM;//常数表
vector<wordNode> Table;//符号表

/*
词法分析使用的表，同时也为建树时候的表
*/
struct Node
{
    int type;//type 1 keyWordList  type 2:ID type 3:NUM
    int value;
    int outLEV;
    int line;//所在的源代码行
    string means;//当前节点的名称
    vector<int> sons;//当前节点可以推出的节点
    Node(int _type, int _value) { type = _type; value = _value; }
};

vector<Node> SYM;//单词表

/*
GEN产生的CODE节点
*/
struct codeNode
{
    string op;//操作
    int lev;//层次
    int offset;//偏移量
    int TableIndex;//call的东西在符号表中的位置
};
vector<codeNode> Codes;//存放中间代码的动态数组

/*
每次从源代码中读入一个字符
*/
char GETCH()
{
    char buf = buffer;
    if (buffer != NULL)
    {
        buffer = NULL;
        return buf;
    }
    else
    {
        buf = fgetc(inputer);
        //将buf都转化为小写
        if (buf >= 'A' && buf <= 'Z')
        {
            buf = buf + 32;
        }
        return buf;
    }
}
/*
是否为字母
*/
bool isLetter(char a)
{
    return ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z'));
}
/*
是否为数字
*/
bool isNumber(char a)
{
    return (a >= '0' && a <= '9');
}
/*
判断某个字符串是否在标识符表中，若不是则返回-1
*/
int isID(string a)
{
    vector<wordNode>::iterator it = ID.begin();
    int c = 0;
    for (; it != ID.end();it++)
    {
        if (it->word == a)
        {
            return c;
        }
        c++;
    }
    return -1;
}
/*
判断某个字符串是否已经在常数表中
*/
int isNUM(string a)
{
    vector<wordNode>::iterator it = NUM.begin();
    int c = 0;
    for (; it != NUM.end(); it++)
    {
        if (it->word == a)
        {
            return c;
        }
        c++;
    }
    return -1;
}
/*
判断是否是KeyWord
*/
int isKey(string a)
{
    for (int i = 0; i < KeyWordNum; i++)
    {
        if (KeyWordList[i] == a)
        {
            return i;
        }
    }
    return -1;
}
/*
词法分析主程序
*/
bool GETSYM()
{
   
    while (true)//the first loop
    { 
        string curstr ="";
        char cur = GETCH();
        if (cur == ' '||cur=='\t')
        {
            continue;
        }
        if (isLetter(cur))
        {
            curstr += cur;
            while (true)
            {
                cur = GETCH();
                if (isLetter(cur) || isNumber(cur))
                {
                    curstr += cur;
                }
                else
                {
                    buffer = cur;
                    break;
                }
            }
            int posi = isKey(curstr);
            if (posi>=0)
            {
                Node temp(1, posi); temp.line = lineCounter;
                SYM.push_back(temp);
            }
            else
            {//应当判断是不是已经在表中
                int flag = isID(curstr);
                if (flag != -1)
                {
                    Node temp(2, flag); temp.line = lineCounter;
                    SYM.push_back(temp);
                }
                else
                {
                    wordNode words(idIndex, curstr);
                    ID.push_back(words);
                    Node temp(2, idIndex); temp.line = lineCounter;
                    SYM.push_back(temp);
                    idIndex++;
                }
            }
        }
        else
        {
            if (isNumber(cur))
            {
                curstr += cur;
                while (true)
                {
                    cur = GETCH();
                    if (isNumber(cur))
                    {
                        curstr += cur;
                    }
                    else
                    {
                        if (isLetter(cur))
                        {
                            printf("Line %d: Error: alphabet after digit\n", lineCounter);
                            break;
                        }
                        buffer = cur;
                        break;
                        
                    }     
                }
                int flag = isNUM(curstr);
                if (flag != -1)
                {
                    Node temp(3, flag);
                    temp.line = lineCounter;
                    SYM.push_back(temp);
                }
                else
                {
                    //应当判断是不是已经在表中
                    wordNode digits(numIndex, curstr);
                    NUM.push_back(digits);
                    Node temp(3, numIndex);
                    temp.line = lineCounter;
                    SYM.push_back(temp);
                    numIndex++;
                }
            }
            else
            {
                if (cur == '+')
                {
                    Node temp(1, PLUS);
                    temp.line = lineCounter;
                    SYM.push_back(temp);
                }
                else
                {
                    if (cur == '-')
                    {
                        Node temp(1, SUB);
                        temp.line = lineCounter;
                        SYM.push_back(temp);
                    }
                    else
                    {
                        if (cur == '*')
                        {
                            Node temp(1, MULTI);
                            temp.line = lineCounter;
                            SYM.push_back(temp);
                        }
                        else
                        {
                            if (cur == '/')
                            {
                                Node temp(1, DIV);
                                temp.line = lineCounter;
                                SYM.push_back(temp);
                            }
                            else
                            {
                                if (cur == '=')
                                {
                                    Node temp(1, EQUAL);
                                    temp.line = lineCounter;
                                    SYM.push_back(temp);
                                }
                                else
                                {
                                    if (cur == '#')
                                    {
                                        Node temp(1, JING);
                                        temp.line = lineCounter;
                                        SYM.push_back(temp);

                                    }
                                    else
                                    {
                                        if (cur == '<')
                                        {
                                            curstr += cur;
                                            cur = GETCH();
                                            if (cur == '=')
                                            {
                                                Node temp(1, LEQ);
                                                temp.line = lineCounter;
                                                SYM.push_back(temp);
                                            }
                                            else
                                            {
                                                buffer = cur;
                                                Node temp(1, LE);
                                                temp.line = lineCounter;
                                                SYM.push_back(temp);
                                            }
                                        }
                                        else
                                        {
                                            if (cur == '>')
                                            {
                                                curstr += cur;
                                                cur = GETCH();
                                                if (cur == '=')
                                                {
                                                    Node temp(1, GEQ);
                                                    temp.line = lineCounter;
                                                    SYM.push_back(temp);
                                                }
                                                else
                                                {
                                                    buffer = cur;
                                                    Node temp(1, GE);
                                                    temp.line = lineCounter;
                                                    SYM.push_back(temp);
                                                }
                                            }
                                            else
                                            {
                                                if (cur == '.')
                                                {
                                                    Node temp(1, DOT);
                                                    temp.line = lineCounter;
                                                    SYM.push_back(temp);
                                                }
                                                else
                                                {
                                                    if (cur == ',')
                                                    {
                                                        Node temp(1, COMMA);
                                                        temp.line = lineCounter;
                                                        SYM.push_back(temp);
                                                    }
                                                    else
                                                    {
                                                        if (cur == ';')
                                                        {
                                                            Node temp(1, SEMI);
                                                            temp.line = lineCounter;
                                                            SYM.push_back(temp);
                                                        }
                                                        else
                                                        {
                                                            if (cur == ':')
                                                            {
                                                                curstr += cur;
                                                                cur = GETCH();
                                                                if (cur == '=')
                                                                {
                                                                    curstr += cur;
                                                                    Node temp(1, COLON);
                                                                    temp.line = lineCounter;
                                                                    SYM.push_back(temp);
                                                                }
                                                                else
                                                                {        
                                                                    printf("Line %d: Error: no such combination\n",lineCounter);
                                                                    return false;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if (cur == '(')
                                                                {
                                                                    Node temp(1, LEFT);
                                                                    temp.line = lineCounter;
                                                                    SYM.push_back(temp);
                                                                }
                                                                else
                                                                {
                                                                    if (cur == ')')
                                                                    {
                                                                        Node temp(1, RIGHT);

                                                                        temp.line = lineCounter;
                                                                        SYM.push_back(temp);
                                                                    }
                                                                    else
                                                                    {
                                                                        if (cur == ENDCHAR)
                                                                        {
                                                                            break;
                                                                        }
                                                                        else
                                                                        {
                                                                            if (cur == '\n')
                                                                            {
                                                                                lineCounter++;
                                                                                continue;
                                                                            }
                                                                            else
                                                                            {
                                                                                if (cur == '\t')
                                                                                {
                                                                                    continue;
                                                                                }
                                                                                else
                                                                                {
                                                                                    printf("Line %d: Error:Invalid input data\n", lineCounter);
                                                                                    return false;
                                                                                }
                                                                            }
                                                                        }

                                                                    }
                                                                }
                                                                
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        
    }
    return true;
}
//词法分析结果：1---保留字表中的索引   2----ID符表    3-----NUM常量表
//表中索引从0开始

int CURLEVEL = 0;//语法分析时记录当前层数
int SYMLength = 0;//SYM表长度
int SYMindex = 0;//SYM表索引，语法分析时一遍扫描，用该值索引单词
int valueIndex = 0;//在SYM表后填充树的节点信息，valueIndex为应当填入的值的位置

/*
语法分析函数声明
*/
int ParPro();
int consdeclare();
int consdef();
int valdeclare();
int processdeclare();
int processHead();
int sentence();
int assignLogue();
int condition();
int condiLogue();
int expression();
int item();
int factor();
int whileLogue();
int readLogue();
int writeLogue();
int callLogue();
int multiLogue();


void GEN(string op, int lev, int offset, int processIDIndex=-1);
int GetCodeLine();

/*
回填函数
*/
void FillCode(int line, int num,int l=-1)
{
    Codes[line].offset = num;
    if (l != -1)
    {
        Codes[line].lev = l;
    }
}
/*
isfather是否是ischild的祖先
*/
bool isAncester(int isfather,int ischild)
{
    int temp = ischild;
    if (Table[ischild].father == -1)
    {
        return false;
    }
    temp = Table[ischild].father;
    while (true)
    {
        if (temp == isfather)
        {
            return true;
        }
        temp = Table[temp].father;
        if (temp == -1)
        {
            return false;
        }
        if (temp == 0)
        {
            if (temp == isfather)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}


//批量回填---通过vector数组拉链
void CalledFill(vector<innerNode> adr, int num)
{
    for (int i = 0; i < adr.size(); i++)
    {
        if (adr[i].pointer1 < pointer)
        {
            bool a=isAncester(adr[i].pointer1,pointer);
            if (!a)
            {
                printf("Error in analyse:call invalid level: %d\n",adr[i].PRELEVEL);
                return;
            }
        }
        FillCode(adr[i].codesline, num, CURLEVEL - adr[i].PRELEVEL);

    }
}
/*
语法分析程序
*/
void BLOCK()
{
    SYMLength = SYM.size();//初始化SYM数组长度
    valueIndex = SYMLength;//初始化SYM数组要填入的树节点的索引
 
    int ff = ParPro();//ParPro返回对应节点所在的索引
    if (ff!=-1 && SYM[SYMindex].type == 1 && SYM[SYMindex].value == DOT)
    {
        Node temp(4, 0);//type 4说明是插入的变量
        temp.means = "Pro";
        temp.sons.push_back(ff);
        temp.sons.push_back(SYMindex);
        SYM.push_back(temp);//代表Pro
        valueIndex++;
        GEN("OPR", 0, 0);
        if (SYMindex + 1 == SYMLength)
        {

            printf("PROGRAM END\n");
        }
        else
        {
            printf("line %d:Error:has word after Dot\n",SYM[SYMindex].line);
        }
    }
    else
    {
        if (SYM[SYMindex].type != 1 ||SYM[SYMindex].value != DOT)
        {
            printf("line %d:Error:No DOT after ParPro\n",SYM[SYMindex].line);
        }
        if (ff == -1)
        {
            printf("line %d:Error:in ParPro\n");
        }
        

    }
        
    valueIndex++;
}


static int countInta = 0;//计数当前procedure中声明的变量个数
int ParPro()
{  
    Node temp(4, 0);//创建一个新的Node节点，找到它推导的子节点的索引，填入sons量中，该子程序结束时，将Node填入SYM,作为树节点
    temp.means="ParPro";
    int preIndex = SYMindex;
    int fa = consdeclare();
    int fb = valdeclare();
    
    int innerSentence = 0;
    innerSentence = GetCodeLine();//得到下一条指令的位置
    /*
    判断当前过程的入口是否设置
    */
    if (Table[pointer].innerPort == -1)
    {
        Table[pointer].innerPort = innerSentence;
        CalledFill(ID[Table[pointer].TableIndex].called, innerSentence);//回填call该过程的指令
        /*
        回填call该过程的指令在本程序中实际上是没有意义的，此处仅实现这个功能
        */
    }    
    if (countInta > 0)
    {    
        GEN("INT", 0, countInta);
        countInta = 0;
    }//出一个分程序中的第一个中间代码----为变量和常量开辟数据区  
    
     //存储JMP代码的信息
    innerNode temple;
    temple.codesline = GetCodeLine();
    temple.PRELEVEL = CURLEVEL;
    temple.pointer1 = pointer;
    ID[Table[pointer].TableIndex].codeFrom = temple;
    GEN("JMP", 0, -1);


    int fc = processdeclare();
    int fd = sentence();

   
    if (fa != -1)
    {
        temp.sons.push_back(fa);
        
    }
    if (fb != -1)
    {
        temp.sons.push_back(fb);
    }
    if (fc != -1)
    {
        temp.sons.push_back(fc);
    }
    if (fd == -1)
    {
        printf("Error in ParPro:in Sentence");
        return -1;
    }
    else
    {
        temp.sons.push_back(fd);
        SYM.push_back(temp);
        int curIndex = valueIndex;
        valueIndex++;
        return curIndex;
    }
}
/*
一个试探过程，通过试探有无CONST判断是否为常量声明
即SYMindex在未读入CONST之前不增，即指向单词数组的指针不改变
*/
int consdeclare()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "consdeclare";
    if (cur.type == 1 && cur.value == 0)//读入的是CONST
    {
        temp.sons.push_back(SYMindex);//存入CONST
        SYMindex++;//读入const之后单词指针才改变
        while (true)
        {
            int fa = consdef();
            if (fa == -1)
            {
                printf("Error:in consdef\n");
                return -1;
            }
            else
            {
                temp.sons.push_back(fa);
                cur = SYM[SYMindex];
                temp.sons.push_back(SYMindex);
                SYMindex++;
                if (cur.type == 1)
                {
                    if (cur.value == SEMI)
                    {
                        SYM.push_back(temp);
                        int curIndex = valueIndex;
                        valueIndex++;
                        return curIndex;
                    }
                    else
                    {
                        if (cur.value != COMMA)//逗号
                        {
                            printf("line %d:Error:No suitable word in consdeclare\n",cur.line );
                            return -1;
                        }
                    }
                }
            }
        }
    }
    else
    {
        return -1;//此时读入的单词当作没有读入
    }
}
/*
是否与祖先的标识符重复
*/
int weatherRepeat(Node cur)
{
  
   //pointer是table中指向当前procedure开始处的
   if (Table.size()==0)
    {
        return 1;
    }
    int father = Table[pointer].father;//找到当前过程父节点
    int comp = pointer;
    
    string curNm = ID[cur.value].word;//当前标识符名称
    while (true)
    {
        //从父节点的procedure项开始向下遍历符号表，看是否找到同名量
        string Name = Table[comp].word;
        if (Name == curNm)
        {
            return -1;
        }
        comp++;
        if (comp >= Table.size() && father == -1)
        {
            return 1;
        }
        if (comp >= Table.size())
        {
            
            comp = father;
            father = Table[father].father;
            continue;
        }
        if (Table[comp].kind == "PROCEDURE" )//遍历到下一个procedure的开始处时，转到父节点继续遍历
        { 
            if (father == -1)
            {
                break;
            }
             comp = father;
            father = Table[father].father;
           
           
        }
    }
    return 1;
}

int consdef()
{
    Node temp(4, 0);
    temp.means = "consdef";
    Node cur = SYM[SYMindex];
    
    SYMindex++;
    if (cur.type == 2)
    { 
        ID[cur.value].kind = "CONST"; ID[cur.value].adr = -1;
        ID[cur.value].declare = true; 
        Node name = cur;
        //判断读入的标识符是否和祖先重名
        if (weatherRepeat(cur) == -1)
        {
            printf("Error in analyse:Repeat ID:Line: %d",cur.line);
            printf("\n");
            return -1;
        }
       
        temp.sons.push_back((SYMindex - 1));
        cur = SYM[SYMindex];
        SYMindex++; 
        if (cur.type == 1 && cur.value == EQUAL)
        {
            temp.sons.push_back((SYMindex - 1));
            cur = SYM[SYMindex];
            SYMindex++;
            if (cur.type == 3)
            {
                string number = NUM[cur.value].word;
                int num = atoi(number.c_str());
                //读入const直接存入constValue中
                ID[name.value].constValue= num;
                ID[name.value].level = CURLEVEL;

                //建符号表，并和词法分析读入的ID表建立对应关系
                wordNode tabelNode = ID[name.value];
                tabelNode.index = Table.size();
                tabelNode.TableIndex = name.value;
                Table.push_back(tabelNode);
                ID[name.value].TableIndex = tabelNode.index;
                

                temp.sons.push_back((SYMindex - 1));
                SYM.push_back(temp);
                int curIndex = valueIndex;
                valueIndex++;
                return curIndex;
            }
            else
            {
                printf("line %d:Error:not NUM\n", cur.line);
                return -1;
            }
        }
        else
        {
            printf("line %d:Error:Not EQUAL symbol\n", cur.line);
            return -1;
        }
    }
    printf("line %d:Error:Not ID\n", cur.line);
    return -1;
}
/*
试探性读入单词，若为var,则移动单词指针，否则不移动单词指针并退出子程序
*/
int valdeclare()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means="valdeclare";
    if (cur.type == 1 && cur.value == 1)
    {
        temp.sons.push_back(SYMindex);
        SYMindex++;//此时才认为真正进入valdeclare,单词指针递增
        while (true)
        {
            cur = SYM[SYMindex];//当前读入单词
            
            /*
            为Variable修改符号表中的内容
            */
            if (cur.type == 2)//是标识符
            {
                ID[cur.value].kind = "VARIABLE"; ID[cur.value].level = CURLEVEL;
                ID[cur.value].declare = true;

                if (weatherRepeat(cur) == -1)
                {
                    printf("Error in analyse:Repeat ID:Line: " + cur.line);
                    printf("\n");
                    return -1;
                }
                if (cur.value - 1 < 0)
                {
                    ID[cur.value].adr = 3;
                }
                else
                {
                    if (ID[cur.value - 1].adr == -1 || ID[cur.value - 1].adr == -2)
                    {
                        ID[cur.value].adr = 3;
                    }
                    else
                    {
                        ID[cur.value].adr = ID[cur.value - 1].adr + 1;
                    }
                }
                /*建立符号表项，和ID表建立对应关系*/
                wordNode TableNode = ID[cur.value];
                TableNode.TableIndex = cur.value;
                TableNode.index = Table.size();
                Table.push_back(TableNode);
                ID[cur.value].TableIndex = TableNode.index;
                
                
                countInta++;//递增当前过程的变量计数值
                temp.sons.push_back(SYMindex);
                SYMindex++;
                cur = SYM[SYMindex];
                temp.sons.push_back(SYMindex);
                SYMindex++;
                if (cur.type == 1)//是固定标记
                {
                    if (cur.value == SEMI)//分号说明结束
                    {
                        SYM.push_back(temp);
                        int curIndex = valueIndex;
                        valueIndex++;
                        return curIndex;
                    }
                    else
                    {
                        if (cur.value != COMMA)//逗号则继续循环
                        {
                            printf("line %d:Error:Invalid ID\n",cur.line);
                            return -1;
                        }
                    }
                }

            }
            else//若读入的不是标识符
            {
                printf("line %d:Error:Invalid ID\n",cur.line);
                return -1;
            }
        }
        
    }
    else
    {
        return -1;
    }


}

/*
过程说明结束之后，pointer应当指向当前过程说明的父亲
此处主要涉及对pointer和pointerStack的操作
通过是否读入procedure符号试探是否为过程说明，若是则递增单词指针，否则不递增并返回
在该子程序中通过不断变更pointer的值和pointerStack的变动，完成在符号表中拉起一条father链，并
使得pointer始终指向当前过程的符号表起始位置
*/
int processdeclare()
{
    Node cur = SYM[SYMindex];//用于试探
    Node temp(4, 0);
    temp.means = "processdeclare";
    if (cur.type == 1 && cur.value == 2)
    {
        int fa = processHead();
        int fb = ParPro();
        
        if (fa != -1)
        {
            temp.sons.push_back(fa);
        }
        else
        {
            printf("Error:in procesdure head\n");
            return -1;
        }
        if (fb != -1)
        {
            temp.sons.push_back(fb);
        }
        else
        {
            printf("Error: in ParPro\n");
            return -1;
        }
        cur = SYM[SYMindex];
        SYMindex++;

        if (cur.type == 1 && cur.value == SEMI)//说明分程序之后的分号
        {
            /*
            processhead中压入了声明过程的pointer,此时的栈顶不是退出声明过程后的过程pointer
            需要弹出栈顶，弹出后的栈顶即为上一个过程调用前的栈顶
            */
            temp.sons.push_back(SYMindex-1);
            pointerStack.pop(); 
            int father = pointerStack.top();//记录该pointer为下一个过程的father 

            GEN("OPR", 0, 0);
            cur = SYM[SYMindex];
            if (cur.type == 1 && cur.value == 2)//说明要进入下一过程说明
            {
                CURLEVEL--;
                int temple = CURLEVEL;//记录当前level
                int fc = processdeclare();
                if (fc != -1)
                {
                    /*
                    由于过程结束后栈顶为父亲，pointer也指向父亲
                    */
                    pointer = father;
                    CURLEVEL = temple;//process建立结束，恢复之前的level
                    temp.sons.push_back(fc);
                    SYM.push_back(temp);
                    int curIndex = valueIndex;
                    valueIndex++;
                    return curIndex;
                }
                else
                {
                    printf("Error:in processDeclare\n");
                    return -1;
                }
            
            }//当前过程结束
            else
            {
                pointer = father;
                SYM.push_back(temp);
                int curIndex = valueIndex;
                valueIndex++;
                CURLEVEL--;
                return curIndex;
            }
        }
        else
        {
            printf("line %d:Error:lose SEMI\n",cur.line);
            return -1;
        }

      
    }
    return -1;
    
    

}

/*
处理过程说明头部，在此处维护的层数变量CURLEVEL增加，在pointerStack栈顶得到过程说明前的pointer,将其设置为father，
将procedure项加入符号表，得到当前过程的pointer，并将其压入pointerStack
在该过程中存储father关系
*/
int processHead()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "processHead";
    temp.sons.push_back(SYMindex);
    SYMindex++;
    cur = SYM[SYMindex];
    if (cur.type == 2)
    {
        ID[cur.value].kind = "PROCEDURE"; ID[cur.value].level = CURLEVEL; ID[cur.value].adr = -2;///////////////-1表示const,,-2表示procedure
        ID[cur.value].declare = true;
        if (CURLEVEL > 3)//判断是否有超过三层---本程序没有此条限制可以支持超过三层的情况
        {
            printf("Error in analyse:More than three layer, line: %d\n", cur.line);
        }
        if (weatherRepeat(cur) == -1)
        {
            printf("Error in analyse:Repeat ID:Line: " + cur.line);
            printf("\n");
            return -1;
        }

        CURLEVEL++;
        int father = pointerStack.top();//得到父亲pointer
        /*
        此时的father是Table表中的father而不是ID表中的
        */
        ID[cur.value].father = father;
        /*将procedure项加入符号表，和ID表建立联系*/
        wordNode TableNode = ID[cur.value];
        TableNode.index = Table.size();
        TableNode.TableIndex = cur.value;
        Table.push_back(TableNode);
        ID[cur.value].TableIndex = TableNode.index;

        temp.sons.push_back(SYMindex);
        SYMindex++;
        cur = SYM[SYMindex];
        if (cur.type == 1 && cur.value == SEMI)
        {
            
            pointer = TableNode.index;//得到当前procedure所在指针
            pointerStack.push(pointer);
           
            temp.sons.push_back(SYMindex);
            SYMindex++;
            SYM.push_back(temp);
            int curIndex = valueIndex;
            valueIndex++;
            return curIndex;
        }
        else
        {
            printf("line %d:Error:lose SEMI\n",cur.line);
            return -1;
        }
    }
    else
    {
        printf("line %d:Error:Invalid word\n",cur.line);
        return -1;
    }
}


/*
根据sentence确定语句入口地址
更新codePort时，可能需要回填
*/
int sentence()
{
    /*记录进入sentence时候的单词指针，由于需要对每个语句进行试探，
    在试探失败的时候恢复当前指针，试探下一条语句
    */
    int preIndex = SYMindex;
    Node temp(4, 0);
    temp.means = "sentence";

    /*
    判断当前过程的语句入口是否设置，若没有设置则在此处设置
    */
    int innerSentence = 0;
    innerSentence = GetCodeLine();
    int flag = 0;//标识是否在此处设置语句入口
    if (Table[pointer].codePort== -1)
    {
        flag = 1;
        Table[pointer].codePort = innerSentence;
        vector<innerNode> tempcode; tempcode.push_back(ID[Table[pointer].TableIndex].codeFrom);
        CalledFill(tempcode, innerSentence);//回填ParPro中的JMP语句，填入应当JMP到的位置

    }

    int fa = assignLogue();
    if (fa == -1)
    {
        SYMindex = preIndex;
        int fb = condiLogue();
        if (fb == -1)
        {
            SYMindex = preIndex;
            int fc = whileLogue();
            if (fc == -1)
            {
                SYMindex = preIndex;
                int fd = callLogue();
                if (fd == -1)
                {
                    SYMindex = preIndex;
                    int fe = readLogue();
                    if (fe == -1)
                    {
                        SYMindex = preIndex;
                        int ff = writeLogue();
                        if (ff == -1)
                        {
                            SYMindex = preIndex;
                            int fg = multiLogue();
                            if (fg == -1)//此时只可能读空
                            {
                                /*
                                此时读空，则之前设置的入口实际上并不在sentence的范围内，应重新设为1
                                */
                               if (flag == 1)
                                {
                                    Table[pointer].codePort = -1;
                                    vector<innerNode> tempcode; tempcode.push_back(ID[Table[pointer].TableIndex].codeFrom);
                                    CalledFill(tempcode, -1);
                                  
                                }
                                SYMindex = preIndex;
                                SYM.push_back(temp);
                                int curIndex = valueIndex;
                                valueIndex++;
                                return curIndex;
                            }
                            else
                            {
                                temp.sons.push_back(fg);
                                SYM.push_back(temp);
                                int curIndex = valueIndex;
                                valueIndex++;
                                return curIndex;
                            }
                        }
                        else
                        {
                            temp.sons.push_back(ff);
                            SYM.push_back(temp);
                            int curIndex = valueIndex;
                            valueIndex++;
                            return curIndex;
                        }
                    }
                    else
                    {
                        temp.sons.push_back(fe);
                        SYM.push_back(temp);
                        int curIndex = valueIndex;
                        valueIndex++;
                        return curIndex;
                    }
                }
                else
                {
                    temp.sons.push_back(fd);
                    SYM.push_back(temp);
                    int curIndex = valueIndex;
                    valueIndex++;
                    return curIndex;
                }
            }
            else
            {
                temp.sons.push_back(fc);
                SYM.push_back(temp);
                int curIndex = valueIndex;
                valueIndex++;
                return curIndex;
            }
        }
        else
        {

            temp.sons.push_back(fb);
            SYM.push_back(temp);
            int curIndex = valueIndex;
            valueIndex++;
            return curIndex;
        }
    }
    else
    {
        temp.sons.push_back(fa);
        SYM.push_back(temp);
        int curIndex = valueIndex;
        valueIndex++;
        return curIndex;
    }
     
}

/*
对标识符的有效性进行判断
并且返回相对位移量
正数----位移量   负数---错误
*/
int judgeNode(Node inn,int error=1 )//默认要输出error信息
{
    int curPointer = pointer;//此时指向当前
    //在符号表中寻找符号
    wordNode cur = ID[inn.value];
    //判断是否声明
    if (cur.declare == 0)
    {
        if (error == 1)
        {
            printf("Error in analyse:not declare::Line: %d",inn.line);
            printf("\n");
        }
        return -2;
    }
    int begin = ID[inn.value].TableIndex;//找到符号出现的位置
    while (true)
    {
        begin--;
        if (begin < 0)
        {
            begin = 0;
            break;
        }
        if (Table[begin].kind == "PROCEDURE")
        {
            break;
        }
    }
    //判断是否在其祖先或自己处声明
    //此时break是其所属的过程
    while (true)
    {
        if (curPointer == begin)
        {
            break;
        }
        else
        {
            if (curPointer == 0)
            {
                if (error == 1)
                {
                    printf("Error in analyse:not in valid procedure::Line: %d" ,inn.line);
                    printf("\n");
                }
                return -1;
            }
            curPointer = Table[curPointer].father;
        }
    }
    if (begin == 0)
    {
        return ID[inn.value].TableIndex - begin;
    }
    return ID[inn.value].TableIndex - begin - 1;

}

/*
赋值语句,生成STO指令
*/
int assignLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "assignLogue";
    if (cur.type == 2)
    {
        Node iden = cur;//记录当前标识符
        /*
        不能对常量进行赋值
        */
        if (ID[cur.value].kind== "CONST"||ID[cur.value].adr==-2)
        {
            printf("Error in analyse:Const before := Line %d\n", cur.line);
        }
        temp.sons.push_back(SYMindex);
        SYMindex++;
        cur = SYM[SYMindex];
        if (cur.type == 1 && cur.value == COLON)
        {
            temp.sons.push_back(SYMindex);
            SYMindex++;
            int fa = expression();
            if (fa != -1)
            {
                /*
                生成sto代码，将栈顶存储到标识符中
                */
                int boool = judgeNode(iden);
                if (boool == -1||boool==-2)
                {
                    return -1;
                }
                int l = CURLEVEL - ID[iden.value].level;
                GEN("STO", l,boool);


                temp.sons.push_back(fa);
                SYM.push_back(temp);
                valueIndex++;
                return (valueIndex - 1);
            }
            else
            {
                printf("Error:in expression\n");
                return -1;
            }
        }
        else
        {
            printf("line %d:Error:lose equal\n",cur.line);
            return -1;
        }
    }
    else
    {
       
        return -1;
    }
}

/*
条件语句
*/
int condiLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "condiLogue";
    int aftThenLine=0;
    if (cur.type == 1 && cur.value == 6)
    {
        temp.sons.push_back(SYMindex);
        SYMindex++;//此时可以确认必然在CondiLogue中
        int fa = condition();
        if (fa != -1)
        {
            temp.sons.push_back(fa);
            cur = SYM[SYMindex];
            if (cur.type == 1&&cur.value == 7)
            {
                /*
                读入then之后得到它的代码地址
                此时生成的JPC并不知道结束后应当跳转到的位置
                */
                aftThenLine = GetCodeLine(); 
                GEN("JPC", 0, -1);

                temp.sons.push_back(SYMindex);
                SYMindex++;
                int fb = sentence();
                if (fb != -1)
                {
                    /*
                    sentence输出完成，回填falselist
                    */
                    FillCode(aftThenLine, GetCodeLine());

                    temp.sons.push_back(fb);
                    SYM.push_back(temp);
                    int curIndex = valueIndex;
                    valueIndex++;
                    return curIndex;
                }
                else
                {
                    printf("Error:in sentence\n");
                    return -1;
                }
            }
            else
            {
                printf("Line %d:Error:lose then\n",cur.line);
                return -1;
            }
        }
        else
        {
            printf("Error:in condition\n");
            return -1;
        }
    }
    return -1;
}
/*
condition运行完后栈顶存储条件结果
0为否 1为是
*/
int condition()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "condition";
    if (cur.type == 1 && cur.value == 5)//odd
    {
        temp.sons.push_back(SYMindex);
        SYMindex++;
        int fa = expression();
        if (fa != -1)
        {
            /*
            奇偶性判断
            */
            GEN("OPR", 0, 6);

            temp.sons.push_back(fa);
            SYM.push_back(temp);
            int curIndex = valueIndex;
            valueIndex++;
            return curIndex;
        }
        else
        {
            printf("Error:in expression\n");
            return -1;
        }
    }
    else
    {
        int fa = expression();
        if (fa == -1)
        {
            printf("Error:in expression\n");
            return -1;
        }
        temp.sons.push_back(fa);
        cur = SYM[SYMindex];
        temp.sons.push_back(SYMindex);
        SYMindex++;
        if (cur.type == 1 && (cur.value == EQUAL || cur.value == JING || cur.value == LE || cur.value == LEQ || cur.value == GE || cur.value == GEQ))
        {
            int fb = expression();
            if (fb != -1)
            {
                /*
                此时两个表达式都已经压入栈中
                根据条件生成目标代码
                */
                switch (cur.value)
                {
                case EQUAL:
                    GEN("OPR", 0, 8);
                    break;
                case JING:
                    GEN("OPR", 0, 9);
                    break;
                case LE:
                    GEN("OPR", 0, 10);
                    break;
                case GEQ:
                    GEN("OPR", 0, 11);
                    break;
                case GE:
                    GEN("OPR", 0, 12);
                    break;
                case LEQ:
                    GEN("OPR", 0, 13);
                    break;
                default:
                    break;
                }
                temp.sons.push_back(fb);
                SYM.push_back(temp);
                int curIndex = valueIndex;
                valueIndex++;
                return curIndex;
            }
            else
            {
                printf("Error:in expression\n");
                return -1;
            }

        }
        else
        {
            printf("Line %d:Error:Invalid symbol\n",cur.line);
            return -1;
        }
    }
    
}

/*
表达式
*/
int expression()//表达式
{
    
    Node temp(4, 0);
    temp.means = "expression";
    Node cur = SYM[SYMindex];//试探第一个word
    int flag = 0;
    if (cur.type == 1)
    {
        if (cur.value == PLUS || cur.value == SUB)//第一个为+或者-
        {  
            if (cur.value == SUB)//为-时候才有处理意义
            {
                flag = 1;
            }
            temp.sons.push_back(SYMindex);
            SYMindex++;
        }
        else
        {
            printf("Line %d: Error: invalid symbol in expression\n", cur.line);
            return -1;
        }
    }
    int fa = item();//项
    /*
    假设前面有负号，对刚刚读入的项进行处理
    */
    if (flag == 1)
    {
        GEN("OPR", 0, 1);
    }
    temp.sons.push_back(fa);
    if (fa == -1)
    {
        printf("Error:In item\n");
        return -1;
    }
    while (true)//加减运算符后一定是项
    {
        cur = SYM[SYMindex];
        if (cur.type == 1 && (cur.value == PLUS || cur.value == SUB))
        {

            temp.sons.push_back(SYMindex);
            SYMindex++;
            int fa = item();
            if (fa == -1)
            {
                printf("Error:In item\n");
                return -1;
            }
            else
            {
                /*
                每个加减循环出完成出入栈
                */
                
                if (cur.value == PLUS)
                {
                    GEN("OPR", 0, 2);
                }
                else
                {
                    if (cur.value == SUB)
                    {
                        GEN("OPR", 0, 3);
                    }
                }
                temp.sons.push_back(fa);
            }
        }
        else
        {
            break;
        }
    }
    SYM.push_back(temp);
    valueIndex++;
    return (valueIndex - 1);
}
/*
项
*/
int item()//项
{//乘除运算符之后必然要跟因子
    int fa = factor();
    Node temp(4, 0);
    temp.means = "item";
    if (fa != -1)
    {
        temp.sons.push_back(fa);
    }
    else
    {
        printf("Error:in factor\n");
        return -1;
    }
    while (true)
    {
        Node cur = SYM[SYMindex];
        if (cur.type == 1 && (cur.value == MULTI || cur.value == DIV))
        {
            temp.sons.push_back(SYMindex);
            SYMindex++;
            int fb = factor();
            if (fb == -1)
            {
                printf("Error:in factor\n");
                return -1;
            }
            /*
            根据乘除号生成指令
            */
            if (cur.value == MULTI)
            {
                GEN("OPR", 0, 4);
            }
            else
            {
                GEN("OPR", 0, 5);
            }
            temp.sons.push_back(fb);
        }
        else
        {
            break;
        }
    }
    SYM.push_back(temp);
    valueIndex++;
    return (valueIndex - 1);

}
/*
因子
*/
int factor()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "factor";
    temp.sons.push_back(SYMindex); SYMindex++;
    if (cur.type == 2 || cur.type == 3)
    {   /*
        若是无符号整数，则在之前没有人将其push到栈中
        */
        if (cur.type == 2)
        {
            /*
            涉及标识符，判断标识符的合法性
            */
            int boool = judgeNode(cur);
            if (boool == -1||boool==-2)
            {
                return -1;
            }
            int l = CURLEVEL-ID[cur.value].level;
            //根据标识符的类型判断调用LIT或LOD
            if (ID[cur.value].kind == "CONST")
            {
                GEN("LIT", 0, ID[cur.value].constValue);
            }
            else
            {
                if (ID[cur.value].adr == -2)//通过adr标识procedure变量
                {
                    printf("Error in analyse:procedure name and := Line: %d\n",cur.line );
                }
                else
                {GEN("LOD", l, boool);

                }

                
            }
        }
        else
        {
            /*
            从常数表中读出数据值，直接将值入栈
            */
            int a; string astr = NUM[cur.value].word;
            a = atoi(astr.c_str());
            GEN("LIT", 0,a);
        }

        SYM.push_back(temp);
        valueIndex++;
        return (valueIndex - 1);
    }
    else
    {
        if (cur.type == 1 && cur.value == LEFT)
        {
            int fa = expression();
            temp.sons.push_back(fa);
            if (fa != -1)
            {
                cur = SYM[SYMindex];
                SYMindex++;
                if (cur.type == 1 && cur.value == RIGHT)
                {
                    temp.sons.push_back((SYMindex - 1));
                    SYM.push_back(temp);
                    valueIndex++;
                    return (valueIndex - 1);
                }
                else
                {
                    printf("Line %d:Error:lose RIGHT brackets\n", cur.line);
                    return -1;
                }
            }
            else
            {
                printf("Error:In expression\n");
                return -1;
            }
        }
        else
        {
            printf("Line %d:Error:Invalid word\n", cur.line);
        }
    }
}

/*
while语句
*/
int whileLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    int aftDoLine = 0;//DO之后的jmp语句位置
    int beforeCondi = 0;//条件判断第一条语句的位置
    beforeCondi = GetCodeLine();
    temp.means = "whileLogue";
    temp.sons.push_back(SYMindex); SYMindex++;
    if (cur.type == 1 && cur.value == 9)//已经是while了则后面的必须按照规则执行
    {
        int fa = condition();
        temp.sons.push_back(fa);
        if (fa != -1)
        {
            cur = SYM[SYMindex];
            temp.sons.push_back(SYMindex); SYMindex++;
            if (cur.type == 1 && cur.value == 10)
            {
                /*
                此时刚刚读入do语句
                */
                aftDoLine = GetCodeLine();
                GEN("JPC", 0, -1);

                int fb = sentence();
                temp.sons.push_back(fb);
                if (fb != -1)
                {
                    /*
                    do之后的sentence结束之后需要无条件跳转到条件判断语句
                    */
                    GEN("JMP", 0, beforeCondi);
                    //代码回填：
                    FillCode(aftDoLine, GetCodeLine());

                    SYM.push_back(temp);
                    valueIndex++;
                    return (valueIndex - 1);
                }
                else
                {
                    printf("Error:In sentence\n");
                    return -1;
                }
            }
            else
            {
                printf("Line %d:Error:lose do\n", cur.line);
                return -1;
            }
        }
        else
        {
            printf("Error:in condition\n");
            return -1;
        }
    }
    else
    {       
        return -1;
    }
    
}

/*
找到两个过程的共同父亲
*/
int findsameFather(int p1, int p2)
{
    int f1 = p1;
    int f2 = Table[p2].father;
    while (true)
    {
        if (f2 == f1)
        {
            return f2;
        }
        f2 = Table[f2].father;
        if (f2 == -1)
        {
            f1 = Table[f1].father;
            f2 = Table[p2].father;
            if (f1 == -1)
            {
                return -1;
            }
        }
    }
}

/*
call语句。对call的标识符没有声明的情况提供了支持，但是在pl0中是没有必要的
*/
int callLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "callLouge";
    temp.sons.push_back(SYMindex); SYMindex++;
    if (cur.type == 1 && cur.value == 8)
    {
        cur = SYM[SYMindex];//得到要call的标识符
        temp.sons.push_back(SYMindex); SYMindex++;
        if (cur.type == 2)
        {
            /*
               判断标识符合法性
               先读入栈顶，再写入标识符
            */
            if (ID[cur.value].adr == -2)//判断call的是不是procedure
            {
                if (ID[cur.value].declare == true)
                {
                    if (ID[cur.value].TableIndex < pointer)//已经声明且调用的标识符在当前procedure标识之前
                    {
                        /*
                        合法情况为调用当前过程的兄弟，通过找到两个标识符的共同最近祖先，判断是否具有兄弟关系。
                        */
                        int samefather = findsameFather(pointer, ID[cur.value].TableIndex);
                        if (samefather == -1)
                        {
                            printf("Error in analyse:can not call Line: %d\n", cur.line);
                        }
                        else
                        {
                            if (Table[ID[cur.value].TableIndex].father != samefather)
                            {
                                printf("Error in analyse:can not call Line: %d\n", cur.line);
                            }
                        }

                        int l = CURLEVEL - ID[cur.value].level;

                        GEN("CAL", l, Table[ID[cur.value].TableIndex].innerPort, cur.value);
                    }
                    else
                    {
                        /*
                        此时的合法情况是调用子孙
                        */
                        bool a = isAncester(pointer, ID[cur.value].TableIndex);
                        if (a || ID[cur.value].TableIndex == pointer)
                        {
                            int l = CURLEVEL - ID[cur.value].level;
                            GEN("CAL", l, Table[ID[cur.value].TableIndex].innerPort, cur.value);
                        }
                        else
                        {
                            printf("Error in analyse:can not call Line: %d\n", cur.line);
                        }
                    }
                }
                else//变量没有声明的情况下，通过innerNode存储产生CAL指令时的状态
                {
                    innerNode temple;
                    temple.codesline = GetCodeLine();
                    temple.PRELEVEL = CURLEVEL;
                    temple.pointer1 = pointer;
                    ID[cur.value].called.push_back(temple);
                    GEN("CAL", 0, -1, cur.value);
                }
            }
            else//call的不是一个procedure
            {
                printf("Error in analyse: Invalid ID after call,line: %d\n", cur.line);
                return -1;
            }
          
            SYM.push_back(temp);
            valueIndex++;
            return (valueIndex - 1);
        }
        else
        {
            printf("Line: %d:invalid word after call\n", cur.line);
            return -1;
        }

    }
    return -1;
}
/*
读语句
*/
int readLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "readLouge";
    temp.sons.push_back(SYMindex); SYMindex++;
    if (cur.type == 1 && cur.value == 11)//值为read
    {
        cur= SYM[SYMindex];
        temp.sons.push_back(SYMindex); SYMindex++;
        if (cur.type == 1 && cur.value == LEFT)//值为(
        {
            while (true)
            {
                 cur = SYM[SYMindex];
                temp.sons.push_back(SYMindex); SYMindex++;
                if (cur.type != 2)
                {
                    printf("Line %d: Error:invalid word after (\n",cur.line);
                    return -1;
                }/*
                判断标识符合法性
                先读入栈顶，再写入标识符
                */
                if (ID[cur.value].kind == "CONST")
                {
                    printf("Line %d: Error:read to CONST\n", cur.line);
                    return -1;
                }
                int boool =judgeNode(cur);
                if (boool == -1)
                {
                    return -1;
                }
                int l = CURLEVEL - ID[cur.value].level;
                GEN("OPR", 0, 16); 
                GEN("STO", l, boool);
                //如何知道自己现在在哪一层
                cur = SYM[SYMindex]; temp.sons.push_back(SYMindex); SYMindex++;//标识符
                
                
                if (cur.type == 1 && cur.value == RIGHT)
                {
                    break;
                }
                else
                {
                    if (cur.type == 1 && cur.value == COMMA)//,
                    {
                        continue;
                    }
                    else
                    {
                        printf("Line %d: Error:invalid word\n",cur.line);
                        return -1;
                    }
                }

            }
            /*
            read处结束
            */

            SYM.push_back(temp);
            valueIndex++;
            return (valueIndex - 1);

        }
    }
    return -1;
}
/*
写语句
*/
int writeLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "writeLouge";
    temp.sons.push_back(SYMindex); SYMindex++;
    if (cur.type == 1 && cur.value == 12)
    {
        cur = SYM[SYMindex];
        temp.sons.push_back(SYMindex); SYMindex++;
        if (cur.type == 1 && cur.value == LEFT)
        {
            while (true)
            {
                cur = SYM[SYMindex];
                temp.sons.push_back(SYMindex); SYMindex++;
                if (cur.type != 2)
                {
                    printf("Line %d: Error:invalid word after (\n", cur.line);
                    return -1;
                }/*
               判断标识符合法性
               先把标识符放到栈顶，再写出
               */
                int boool = judgeNode(cur);
                if (boool == -1)
                {
                    return -1;
                }
                int l = CURLEVEL - ID[cur.value].level;
                if (ID[cur.value].kind == "CONST")
                {
                    GEN("LIT", 0, ID[cur.value].constValue);
                }
                else
                {
                    if (ID[cur.value].adr == -2)
                    {
                        printf("Error in analyse:procedure name and := Line: %d\n", cur.line);
                    }
                    else
                    {
                        GEN("LOD", l, boool);

                    }
                }
         
                GEN("OPR", 0, 14);
                GEN("OPR", 0, 15);//换行
                cur = SYM[SYMindex]; temp.sons.push_back(SYMindex); SYMindex++;
                


                if (cur.type == 1 && cur.value == RIGHT)
                {
                    break;
                }
                else
                {
                    if (cur.type == 1 && cur.value == COMMA)
                    {
                        continue;
                    }
                    else
                    {
                        printf("Line %d: Error:invalid word\n", cur.line);
                        return -1;
                    }
                }

            }
            SYM.push_back(temp);
            valueIndex++;
            return (valueIndex - 1);

        }
    }
    return -1;
}
/*
复合语句，没有语义分析处理
*/
int multiLogue()
{
    Node cur = SYM[SYMindex];
    Node temp(4, 0);
    temp.means = "multiLouge";
    temp.sons.push_back(SYMindex); SYMindex++;
    if (cur.type == 1 && cur.value == 3)
    {
        if (cur.line == 24)
        {
            printf("in\n");
        }
        while (true)
        {
            int fa = sentence();
            temp.sons.push_back(fa);
            if (fa == -1)
            {
                printf("Error:in sentence\n");
                return -1;
            }
            cur = SYM[SYMindex]; temp.sons.push_back(SYMindex); SYMindex++;
            if (cur.type == 1 && cur.value == 4)//end
            {
                break;
            }
            else
            {
                if (cur.type == 1 && cur.value == SEMI)//分号
                {
                    continue;
                }
                else
                {
                    printf("line %d:Error:Invalid word\n",cur.line);
                    return -1;
                }
            }
        }
        SYM.push_back(temp);
        valueIndex++;
        return (valueIndex - 1);

    }
    return -1;
}
/*
绘制语法分析得到的树
*/
//绘制节点----离边框10开始写字符，每个字符间隔10
void buildWordNode(int l1,int l2,int l3,int l4,const char *p,int length)
{
    setfillcolor(YELLOW);//设置长方形填充颜色
    setbkcolor(YELLOW);
    fillrectangle(l1, l2, l3, l4);
    int row = (l2 + l4) / 2;
    int begincol = l1 + 10;
    int i = 0;
    while (i<length)
    {
        outtextxy(begincol, row, p[i]);
        begincol += 10;
        i++;
    }

}
int rowline[20] = { 0 };//建立数组指明当前行的最右列的位置
void draw(int curs,int _row, int _col)//说明从该节点开始画----该节点已经画了
{
   Node cur = SYM[curs];//当前节点
   int row = _row;
   int col =_col;
   row++;col = rowline[row];
   int flag = true;//从左向右画
   //以树的结构遍历SYM点
   for (int i = 0; i < cur.sons.size(); i++)
   {
       Node child = SYM[cur.sons[i]];
       if (100 + 110 * col >= 1600)
       {
           row++;
           col =13 ;
           flag = false;
       }
       if (child.type == 4)
       {
           buildWordNode(20+110*col,10+row*70 ,100+110*col , 50+row*70, child.means.c_str(), child.means.length());//假设高40宽150
       }
       else
       {
           if (SYM[cur.sons[i]].type == 1)
           {
               buildWordNode(20 + 110 * col, 10 + row * 70, 100 + 110 * col, 50 + row * 70, KeyWordList[child.value].c_str(), KeyWordList[child.value].length());//假设高40宽150
           }
           else
           {
               if (SYM[cur.sons[i]].type == 2)
               {
                   buildWordNode(20 + 110 * col, 10 + row * 70, 100 + 110 * col, 50 + row * 70, ID[child.value].word.c_str(), ID[child.value].word.length());//假设高40宽150 
               }
               else
               {
                   buildWordNode(20 + 110 * col, 10 + row * 70, 100 + 110 * col, 50 + row * 70, NUM[child.value].word.c_str(), NUM[child.value].word.length());//假设高40宽150
               }
           }
       }

       line((_col*110 + 100), (_row*70 + 50), col*110+20, 70*row+10);//画父子之间的线
       if (flag)
       {
           col++; rowline[row]++;
           draw(cur.sons[i], row, (col - 1));
       }
       else
       {
           col--;
           draw(cur.sons[i], row, (col +1));
       }
   }
   
}


/*
设定此时GEN会记录一个行号
CODELine中为下一次产生的代码的地址
通过GetCodeLine得到其值
*/
int CODELine = 0;
void GEN(string op, int lev, int offset,int processIDIndex)
{
    cout << op;
    printf(" %d %d\n" ,lev, offset);
    codeNode temp; temp.op = op; temp.lev = lev; temp.offset = offset;
    temp.TableIndex = processIDIndex;
    Codes.push_back(temp);
    CODELine++;
}
int GetCodeLine()
{
    return CODELine;
}

/*
语法树的绘图初始化
*/
void drawinit()
{
    initgraph(3500, 1500);//设置绘图窗口为800 x 1500
    setlinestyle(PS_SOLID | PS_ENDCAP_ROUND, 5);
    setbkcolor(WHITE);
    cleardevice();
    setcolor(BLACK);
    line(0, 0, 1500, 0);
    setlinestyle(PS_SOLID | PS_ENDCAP_FLAT, 1);//厚度为1，的平头实线
    setlinecolor(LIGHTRED);
    int root = SYM.size()-1;
    
    buildWordNode(20, 10, 100, 50,SYM[root].means.c_str(),SYM[root].means.length());//假设高40宽80
    rowline[0] = 1;
    draw(root, 0, 0);//从根节点绘图
    _getch();
    closegraph();

}
void buildMap()//根据SYM数组创建Map
{
    drawinit();
}

stack<int> drawing;
/*
以命令行形式绘制语法树
*/
void drawInConsole()
{
    int root = SYM.size() - 1;//从root开始输出
    SYM[root].outLEV = 1;
    drawing.push(root);
    int counter = 1;
    while (true)
    {
        if (drawing.empty())
        {
            break;
        }
        int cur = drawing.top(); drawing.pop();
        Node curN = SYM[cur];
        vector<int> sons = curN.sons;
        string out = "";
        for (int i = 0; i < curN.outLEV;i++)
        {
            out += "---";
        }
        if (curN.type == 4)
        {
            out += curN.means;
        }
        else
        {
            if (curN.type == 1)
            {
                out += KeyWordList[curN.value];
            }
            else
            {
                if (curN.type == 2)
                {
                    out += ID[curN.value].word;
                }
                else
                {
                    if (curN.type == 3)
                    {
                        out += NUM[curN.value].word;
                    }
                }
            }
        }
        cout << out << endl;
        for (int i = sons.size()-1; i >=0; i--)
        {
            SYM[sons[i]].outLEV = curN.outLEV + 1;
            drawing.push(sons[i]);
        }

    }
}

vector<int> stackexec;
//----------------------------------------------解释执行部分

stack<int> newPointerStack;
int registerL = -1;//指令寄存器
int registerP = 0;//指向下一条执行的指令
int T;//栈顶寄存器
int B;//基地址寄存器---要存储静态链
/*
打印当前的调用栈
*/
void printStack()
{
    printf("----------------stack--------------\n");
    for (int i = 0; i <= T; i++)
    {
        printf("%d:  %d\n", i,stackexec[i]);
    }
    printf("----------------stack--------------\n");
}
/*
得到在符号表中的位置
*/
int getIndex(int lev, int offset)
{
    int le = CURLEVEL - lev;
    int tableIndex = 0;
    if (le == 0)//最底层
    {
        tableIndex = offset;
    }
    else
    {
        int num = lev;
        int curpointer = pointer;
        while (true)
        {
            if (num == 0)
            {
                break;
            }
            curpointer = Table[pointer].father;
            num--;
        }
        tableIndex = curpointer+offset + 1;
    }
    return tableIndex;
}

/*
处理OPR指令
*/
void OPRexec(codeNode cur)
{
    if (cur.offset == 0)//调用返回
    {
        printStack();
        int preB = stackexec[B + 1];//动态链地址
        int ret = stackexec[B + 2];//返回地址
        T = B - 1;//当前栈帧弹出
        while (stackexec.size() > B)
        {
            if (!stackexec.empty())
            {
                stackexec.pop_back();
            }
        }
        B = preB;//恢复基址寄存器
        registerP = ret;//恢复返回地址
        if (!newPointerStack.empty())
        {
            pointer = newPointerStack.top();newPointerStack.pop();
        }
        if (pointer == 0)
        {
            CURLEVEL = 0;
        }
        else
        {
            CURLEVEL = Table[pointer].level + 1;
        }
    }
    else
    {
        if (cur.offset == 1)
        {
            stackexec[T] = -stackexec[T];
        }
        else
        {
            if (cur.offset == 6)//奇偶判断
            {
                int a = stackexec[T];
                if (a % 2 == 0)
                {
                    stackexec[T] = 0;//偶数返回false
                }
                else
                {
                    stackexec[T] = 1;
                }
            }
            else
            {
                if (cur.offset == 14)
                {
                    printf("%d", stackexec[T]);
                    T--; stackexec.pop_back();

                }
                else
                {
                    if (cur.offset == 15)
                    {
                        printf("\n");
                    }
                    else
                    {
                        if (cur.offset == 16)
                        {
                            int a;
                            cin >> a;
                            stackexec.push_back(a);
                            T++;
                        }
                        else//以下指令都需要弹出两个栈顶
                        {
                            int a = stackexec[T]; T--; stackexec.pop_back();
                            int b = stackexec[T];
                            if (cur.offset == 2)
                            {
                                stackexec[T] = a + b;
                            }
                            else
                            {
                                if (cur.offset == 3)
                                {
                                    stackexec[T] = b - a;
                                }
                                else
                                {
                                    if (cur.offset == 4)
                                    {
                                        stackexec[T] = b * a;
                                    }
                                    else
                                    {
                                        if (cur.offset == 5)
                                        {
                                            stackexec[T] = b / a;
                                        }
                                        else
                                        {//以下都是栈顶和次栈的比较

                                            if (cur.offset == 8)
                                            {
                                                if (a == b)
                                                {
                                                    stackexec[T] = 1;
                                                }
                                                else
                                                {
                                                    stackexec[T] = 0;
                                                }
                                            }
                                            else
                                            {
                                                if (cur.offset == 9)
                                                {
                                                    if (a == b)
                                                    {
                                                        stackexec[T] = 0;
                                                    }
                                                    else
                                                    {
                                                        stackexec[T] = 1;
                                                    }
                                                }
                                                else
                                                {
                                                    if (cur.offset == 10)
                                                    {
                                                        if (b < a)
                                                        {
                                                            stackexec[T] = 1;
                                                        }
                                                        else
                                                        {
                                                            stackexec[T] = 0;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (cur.offset == 11)
                                                        {
                                                            if (b >= a)
                                                            {
                                                                stackexec[T] = 1;
                                                            }
                                                            else
                                                            {
                                                                stackexec[T] = 0;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if (cur.offset == 12)
                                                            {
                                                                if (b > a)
                                                                {
                                                                    stackexec[T] = 1;
                                                                }
                                                                else
                                                                {
                                                                    stackexec[T] = 0;
                                                                }
                                                            }
                                                            else
                                                            {

                                                                if (cur.offset == 13)
                                                                {
                                                                    if (b <= a)
                                                                    {
                                                                        stackexec[T] = 1;
                                                                    }
                                                                    else
                                                                    {
                                                                        stackexec[T] = 0;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
           
        }
    }


}
void nextCode()//执行下一条指令
{
    registerL = registerP;
    registerP++;

    codeNode cur = Codes[registerL];
    if (cur.op == "INT")
    {
        int num = cur.offset;
        /*for (int i = 0; i < num; i++)
        {
            stackexec.push_back(0);
            T++;
        }*/
    }
    else
    {
        if (cur.op == "LIT")
        {
            stackexec.push_back(cur.offset);
            T++;
        }
        else
        {
            if (cur.op == "LOD")
            {
                
                int tableIndex = getIndex(cur.lev, cur.offset);
                if (Table[tableIndex].isassigned == false)
                {
                    printf("EXEC ERROR:Variable is not defined CodeLine:%d\n", registerL);
                }
                else
                {
                    stackexec.push_back(Table[tableIndex].assignedValue);
                    T++;
                }
            }
            else
            {
                if (cur.op == "STO")
                {
                    int tableIndex = getIndex(cur.lev, cur.offset);
                    Table[tableIndex].isassigned = true;
                    Table[tableIndex].assignedValue = stackexec[T];//记录赋予的值
                    stackexec.pop_back(); T--;
                }
                else
                {
                    if (cur.op == "JMP")
                    {
                        registerP = cur.offset;
                    }
                    else
                    {
                        if (cur.op == "JPC")
                        {
                            if (stackexec[T] == 0)
                            {
                                registerP = cur.offset;
                            }
                            stackexec.pop_back();
                            T--;
                        }
                        else
                        {
                            if (cur.op == "CAL")
                            {
                                int preB = B;//动态链基地址
                                int returnAdr = registerP;

                                registerP = cur.offset;//给出下一条指令的地址
                                int tableInex=ID[cur.TableIndex].TableIndex;//转到的地址的index
                             
                                if (Table[tableInex].father == -1)
                                {
                                    stackexec.push_back(-1);//静态链
                                }
                                else
                                {
                                    stackexec.push_back(Table[Table[tableInex].father].B);
                                } T++;
                                    B = T;
                                    Table[tableInex].B = B;
                                    stackexec.push_back(preB);//动态链
                                    T++;
                                    stackexec.push_back(returnAdr);//返回地址
                                    T++;
                                    CURLEVEL = Table[tableInex].level + 1;
                                    newPointerStack.push(pointer);//存储进行的过程的父亲
                                    pointer = tableInex;
                            }
                            else
                            {
                                if (cur.op == "OPR")
                                {
                                    OPRexec(cur);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
void CodePro()//解释执行主程序
{
    /*
    最外层主程序
    */
    pointer = 0;

    B = 0; T = -1; Table[0].B = 0;
    CURLEVEL = 0;
    stackexec.push_back(-1);//此时静态链为-1
    T++;
    stackexec.push_back(-1);//动态链-1
    T++;
    stackexec.push_back(-1);//返回地址
    T++;
    while (registerP < Codes.size())
    {
        nextCode();
    }

}


int main()
{
    pointer = 0;
    codeNode temp;
    pointerStack.push(pointer);
    errno_t err=fopen_s(&inputer,inputerRoute,"r");
    if (err!=0)
    {
        printf("Error: the file is not found\n");
    }
    if (GETSYM())
    {
        vector<Node>::iterator it = SYM.begin();
        int c = 0;
        for (; it != SYM.end(); it++)
        {
            printf("%d: <%d,%d>\n",it->line, it->type, it->value);
        }
    }
    else
    {
        printf("Error: Something Wrong In GETSYM");
    }
    BLOCK();
    //drawinit();
  //  drawInConsole();
  //  buildMap();
    printf("-------------------------purpose Code\n");
    for (int i = 0; i < Codes.size(); i++)
    {
        printf("%d ", i);
        cout << Codes[i].op;
        printf(" %d %d\n", Codes[i].lev, Codes[i].offset);
    }
   
    printf("Run result----------------------\n");
    CodePro();
}

