#pragma once

#include "astnode.hpp"

struct CodeGen {
    CodeGen();
    char allocate_reg();
    void free_reg(char reg);

    void gen_program(const char* outFilePath, ProgramNode& program);
    void gen_block(BlockNode* block);
    void gen_func(FunctionNode* func);
    const char* optimize_branch_condition(Node* condition);
    void gen_if(IfNode* ifnode);
    void gen_while(WhileNode* whilenode);
    void gen_local_decl(DeclNode* decl);

    char gen_expr(Node* expr);
    char gen_call(CallNode* call);
    char gen_ident(NameNode* name);
    char gen_lit(LitNode* lit);
    char gen_bin_op(BinOpNode* binOp);
    char gen_subtract(Node* left, Node* right);
    char gen_un_op(UnOpNode* unOp);
    char gen_subscript(SubscriptNode* subNode);
    void gen_assignment(Node* assignTo, char assignAs);

    static constexpr char NUM_GENERAL_REG = 5;
    bool availableReg[NUM_GENERAL_REG];

    FILE* out;

    ProgramNode* curProgram;
    Scope* curScope;

    FunctionNode* curFunction;
    Queue<uint16_t> constantTable;
    char ifCount;
    char whileCount;
};
