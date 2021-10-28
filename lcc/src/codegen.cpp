#include "codegen.hpp"

CodeGen::CodeGen() {
    for (size_t i = 0; i < NUM_GENERAL_REG; i++) {
        availableReg[i] = true;
    }
}

char CodeGen::allocate_reg() {
    for (size_t i = 0; i < NUM_GENERAL_REG; i++) {
        if (availableReg[i]) {
            availableReg[i] = false;
            return i;
        }
    }
    fatal("TODO all registers are allocated");
}

void CodeGen::free_reg(char reg) {
    unsigned char r = (unsigned char)reg;
    if (availableReg[r]) {
        fatal("Can't free already freed register: R%c", r);
    }
    availableReg[r] = true;
}

void CodeGen::gen_program(const char* outFilePath, ProgramNode& program) {
    if (outFilePath) {
        out = fopen(outFilePath, "w");
        if (!out) {
            fatal("Could not open ouput file: %s", outFilePath);
        }
    } else {
        out = stdout;
    }

    curProgram = &program;
    auto main = *program.globalScope->declTable.get("main");
    if (!main || main->type != NodeType::FUNC) {
        fatal("function 'main()' not found");
    }
    auto mainFunction = static_cast<FunctionNode*>(main);
    if (mainFunction->parameters.size()) {
        fatal("function 'main()' expected to have no args in this compiler");
    }
    curScope = program.globalScope;

    fprintf(out, ".orig x3000\n");
    for (auto& statement : program.statements) {
        if (statement->type == NodeType::DECL) {
            auto decl = static_cast<DeclNode*>(statement.get());
            if (decl->rhs && decl->rhs->type == NodeType::LIT) {
                auto lit = static_cast<LitNode*>(decl->rhs.get());
                if (lit->literal->type == TokenType::STR_LITERAL) {
                    fprintf(out, "\tLEA R0, %s_arr\n", std::string(decl->varName->str).c_str());
                    fprintf(out, "\tST R0, %s\n", std::string(decl->varName->str).c_str());
                }
            }
        }
    }

    fprintf(out, "\tLD R6, STACK\n");
    fprintf(out, "\tJSR main\n");
    fprintf(out, "\tLDR R0, R6, 0\n");
    fprintf(out, "\tTRAP x21\n");
    fprintf(out, "\tADD R6, R6, 1\n");  // Push return value off stack
    fprintf(out, "\tHALT\n");
    fprintf(out, "STACK\t.fill xF000\n");  // Push return value off stack
    for (auto& statement : program.statements) {
        switch (statement->type) {
            case NodeType::DECL: {
                auto decl = static_cast<DeclNode*>(statement.get());
                if (decl->rhs) {
                    if (decl->rhs->type != NodeType::LIT) {
                        fatal("Global declaration must be literal: %s\n",
                              std::string(decl->varName->str).c_str());
                    }
                    auto lit = static_cast<LitNode*>(decl->rhs.get());
                    switch (lit->literal->type) {
                        case TokenType::INT_LITERAL:
                            fprintf(out, "%s\t.fill %hi\n", std::string(decl->varName->str).c_str(),
                                    lit->literal->num);
                            break;
                        case TokenType::STR_LITERAL: {
                            fprintf(out, "%s\t.blkw 1\n", std::string(decl->varName->str).c_str(),
                                    lit->literal->num);

                            std::string_view str = std::move(lit->literal->str);
                            fprintf(out, "%s_arr\t.stringz \"%s\"\n",
                                    std::string(decl->varName->str).c_str(),
                                    std::string(str).c_str());
                        } break;
                        default:
                            fatal("TODO unimplemented global declaration rvalue");
                    }
                } else {
                    // TODO change allocation size depending on decl.type
                    fprintf(out, "%s\t.blkw 1\n", std::string(decl->varName->str).c_str());
                }
            } break;
            case NodeType::FUNC: {
                gen_func(static_cast<FunctionNode*>(statement.get()));
            } break;
            default:
                fatal("TODO unimplemented program statement code gen");
        }
    }
    fprintf(out, ".end\n");
}

