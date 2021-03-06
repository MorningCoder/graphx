#include <sstream>
#include <iostream>
#include "Expression.h"

//处理最低级加减法并返回递归后的语法树
std::shared_ptr<ExpressTree> Expression::GetExpression()
{
    bool while_flag = false;
    std::shared_ptr<ExpressTree> node = nullptr;
    std::shared_ptr<ExpressTree> left = nullptr;
    std::shared_ptr<ExpressTree> right = nullptr;
    //先处理左部表达式
    left = term();
    //获取符号
    Parser::token = lexer->GetToken();
    //如果是匹配的操作符
    while(Parser::token.type == TokenType::Plus || Parser::token.type == TokenType::Minus)
    {
        while_flag = true;
        //初始化临时节点
        std::shared_ptr<ExpressTree> tmp = ExpressTree::CreateNode(Parser::token);
        //得到右部表达式
        right = term();
        //继续匹配下一个
        Parser::token = lexer->GetToken();
        //如果又是加减法
        if(Parser::token.type == TokenType::Plus || Parser::token.type == TokenType::Minus)
        {
            //产生节点左右儿子
            tmp->SetChild(Child::left,left);
            tmp->SetChild(Child::right,right);
            //设置到left
            left = tmp;
        }
        else
        {
            //先吐回去
            lexer->PushToken(Parser::token);
            //设置Node
            node = tmp;
            node->SetChild(Child::left,left);
            node->SetChild(Child::right,right);
            break;
        }
    }
    //没执行while直接返回left
    if(!while_flag)
    {
        lexer->PushToken(Parser::token);
        return left;
    }
    else
        return node;
}

//自己的匹配
void Expression::match(TokenType type)
{
    std::ostringstream out;
    //先获取符号
    Parser::token = lexer->GetToken();
    //匹配
    if(Parser::token.type == TokenType::Err)
    {
        out << "行号：" << Parser::token.no << " 错误类型：无法识别的符号";
        Parser::log.push_back(out.str());
    }
    else if(Parser::token.type != type)
    {
        out << "行号：" << Parser::token.no << "错误类型：未预期的符号：" << Parser::token.str;
        Parser::log.push_back(out.str());
    }
}

//处理常数、参数、函数、以及括号表达式
std::shared_ptr<ExpressTree> Expression::atom()
{
    //表示本节点
    std::shared_ptr<ExpressTree> node;
    //获取一个记号
    Parser::token = lexer->GetToken();
    //如果是终结符
    switch(Parser::token.type)
    {
    //最高级匹配（常数、参数、函数等）并创建一个树节点
    case TokenType::Const:
    case TokenType::T:
        node = ExpressTree::CreateNode(Parser::token);
        return node;
    case TokenType::Func:
        //先产生本节点
        node = ExpressTree::CreateNode(Parser::token);
        //如果是函数则继续匹配左括号
        match(TokenType::Lbracket);
        //产生本节点左儿子（即函数内参数子树）
        node->SetChild(Child::left,GetExpression());
        //匹配右括号
        match(TokenType::Rbracket);
        return node;
    case TokenType::Lbracket:
        //匹配左括号内表达式
        node = GetExpression();
        //匹配右括号
        match(TokenType::Rbracket);
        return node;
    default:
        std::ostringstream out;
        out << "行号：" << Parser::token.no << " 错误类型：未预期的符号";
        Parser::log.push_back(out.str());
        return nullptr;
    }
}

//处理乘方运算
std::shared_ptr<ExpressTree> Expression::component()
{
    //产生当前节点
    std::shared_ptr<ExpressTree> node;
    //产生左右儿子
    std::shared_ptr<ExpressTree> left;
    std::shared_ptr<ExpressTree> right;
    //给左儿子赋值
    left = atom();
    //获取下一个记号
    Parser::token = lexer->GetToken();
    //如果下一个记号符合乘方运算则继续匹配
    if(Parser::token.type == TokenType::Power)
    {
        //先产生本节点
        node = ExpressTree::CreateNode(Parser::token);
        //继续递归一次实现右结合
        right = component();
        //产生左右儿子节点
        node->SetChild(Child::left,left);
        node->SetChild(Child::right,right);
        return node;
    }
    //不是则将符号返回给Lexer缓冲区
    else
    {
        lexer->PushToken(Parser::token);
        return left;
    }
}

//处理一元加减法
std::shared_ptr<ExpressTree> Expression::factor()
{
    Token tmp(TokenType::Const,"0",0,nullptr,0);
    std::shared_ptr<ExpressTree> node;
    //获取符号
    Parser::token = lexer->GetToken();
    //一元加仅返回右操作树的表达式树根
    if(Parser::token.type == TokenType::Plus)
        node = factor();
    //一元减转化为二元减法（0 - x）
    else if(Parser::token.type == TokenType::Minus)
    {
        //先获创建本节点
        node = ExpressTree::CreateNode(Parser::token);
        //获取右部表达式树根
        node->SetChild(Child::right,factor());
        //左边表示常数0
        node->CreateChild(Child::left,tmp);
    }
    //匹配其他非终结符
    else
    {
        lexer->PushToken(Parser::token);
        node = component();
    }
    return node;
}

//处理普通乘除法
std::shared_ptr<ExpressTree> Expression::term()
{
    bool while_flag = false;
    std::shared_ptr<ExpressTree> node = nullptr;
    std::shared_ptr<ExpressTree> left = nullptr;
    std::shared_ptr<ExpressTree> right = nullptr;
    //先把左操作数匹配
    left = factor();
    //读取下一个符号
    Parser::token = lexer->GetToken();
    //匹配操作符
    while(Parser::token.type == TokenType::Mul || Parser::token.type == TokenType::Div)
    {
        while_flag = true;
        //初始化一个临时节点
        std::shared_ptr<ExpressTree> tmp = ExpressTree::CreateNode(Parser::token);
        //并且继续匹配右操作符
        right = factor();
        //继续匹配下一个
        Parser::token = lexer->GetToken();
        //如果又是乘除法
        if(Parser::token.type == TokenType::Mul || Parser::token.type == TokenType::Div)
        {
            //产生节点左右儿子
            tmp->SetChild(Child::left,left);
            tmp->SetChild(Child::right,right);
            //设置到left
            left = tmp;
        }
        else
        {
            //先吐回去
            lexer->PushToken(Parser::token);
            //设置Node
            node = tmp;
            node->SetChild(Child::left,left);
            node->SetChild(Child::right,right);
            break;
        }
    }
    //如果没执行while就吐回去一个符号并返回left
    if(!while_flag)
    {
        lexer->PushToken(Parser::token);
        return left;
    }
    else
        return node;
}




