#pragma once

#include <memory>
#include <vector>

#include "general.hpp"
#include "lexer.hpp"
#include "queue.hpp"
#include "symtable.hpp"

enum class NodeType {
    PROGRAM,
    BLOCK,
    RETURN,
    DECL,
    PARAM,
    FUNC,

    IF,
    WHILE,

    TYPE_LIT,

    CALL,
    BIN_OP,
    UN_OP,
    SUBSCRIPT,
    NAME,
    LIT,
};

struct Node {
    NodeType type;
    Node(NodeType type) : type(type) {}
};

struct Scope {
    Scope* parent;
    SymTable<Node*> declTable;
};

struct TypeLitNode;
struct NameNode;
struct FunctionNode;

struct ProgramNode : Node {
    Scope* globalScope;
    std::vector<std::unique_ptr<Node>> statements;

    Queue<Scope*> scopeQueue;

    ProgramNode() : Node(NodeType::PROGRAM) {}

    ~ProgramNode() {
        printf("deleting %d scopes...\n", scopeQueue.size);
        while (scopeQueue.size) {
            delete scopeQueue.pop();
        }
    }
};

struct BlockNode : Node {
    Scope* blockScope;
    std::vector<std::unique_ptr<Node>> statements;
    BlockNode() : Node(NodeType::BLOCK) {}
};

struct ReturnNode : Node {
    std::unique_ptr<Node> expr;
    ReturnNode() : Node(NodeType::RETURN) {}
};

struct DeclNode : Node {
    std::unique_ptr<TypeLitNode> type;
    std::unique_ptr<Token> varName;  // identifier

    char offset;  // offset in memory
    bool isGlobal;

    std::unique_ptr<Node> rhs;
    DeclNode() : Node(NodeType::DECL) {}
};

struct ParamNode : Node {
    std::unique_ptr<TypeLitNode> type;
    std::unique_ptr<Token> varName;  // identifier
    char offset;                     // param order
    ParamNode() : Node(NodeType::PARAM) {}
};

struct FunctionNode : Node {
    std::unique_ptr<TypeLitNode> returnType;
    std::unique_ptr<Token> functionName;                 // identifier
    std::vector<std::unique_ptr<ParamNode>> parameters;  // vector of declarations

    std::unique_ptr<BlockNode> block;
    FunctionNode() : Node(NodeType::FUNC) {}
};

struct IfNode : Node {
    std::unique_ptr<Node> condition;
    std::unique_ptr<BlockNode> conseq;
    std::unique_ptr<Node> alt;
    IfNode() : Node(NodeType::IF) {}
};

struct WhileNode : Node {
    std::unique_ptr<Node> condition;
    std::unique_ptr<BlockNode> loopBlock;
    WhileNode() : Node(NodeType::WHILE) {}
};

struct TypeLitNode : Node {
    bool isPtr;
    std::unique_ptr<Token> typeTkn;
    TypeLitNode() : Node(NodeType::TYPE_LIT) {}
};

struct CallNode : Node {
    std::unique_ptr<NameNode> callRef;
    std::vector<std::unique_ptr<Node>> arguments;
    CallNode() : Node(NodeType::CALL) {}
};

struct BinOpNode : Node {
    std::unique_ptr<Node> left;
    TokenType op;
    std::unique_ptr<Node> right;
    BinOpNode() : Node(NodeType::BIN_OP) {}
};

struct UnOpNode : Node {
    TokenType op;
    std::unique_ptr<Node> inner;
    bool isPost;
    UnOpNode() : Node(NodeType::UN_OP) {}
};

struct SubscriptNode : Node {
    std::unique_ptr<Node> pointer;
    std::unique_ptr<Node> index;
    SubscriptNode() : Node(NodeType::SUBSCRIPT) {}
};

struct NameNode : Node {
    Node* ref;
    NameNode() : Node(NodeType::NAME) {}
};

struct LitNode : Node {
    std::unique_ptr<Token> literal;
    LitNode() : Node(NodeType::LIT) {}
};

std::string_view get_ref_name(NameNode* node);

void dump_ast(const Node& node);