void CodeGen::gen_block(BlockNode* block) {
    for (auto& statement : block->statements) {
        switch (statement->type) {
            case NodeType::DECL:
                gen_local_decl(static_cast<DeclNode*>(statement.get()));
                break;
            case NodeType::RETURN: {
                auto* ret = static_cast<ReturnNode*>(statement.get());
                char reg = gen_expr(ret->expr.get());
                fprintf(out, "\tSTR R%d, R5, 3\t;; Set return value to value in R%d\n", reg, reg);
                fprintf(out, "\tBR %sEnd\n", std::string(curFunction->functionName->str).c_str());
                free_reg(reg);
            } break;
            case NodeType::IF: {
                gen_if(static_cast<IfNode*>(statement.get()));
            } break;
            case NodeType::WHILE: {
                gen_while(static_cast<WhileNode*>(statement.get()));
            } break;
            default:
                free_reg(gen_expr(statement.get()));
        }
    }
}

void CodeGen::gen_func(FunctionNode* func) {
    curFunction = func;
    curScope = func->block->blockScope;
    ifCount = 0;
    whileCount = 0;

    int numLocals = func->block->blockScope->declTable.size;
    if (numLocals == 0) {
        numLocals = 1;
    }
    std::string funcName(func->functionName->str);
    fprintf(out, "%s\n", funcName.c_str());
    fprintf(out, "\tADD R6, R6, -4\n");
    fprintf(out, "\tSTR R7, R6, 2\n");
    fprintf(out, "\tSTR R5, R6, 1\n");
    fprintf(out, "\tADD R5, R6, 0\n");
    fprintf(out, "\tADD R6, R6, -%d\n", numLocals + 4);
    for (int i = 0; i < 5; i++) {
        fprintf(out, "\tSTR R%d, R6, %d\n", i, 4 - i);
    }

    fprintf(out, "\n\t;; Begin Function\n");

    gen_block(func->block.get());

    fprintf(out, "%sEnd\n", funcName.c_str());
    for (int i = 0; i < 5; i++) {
        fprintf(out, "\tLDR R%d, R6, %d\n", 4 - i, i);
    }
    fprintf(out, "\tADD R6, R5, 0\n");
    fprintf(out, "\tLDR R5, R6, 1\n");
    fprintf(out, "\tLDR R7, R6, 2\n");
    fprintf(out, "\tADD R6, R6, 3\n");
    fprintf(out, "\tRET\n");
    int constantTableCnt = 0;
    while (constantTable.size) {
        fprintf(out, "c_%s_%d\t.fill %hi\n", funcName.c_str(), constantTableCnt,
                constantTable.pop());
        constantTableCnt++;
    }
}

const char* CodeGen::optimize_branch_condition(Node* condition) {
    switch (condition->type) {
        case NodeType::BIN_OP: {
            auto binop = static_cast<BinOpNode*>(condition);
            switch (binop->op) {
                case TokenType::COND_EQUAL: {
                    free_reg(gen_compare(binop->left.get(), binop->right.get()));
                    return "np";
                } break;
                case TokenType::COND_GT: {
                    free_reg(gen_compare(binop->left.get(), binop->right.get()));
                    return "nz";
                } break;
                case TokenType::COND_LT: {
                    free_reg(gen_compare(binop->left.get(), binop->right.get()));
                    return "zp";
                } break;
                case TokenType::COND_GT_EQUAL: {
                    free_reg(gen_compare(binop->left.get(), binop->right.get()));
                    return "n";
                } break;
                case TokenType::COND_LT_EQUAL: {
                    free_reg(gen_compare(binop->left.get(), binop->right.get()));
                    return "p";
                } break;
                default:
                    break;
            }
        } break;
        case NodeType::UN_OP: {
            auto unop = static_cast<UnOpNode*>(condition);
            switch (unop->op) {
                case TokenType::COND_NOT:
                    free_reg(gen_expr(unop->inner.get()));
                    return "np";
                default:
                    break;
            }
        } break;
        default:
            break;
    }
    free_reg(gen_expr(condition));
    return "z";
}

