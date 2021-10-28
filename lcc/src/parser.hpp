#pragma once

#include "astnode.hpp"
#include "general.hpp"
#include "lexer.hpp"

struct Parser {
    void load_source(std::string&& source);
    [[noreturn]] void error(Token& token, std::string&& msg);
    Scope* make_scope(Scope* parent);
    void put_decl(Scope* base, DeclNode* decl);

    void assert_semi_colon();

    std::unique_ptr<ProgramNode> parse_program();
    std::unique_ptr<BlockNode> parse_block();
    std::unique_ptr<FunctionNode> parse_function(std::unique_ptr<TypeLitNode>&& returnType,
                                                 std::unique_ptr<Token>&& name);

    std::unique_ptr<IfNode> parse_if();
    std::unique_ptr<WhileNode> parse_while();

    std::unique_ptr<DeclNode> parse_decl(std::unique_ptr<TypeLitNode>&& type,
                                         std::unique_ptr<Token>&& name);

    std::unique_ptr<TypeLitNode> parse_type();
    std::unique_ptr<CallNode> parse_call(std::unique_ptr<Node>&& callRef, char exprDepth);
    std::unique_ptr<Node> parse_expr();
    std::unique_ptr<Node> parse_operand(char exprDepth);
    std::unique_ptr<Node> recur_expr(char prec, char exprDepth);

    Lexer lexer;
    Scope* curScope;
    ProgramNode* curProgram;
};
