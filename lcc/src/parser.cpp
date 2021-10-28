#include "parser.hpp"

namespace internal {
enum { LOWEST_PRECEDENCE = 0, HIGHEST_PRECEDENCE = 100 };

int get_precedence(TokenType type) {
    switch (type) {
        case TokenType::OP_ASSIGN:
            return 1;

        case TokenType::BIT_AND:
            return 2;

        case TokenType::COND_EQUAL:
            return 3;

        case TokenType::COND_GT:
        case TokenType::COND_LT:
        case TokenType::COND_GT_EQUAL:
        case TokenType::COND_LT_EQUAL:
            return 4;

        case TokenType::OP_ADD:
        case TokenType::OP_SUB_NEGATE:
            return 5;

        case TokenType::L_SUBSCRIPT:
        case TokenType::L_PARENS:  // function calls have highest precedence
            return 6;
        default:
            return LOWEST_PRECEDENCE;
    }
}
}  // namespace internal

void Parser::load_source(std::string&& source) { lexer.load_source(std::move(source)); }

void Parser::error(Token& token, std::string&& msg) {
    fatal(("(l:%d,c:%d): " + msg).c_str(), token.line, token.col);
}

Scope* Parser::make_scope(Scope* parent) {
    Scope* scope = new Scope();
    scope->parent = parent;
    curProgram->scopeQueue.push(scope);
    return scope;
}

namespace internal {
Node* get_decl(Scope* base, std::string_view& ident) {
    if (!base) {
        return nullptr;
    }
    auto decl = base->declTable.get(ident);
    if (decl) {
        return *decl;
    }
    return get_decl(base->parent, ident);
}
}  // namespace internal

void Parser::put_decl(Scope* base, DeclNode* decl) {
    decl->offset = base->declTable.size;
    if (internal::get_decl(base, decl->varName->str)) {
        error(*decl->varName, "Can't redefine variable '" + std::string(decl->varName->str) + "'");
    }
    base->declTable.put(decl->varName->str, decl);
}

void Parser::assert_semi_colon() {
    if (lexer.peek_token()->type == TokenType::SEMI_COLON) {
        lexer.eat_token();  // Eat ;
    } else {
        error(*lexer.peek_token(), "Expected a semi colon at the end of statement");
    }
}

std::unique_ptr<ProgramNode> Parser::parse_program() {
    auto program = std::make_unique<ProgramNode>();
    curProgram = program.get();
    curScope = make_scope(nullptr);
    program->globalScope = curScope;

    while (lexer.peek_token()->type != TokenType::END) {
        auto type = parse_type();
        if (lexer.peek_token()->type != TokenType::IDENTIFIER) {
            error(*lexer.peek_token(), "Expected an identifier");
        }
        auto identifier = lexer.eat_token();  // Eat identifier

        switch (lexer.peek_token()->type) {
            case TokenType::SEMI_COLON:
            case TokenType::OP_ASSIGN: {
                auto decl = parse_decl(std::move(type), std::move(identifier));
                decl->isGlobal = true;
                program->statements.push_back(std::move(decl));
                assert_semi_colon();
            } break;
            case TokenType::L_PARENS: {
                program->statements.push_back(
                    parse_function(std::move(type), std::move(identifier)));
            } break;
            default:
                error(*lexer.peek_token(), "Expected either = or (");
        }
    }
    return program;
}