void CodeGen::gen_if(IfNode* ifnode) {
    fprintf(out, "\t;; If statement\n");
    const char* brCond = optimize_branch_condition(ifnode->condition.get());
    if (ifnode->alt) {
        fprintf(out, "\tBR%s elif_%s_%d\n", brCond,
                std::string(curFunction->functionName->str).c_str(), ifCount);
        gen_block(static_cast<BlockNode*>(ifnode->conseq.get()));
        fprintf(out, "\tBR endif_%s_%d\n", std::string(curFunction->functionName->str).c_str(),
                ifCount);
        fprintf(out, "elif_%s_%d\n", std::string(curFunction->functionName->str).c_str(), 0);
        gen_block(static_cast<BlockNode*>(ifnode->alt.get()));
    } else {
        fprintf(out, "\tBR%s endif_%s_%d\n", brCond,
                std::string(curFunction->functionName->str).c_str(), ifCount);
        gen_block(static_cast<BlockNode*>(ifnode->conseq.get()));
    }
    fprintf(out, "endif_%s_%d\n", std::string(curFunction->functionName->str).c_str(), ifCount);
    ifCount++;
    fprintf(out, "\n");
}

void CodeGen::gen_while(WhileNode* whilenode) {
    fprintf(out, "while_%s_%d\n", std::string(curFunction->functionName->str).c_str(), whileCount);
    const char* brCond = optimize_branch_condition(whilenode->condition.get());
    fprintf(out, "\tBR%s endwhile_%s_%d\n", brCond,
            std::string(curFunction->functionName->str).c_str(), whileCount);
    gen_block(static_cast<BlockNode*>(whilenode->loopBlock.get()));
    fprintf(out, "\tBR while_%s_%d\n", std::string(curFunction->functionName->str).c_str(),
            whileCount);
    fprintf(out, "endwhile_%s_%d\n", std::string(curFunction->functionName->str).c_str(),
            whileCount);
    whileCount++;
    fprintf(out, "\n");
}

void CodeGen::gen_local_decl(DeclNode* decl) {
    if (decl->rhs) {
        int rreg = gen_expr(decl->rhs.get());
        if (decl->offset > 0) {
            fprintf(out, "\tSTR R%d, R5, -%d", rreg, decl->offset);
        } else {
            fprintf(out, "\tSTR R%d, R5, %d", rreg, decl->offset);
        }
        fprintf(out, "\t;; Store R%d in '%s'\n", rreg, std::string(decl->varName->str).c_str());
        free_reg(rreg);
        fprintf(out, "\n");
    }
}

char CodeGen::gen_expr(Node* expr) {
    switch (expr->type) {
        case NodeType::LIT:
            return gen_lit(static_cast<LitNode*>(expr));
        case NodeType::NAME:
            return gen_ident(static_cast<NameNode*>(expr));
        case NodeType::BIN_OP:
            return gen_bin_op(static_cast<BinOpNode*>(expr));
        case NodeType::UN_OP:
            return gen_un_op(static_cast<UnOpNode*>(expr));
        case NodeType::CALL:
            return gen_call(static_cast<CallNode*>(expr));
        case NodeType::SUBSCRIPT:
            return gen_subscript(static_cast<SubscriptNode*>(expr));
        default:
            fatal("TODO unimplemented expression generation");
    }
}

char CodeGen::gen_call(CallNode* call) {
    std::string funcName(get_ref_name(call->callRef.get()));
    auto func = curProgram->globalScope->declTable.get(funcName);
    if (!func || (*func)->type != NodeType::FUNC) {
        fatal("Could not find reference function for call: '%s'", funcName.c_str());
    }
    auto funcNode = static_cast<FunctionNode*>(*func);
    if (funcNode->parameters.size() != call->arguments.size()) {
        fatal(
            "Invalid number of arguments to function call: '%s', Expected %d argument(s) but got "
            "%d",
            funcName.c_str(), funcNode->parameters.size(), call->arguments.size());
    }
    fprintf(out, "\t;; Call %s\n", funcName.c_str());
    for (int i = call->arguments.size() - 1; i >= 0; i--) {
        char argReg = gen_expr(call->arguments[(size_t)i].get());
        fprintf(out, "\tADD R6, R6, -1\t;; Push arg %d\n", i);
        fprintf(out, "\tSTR R%d, R6, 0\n", argReg);
        free_reg(argReg);
    }
    fprintf(out, "\tJSR %s\n", funcName.c_str());
    char returnReg = allocate_reg();
    fprintf(out, "\tLDR R%d, R6, 0\t;; Store call value in R%d\n", returnReg, returnReg);
    fprintf(out, "\tADD R6, R6, %d\n", 1 + call->arguments.size());
    return returnReg;
}

namespace internal {
Node* get_decl(Scope* base, std::string_view&& ident) {
    if (!base) {
        fatal("Could not find referenced declaration: %s", std::string(ident).c_str());
    }
    auto varRef = base->declTable.get(ident);
    if (varRef) {
        return *varRef;
    }
    return get_decl(base->parent, std::move(ident));
}
}  // namespace internal

char CodeGen::gen_ident(NameNode* name) {
    std::string identName(get_ref_name(name));
    auto varRef = internal::get_decl(curScope, identName);
    char reg = allocate_reg();
    if (varRef->type == NodeType::DECL) {
        auto decl = static_cast<DeclNode*>(varRef);
        if (decl->isGlobal) {
            fprintf(out, "\tLD R%d, %s", reg, identName.c_str());
        } else if (decl->offset > 0) {
            fprintf(out, "\tLDR R%d, R5, -%d", reg, decl->offset);
        } else {
            fprintf(out, "\tLDR R%d, R5, 0", reg);
        }
    } else if (varRef->type == NodeType::PARAM) {
        auto param = static_cast<ParamNode*>(varRef);
        fprintf(out, "\tLDR R%d, R5, %d", reg, 4 + param->offset);
    }
    fprintf(out, "\t;; Load '%s' into R%d\n", identName.c_str(), reg);
    return reg;
}

char CodeGen::gen_lit(LitNode* lit) {
    switch (lit->literal->type) {
        case TokenType::INT_LITERAL: {
            char reg = allocate_reg();
            if (lit->literal->num == 0) {
                fprintf(out, "\tAND R%d, R%d, 0\t;; Clear R%d\n", reg, reg, reg);
                return reg;
            }
            if ((lit->literal->num & 0x80) == 0x0) {
                if ((lit->literal->num & ~0xF) == 0) {
                    fprintf(out, "\tAND R%d, R%d, 0\t;; Set R%d to %d\n", reg, reg, reg,
                            lit->literal->num);
                    fprintf(out, "\tADD R%d, R%d, %d\n", reg, reg, lit->literal->num);
                    return reg;
                }
            } else if ((lit->literal->num & ~0xF) == 0xFFF0) {
                char val = (-lit->literal->num) & 0x1F;
                fprintf(out, "\tAND R%d, R%d, 0\t;; Set R%d to %d\n", reg, reg, reg, val);
                fprintf(out, "\tADD R%d, R%d, -%d\n", reg, reg, val);
                return reg;
            }
            std::string tempName = "c_" + std::string(curFunction->functionName->str) + "_" +
                                   std::to_string(constantTable.size);
            constantTable.push(lit->literal->num);
            fprintf(out, "\tLD R%d, %s\t;; Load %d into R%d\n", reg, tempName.c_str(),
                    lit->literal->num, reg);
            return reg;
        } break;
        case TokenType::STR_LITERAL: {
            fatal("TODO implement string literal generation");
        } break;
        default:
            fatal("TODO unimplemented literal generation");
    }
}