std::unique_ptr<BlockNode> Parser::parse_block() {
    auto block = std::make_unique<BlockNode>();
    block->blockScope = curScope;

    if (lexer.peek_token()->type != TokenType::L_CURLY) {
        error(*lexer.peek_token(), "Expected { to start a block");
    }
    lexer.eat_token();  // Eat {

    while (lexer.peek_token()->type != TokenType::END) {
        switch (lexer.peek_token()->type) {
            case TokenType::R_CURLY:
                lexer.eat_token();
                return block;
            case TokenType::INT: {
                auto type = parse_type();
                auto name = lexer.eat_token();
                if (name->type != TokenType::IDENTIFIER) {
                    error(*name, "Expected a name for local variable declaration");
                }
                auto decl = parse_decl(std::move(type), std::move(name));
                decl->isGlobal = false;
                block->statements.push_back(std::move(decl));
                assert_semi_colon();
            } break;
            case TokenType::L_CURLY: {
                Scope* parent = curScope;
                curScope = make_scope(parent);
                block->statements.push_back(parse_block());
                curScope = parent;
            } break;
            case TokenType::RETURN: {
                auto ret = std::make_unique<ReturnNode>();
                lexer.eat_token();  // Eat return
                ret->expr = parse_expr();
                block->statements.push_back(std::move(ret));
                assert_semi_colon();
            } break;
            case TokenType::IF: {
                block->statements.push_back(parse_if());
            } break;
            case TokenType::WHILE: {
                block->statements.push_back(parse_while());
            } break;
            default:
                block->statements.push_back(parse_expr());
                if (lexer.peek_token()->type != TokenType::SEMI_COLON) {
                    error(*lexer.peek_token(), "Expected a semi-colon");
                }
                lexer.eat_token();  // eat ;
                break;
        }
    }
    error(*lexer.peek_token(), "Reached end of file before finishing block");
}

std::unique_ptr<FunctionNode> Parser::parse_function(std::unique_ptr<TypeLitNode>&& returnType,
                                                     std::unique_ptr<Token>&& name) {
    auto func = std::make_unique<FunctionNode>();
    func->returnType = std::move(returnType);
    func->functionName = std::move(name);

    auto dup = curScope->declTable.put(func->functionName->str, func.get());
    if (dup) {
        std::string funcName(func->functionName->str);
        if (dup->data->type == NodeType::FUNC) {
            error(
                *func->functionName,
                "Function overloading not supported. Duplicate function name: '" + funcName + "'");
        } else {
            error(*func->functionName, "Redefinition of '" + funcName + "' as a different symbol");
        }
    }

    Scope* parent = curScope;
    curScope = make_scope(parent);

    lexer.eat_token();  // Eat (
    bool lastDecl = false;
    int paramCount = 0;
    while (lexer.peek_token()->type != TokenType::END) {
        switch (lexer.peek_token()->type) {
            case TokenType::R_PARENS:
                lexer.eat_token();  // Eat )
                func->block = parse_block();
                curScope = parent;
                return func;
            case TokenType::INT: {
                if (lastDecl) {
                    error(*lexer.peek_token(),
                          "Expected a comma to separate the parameters in the function parameter "
                          "list");
                }
                lastDecl = true;
                auto param = std::make_unique<ParamNode>();
                param->type = parse_type();
                param->varName = lexer.eat_token();
                param->offset = paramCount++;
                if (param->varName->type != TokenType::IDENTIFIER) {
                    error(*param->varName, "Expected a name for function parameter");
                }
                if (curScope->declTable.put(param->varName->str, param.get())) {
                    error(*param->varName,
                          "Can't redefine variable '" + std::string(param->varName->str) + "'");
                }
                func->parameters.push_back(std::move(param));
            } break;
            case TokenType::COMMA:
                if (!lastDecl) {
                    error(*lexer.peek_token(), "Expected a function parameter after the comma");
                }
                lastDecl = false;
                lexer.eat_token();  // Eat ,
                break;
            default:
                error(*lexer.peek_token(), "Unknown token in function parameter list");
                break;
        }
    }
    error(*lexer.peek_token(), "Reached end of file before finishing function parameter list");
}

std::unique_ptr<IfNode> Parser::parse_if() {
    auto ifNode = std::make_unique<IfNode>();
    lexer.eat_token();  // Eat if
    if (lexer.peek_token()->type != TokenType::L_PARENS) {
        error(*lexer.peek_token(), "Expected a left parenthesis after if token");
    }
    ifNode->condition = parse_expr();

    Scope* parent = curScope;
    curScope = make_scope(parent);
    ifNode->conseq = parse_block();
    curScope = parent;

    if (lexer.peek_token()->type == TokenType::ELSE) {
        lexer.eat_token();  // Eat else
        switch (lexer.peek_token()->type) {
            case TokenType::IF:
                ifNode->alt = parse_if();
                break;
            case TokenType::L_CURLY: {
                Scope* parent = curScope;
                curScope = make_scope(parent);
                ifNode->alt = parse_block();
                curScope = parent;
            } break;
            default:
                error(*lexer.peek_token(), "Expected either an if or {");
        }
    }
    return ifNode;
}