char CodeGen::gen_bin_op(BinOpNode* binOp) {
    switch (binOp->op) {
        case TokenType::OP_ASSIGN: {
            char rreg = gen_expr(binOp->right.get());

            switch (binOp->left->type) {
                case NodeType::NAME: {
                    auto name = static_cast<NameNode*>(binOp->left.get());
                    std::string identName(get_ref_name(name));
                    auto leftVal = internal::get_decl(curScope, identName);

                    if (leftVal->type == NodeType::DECL) {
                        auto decl = static_cast<DeclNode*>(leftVal);
                        if (decl->isGlobal) {
                            fprintf(out, "\tST R%d, %s", rreg, identName.c_str());
                        } else if (decl->offset > 0) {
                            fprintf(out, "\tSTR R%d, R5, -%d", rreg, decl->offset);
                        } else {
                            fprintf(out, "\tSTR R%d, R5, 0", rreg);
                        }
                    } else if (leftVal->type == NodeType::PARAM) {
                        auto param = static_cast<ParamNode*>(leftVal);
                        fprintf(out, "\tSTR R%d, R5, %d", rreg, 4 + param->offset);
                    }
                    fprintf(out, "\t;; %s = R%d\n", identName.c_str(), rreg);
                } break;
                case NodeType::UN_OP: {
                    auto derefLValue = static_cast<UnOpNode*>(binOp->left.get());
                    char lreg = gen_expr(derefLValue->inner.get());
                    fprintf(out, "\tSTR R%d, R%d, 0\n", rreg, lreg);
                    free_reg(lreg);
                } break;
                case NodeType::SUBSCRIPT: {
                    auto subscriptLValue = static_cast<SubscriptNode*>(binOp->left.get());
                    char pointerReg = gen_expr(subscriptLValue->pointer.get());
                    if (subscriptLValue->index->type == NodeType::LIT) {
                        auto litSubscript = static_cast<LitNode*>(subscriptLValue->index.get());
                        if (litSubscript->literal->type != TokenType::INT_LITERAL) {
                            if (litSubscript->literal->num <= 0xF) {
                                fprintf(out, "\tSTR R%d, R%d, %d\t;; R%d[%d] = R%d\n", rreg,
                                        pointerReg, litSubscript->literal->num, pointerReg,
                                        litSubscript->literal->num, rreg);
                                free_reg(pointerReg);
                                break;
                            } else {
                                fatal("TODO allow max 6-bit offset for array index subscript");
                            }
                        }
                    }
                    char indexReg = gen_expr(subscriptLValue->index.get());
                    fprintf(out, "\tADD R%d, R%d, R%d\t;; Get offset of R%d and store R%d\n",
                            pointerReg, pointerReg, indexReg, pointerReg, rreg);
                    free_reg(indexReg);
                    fprintf(out, "\tSTR R%d, R%d, 0\n", rreg, pointerReg);
                    free_reg(pointerReg);
                } break;
                default:
                    fatal("Unimplemented code gen for assign lvalue");
            }
            return rreg;
        }
        case TokenType::OP_ADD: {
            char lreg = gen_expr(binOp->left.get());
            char rreg = gen_expr(binOp->right.get());
            fprintf(out, "\tADD R%d, R%d, R%d\t;; R%d = R%d + R%d\n", lreg, lreg, rreg, lreg, lreg,
                    rreg);
            free_reg(rreg);
            return lreg;
        }
        case TokenType::COND_EQUAL:
        case TokenType::OP_SUB_NEGATE:
            return gen_compare(binOp->left.get(), binOp->right.get());
        case TokenType::COND_GT: {
            char cmpReg = gen_compare(binOp->left.get(), binOp->right.get());
            fprintf(out, "\tBRp 1\t\t;; If not positive, clear R%d\n", cmpReg);
            fprintf(out, "\tAND R%d, R%d, 0\n", cmpReg, cmpReg);
            return cmpReg;
        }
        case TokenType::COND_LT: {
            char cmpReg = gen_compare(binOp->left.get(), binOp->right.get());
            fprintf(out, "\tBRn 1\t\t;; If not negative, clear R%d\n", cmpReg);
            fprintf(out, "\tAND R%d, R%d, 0\n", cmpReg, cmpReg);
            return cmpReg;
        }
        case TokenType::COND_GT_EQUAL: {
            char cmpReg = gen_compare(binOp->left.get(), binOp->right.get());
            fprintf(out, "\tADD R%d, R%d, 1\t\t;; If not positive, clear R%d\n", cmpReg, cmpReg,
                    cmpReg);
            fprintf(out, "\tBRp 1\n", cmpReg);
            fprintf(out, "\tAND R%d, R%d, 0\n", cmpReg, cmpReg);
            return cmpReg;
        }
        case TokenType::COND_LT_EQUAL: {
            char cmpReg = gen_compare(binOp->left.get(), binOp->right.get());
            fprintf(out, "\tADD R%d, R%d, 1\t\t;; If not negative, clear R%d\n", cmpReg, cmpReg,
                    cmpReg);
            fprintf(out, "\tBRn 1\n", cmpReg);
            fprintf(out, "\tAND R%d, R%d, 0\n", cmpReg, cmpReg);
            return cmpReg;
        }
        case TokenType::BIT_AND: {
            char lreg = gen_expr(binOp->left.get());
            char rreg = gen_expr(binOp->right.get());
            fprintf(out, "\tAND R%d, R%d, R%d\t;; R%d = R%d & R%d\n", lreg, lreg, rreg, lreg, lreg,
                    rreg);
            free_reg(rreg);
            return lreg;
        }
        default:
            fatal("TODO unimplemented binop generation");
    }
}