std::unique_ptr<WhileNode> Parser::parse_while() {
    auto whileNode = std::make_unique<WhileNode>();
    lexer.eat_token();  // Eat while
    if (lexer.peek_token()->type != TokenType::L_PARENS) {
        error(*lexer.peek_token(), "Expected a left parenthesis after while token");
    }
    whileNode->condition = parse_expr();

    Scope* parent = curScope;
    curScope = make_scope(parent);
    whileNode->loopBlock = parse_block();
    curScope = parent;
    return whileNode;
}

std::unique_ptr<DeclNode> Parser::parse_decl(std::unique_ptr<TypeLitNode>&& type,
                                             std::unique_ptr<Token>&& name) {
    auto decl = std::make_unique<DeclNode>();
    decl->type = std::move(type);

    decl->varName = std::move(name);

    if (lexer.peek_token()->type == TokenType::OP_ASSIGN) {
        lexer.eat_token();  // Eat =
        decl->rhs = parse_expr();
    } else {
        decl->rhs = nullptr;
    }
    put_decl(curScope, decl.get());
    return decl;
}

std::unique_ptr<TypeLitNode> Parser::parse_type() {
    auto typeTkn = lexer.eat_token();
    switch (typeTkn->type) {
        case TokenType::INT:
            break;
        default:
            error(*typeTkn, "Expected a type");
    }
    auto typeNode = std::make_unique<TypeLitNode>();
    typeNode->typeTkn = std::move(typeTkn);
    if (lexer.peek_token()->type == TokenType::ASTERISK) {
        lexer.eat_token();  // Eat *
        typeNode->isPtr = true;
    } else {
        typeNode->isPtr = false;
    }
    return typeNode;
}

std::unique_ptr<CallNode> Parser::parse_call(std::unique_ptr<Node>&& callRef, char exprDepth) {
    auto call = std::make_unique<CallNode>();
    if (callRef->type != NodeType::NAME) {
        error(*lexer.peek_token(),
              "Function call must be made on a function name, not an expression");
    }
    call->callRef = std::unique_ptr<NameNode>(static_cast<NameNode*>(callRef.release()));
    if (call->callRef->ref->type != NodeType::FUNC) {
        error(*lexer.peek_token(), "'" + std::string(get_ref_name(call->callRef.get())) +
                                       "' does not reference a function and can't be called");
    }

    lexer.eat_token();  // Eat (
    bool lastArg = false;
    while (lexer.peek_token()->type != TokenType::END) {
        if (lexer.peek_token()->type == TokenType::R_PARENS) {
            lexer.eat_token();  // Eat )
            return call;
        }
        if (lexer.peek_token()->type == TokenType::COMMA) {
            if (!lastArg) {
                error(*lexer.peek_token(), "Expected an argument after the comma");
            }
            lastArg = false;
            lexer.eat_token();  // Eat ,
        } else {
            if (lastArg) {
                error(*lexer.peek_token(), "Expected an comma after the argument");
            }
            lastArg = true;
            call->arguments.push_back(recur_expr(internal::LOWEST_PRECEDENCE, exprDepth + 1));
        }
    }
    error(*lexer.peek_token(), "Reached end of file before finishing call argument list");
}

std::unique_ptr<Node> Parser::parse_expr() { return recur_expr(internal::LOWEST_PRECEDENCE, 0); }