char CodeGen::gen_compare(Node* left, Node* right) {
    char lreg = gen_expr(left);
    if (right->type == NodeType::LIT) {
        auto lit = static_cast<LitNode*>(right);
        if (lit->literal->type == TokenType::INT_LITERAL && lit->literal->num == 0) {
            return lreg;
        }
    }
    char rreg = gen_expr(right);
    fprintf(out, "\tNOT R%d, R%d\t;; R%d = R%d - R%d\n", rreg, rreg, lreg, lreg, rreg);
    fprintf(out, "\tADD R%d, R%d, 1\n", rreg, rreg);
    fprintf(out, "\tADD R%d, R%d, R%d\n", lreg, lreg, rreg);
    free_reg(rreg);
    return lreg;
}

char CodeGen::gen_un_op(UnOpNode* unOp) {
    switch (unOp->op) {
        case TokenType::ASTERISK: {
            char inner = gen_expr(unOp->inner.get());
            fprintf(out, "\tLDR R%d, R%d, 0\t;; R%d = *R%d\n", inner, inner, inner, inner);
            return inner;
        } break;
        case TokenType::BIT_NOT: {
            char inner = gen_expr(unOp->inner.get());
            fprintf(out, "\tNOT R%d, R%d\t;; R%d = ~R%d\n", inner, inner, inner, inner);
            return inner;
        }
        case TokenType::COND_NOT: {
            fatal("TODO conditional not as expression");
        } break;
        default:
            fatal("TODO unimplemented unop generation");
    };
}

char CodeGen::gen_subscript(SubscriptNode* subNode) {
    char pointerReg = gen_expr(subNode->pointer.get());
    char indexReg = gen_expr(subNode->index.get());
    fprintf(out, "\tADD R%d, R%d, R%d\t;; Load R%d at offset R%d\n", pointerReg, pointerReg,
            indexReg, pointerReg, indexReg);
    free_reg(indexReg);
    fprintf(out, "\tLDR R%d, R%d, 0\n", pointerReg, pointerReg);
    return pointerReg;
}