std::unique_ptr<Node> Parser::parse_operand(char exprDepth) {
    switch (lexer.peek_token()->type) {
        case TokenType::IDENTIFIER: {
            auto name = std::make_unique<NameNode>();
            auto refDecl = internal::get_decl(curScope, lexer.peek_token()->str);
            auto ident = lexer.eat_token();
            if (!refDecl) {
                error(*ident, "Reference to undefined symbol: " + std::string(ident->str));
            }
            name->ref = refDecl;
            return name;
        }
        case TokenType::INT_LITERAL:
        case TokenType::STR_LITERAL: {
            auto lit = std::make_unique<LitNode>();
            lit->literal = lexer.eat_token();
            return lit;
        }
        case TokenType::L_PARENS: {
            lexer.eat_token();  // Eat (
            auto expr = recur_expr(internal::LOWEST_PRECEDENCE, exprDepth + 1);
            if (lexer.peek_token()->type != TokenType::R_PARENS) {
                error(*lexer.peek_token(), std::to_string(exprDepth + 1) +
                                               " unclosed parenthesis here. Add right parenthesis");
            }
            lexer.eat_token();  // Eat )
            return expr;
        }
        case TokenType::ASTERISK:
        case TokenType::BIT_NOT:
        case TokenType::COND_NOT: {
            auto unOp = std::make_unique<UnOpNode>();
            unOp->op = lexer.eat_token()->type;
            unOp->inner = recur_expr(internal::HIGHEST_PRECEDENCE, exprDepth);
            return unOp;
        }
        case TokenType::END:
            error(*lexer.peek_token(), "Reached end of file before finding operand");
        default:
            if (lexer.peek_token()->type == TokenType::R_PARENS && exprDepth == 0) {
                error(*lexer.peek_token(), "Extra right parenthesis");
            }
            error(*lexer.peek_token(), "Expected an operand");
    }
}

std::unique_ptr<Node> Parser::recur_expr(char prec, char exprDepth) {
    auto expr = parse_operand(exprDepth);

    int peekedPrec = internal::get_precedence(lexer.peek_token()->type);
    if (lexer.peek_token()->type == TokenType::R_PARENS && exprDepth == 0) {
        error(*lexer.peek_token(), "Extra right parenthesis");
    }
    while (peekedPrec > prec) {
        switch (lexer.peek_token()->type) {
            case TokenType::OP_ASSIGN: {
                auto assignOp = std::make_unique<BinOpNode>();
                assignOp->left = std::move(expr);
                if (assignOp->left->type != NodeType::NAME) {
                    if (assignOp->left->type != NodeType::UN_OP) {
                        if (assignOp->left->type != NodeType::SUBSCRIPT) {
                            error(*lexer.peek_token(),
                                  "Left side of assignment expected to be an identifier");
                        }
                    } else {
                        auto unop = static_cast<UnOpNode*>(assignOp->left.get());
                        if (unop->op != TokenType::ASTERISK) {
                            error(*lexer.peek_token(),
                                  "Left side of assignment expected to be an lvalue");
                        }
                    }
                }
                assignOp->op = lexer.eat_token()->type;
                assignOp->right = recur_expr(0, exprDepth);
                return assignOp;
            }
            case TokenType::L_PARENS: {
                expr = parse_call(std::move(expr), exprDepth);
            } break;
            case TokenType::L_SUBSCRIPT: {
                lexer.eat_token();  // Eat [
                auto subNode = std::make_unique<SubscriptNode>();
                subNode->pointer = std::move(expr);
                subNode->index = recur_expr(internal::LOWEST_PRECEDENCE, exprDepth);
                if (lexer.peek_token()->type != TokenType::R_SUBSCRIPT) {
                    error(*lexer.peek_token(), "Expected ] to proceed [ in subscript");
                }
                lexer.eat_token();  // Eat ]
                expr = std::move(subNode);
                break;
            }
            default: {
                auto binOp = std::make_unique<BinOpNode>();
                binOp->left = std::move(expr);
                binOp->op = lexer.eat_token()->type;
                binOp->right = recur_expr(peekedPrec, exprDepth);
                expr = std::move(binOp);
                break;
            }
        }
        peekedPrec = internal::get_precedence(lexer.peek_token()->type);
    }
    return expr;
}
