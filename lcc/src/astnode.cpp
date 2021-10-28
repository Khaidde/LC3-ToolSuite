#include "astnode.hpp"

std::string_view get_ref_name(NameNode* node) {
    switch (node->ref->type) {
        case NodeType::DECL:
            return static_cast<DeclNode*>(node->ref)->varName->str;
        case NodeType::PARAM:
            return static_cast<ParamNode*>(node->ref)->varName->str;
        case NodeType::FUNC:
            return static_cast<FunctionNode*>(node->ref)->functionName->str;
        default:
            fatal("Can' find the reference name of the given node");
    }
}

namespace internal {

constexpr int DUMP_INDENT_LENGTH = 2;

std::string indent_guide(int count) {
    std::string base = "|" + std::string(DUMP_INDENT_LENGTH - 1, ' ');
    std::string res = base;
    for (int i = 0; i < count; i++) {
        res += base;
    }
    return res;
}

std::string node_type_to_str(NodeType type) {
    switch (type) {
        case NodeType::PROGRAM:
            return "Program";
        case NodeType::BLOCK:
            return "Block";
        case NodeType::RETURN:
            return "Return";
        case NodeType::FUNC:
            return "Function";
        case NodeType::IF:
            return "If";
        case NodeType::WHILE:
            return "While";
        case NodeType::TYPE_LIT:
            return "TypeLit";
        case NodeType::DECL:
            return "Decl";
        case NodeType::PARAM:
            return "Param";
        case NodeType::CALL:
            return "Call";
        case NodeType::BIN_OP:
            return "BinOp";
        case NodeType::UN_OP:
            return "UnOp";
        case NodeType::SUBSCRIPT:
            return "Subscript";
        case NodeType::NAME:
            return "Name";
        case NodeType::LIT:
            return "Lit";
        default:
            fatal("Unknown ast node type which can't be converted to string");
    }
}

std::string print_node(const Node& node) {
    std::string dump;
    switch (node.type) {
        case NodeType::TYPE_LIT: {
            auto& typeLit = static_cast<const TypeLitNode&>(node);
            dump += token_type_to_str(typeLit.typeTkn->type);
            if (typeLit.isPtr) {
                dump += "*";
            }
        } break;
        case NodeType::NAME: {
            auto name = static_cast<const NameNode&>(node);
            dump += std::string(get_ref_name(&name));
        } break;
        case NodeType::LIT: {
            auto& lit = static_cast<const LitNode&>(node);
            switch (lit.literal->type) {
                case TokenType::INT_LITERAL:
                    dump += std::to_string((int16_t)lit.literal->num);
                    break;
                case TokenType::STR_LITERAL:
                    dump += std::string(lit.literal->str);
                    break;
                default:
                    fatal("Unknown lit node type which can't be printed");
                    break;
            }
        } break;
        default:
            fatal("Unknown ast node which can't be printed");
            break;
    }
    return dump;
}

std::string recur_dump(const Node& node, int indentCt) {
    std::string dump = indent_guide(indentCt) + node_type_to_str(node.type) + " ";
    switch (node.type) {
        case NodeType::PROGRAM: {
            auto& prgm = static_cast<const ProgramNode&>(node);
            dump += "\n";
            for (auto& statement : prgm.statements) {
                dump += recur_dump(*statement, indentCt + 1) + "\n";
            }
        } break;
        case NodeType::BLOCK: {
            auto& block = static_cast<const BlockNode&>(node);
            if (!block.statements.empty()) {
                dump += "\n";
                for (auto& stmt : block.statements) {
                    dump += recur_dump(*stmt, indentCt + 1) + "\n";
                }
                dump = dump.substr(0, dump.length() - 1);
            } else {
                dump += " {EMPTY}";
            }
        } break;
        case NodeType::RETURN: {
            auto& ret = static_cast<const ReturnNode&>(node);
            dump += "\n";
            dump += recur_dump(*ret.expr, indentCt + 1);
        } break;
        case NodeType::FUNC: {
            auto& func = static_cast<const FunctionNode&>(node);
            dump += func.functionName->str;
            dump += ": ";
            dump += print_node(*func.returnType);
            dump += "\n";
            for (auto& params : func.parameters) {
                dump += recur_dump(*params, indentCt + 1) + "\n";
            }
            dump += recur_dump(*func.block, indentCt + 1);
        } break;
        case NodeType::IF: {
            auto& ifNode = static_cast<const IfNode&>(node);
            dump += "\n";
            dump += recur_dump(*ifNode.condition, indentCt + 1);
            dump += "\n";
            dump += recur_dump(*ifNode.conseq, indentCt + 1);
            if (ifNode.alt) {
                dump += "\n";
                dump += recur_dump(*ifNode.alt, indentCt + 1);
            }
        } break;
        case NodeType::WHILE: {
            auto& whileNode = static_cast<const WhileNode&>(node);
            dump += "\n";
            dump += recur_dump(*whileNode.condition, indentCt + 1);
            dump += "\n";
            dump += recur_dump(*whileNode.loopBlock, indentCt + 1);
        } break;
        case NodeType::DECL: {
            auto& decl = static_cast<const DeclNode&>(node);
            dump += decl.varName->str;
            dump += ": ";
            dump += print_node(*decl.type);
            if (decl.rhs) {
                dump += "\n";
                dump += recur_dump(*decl.rhs, indentCt + 1);
            }
        } break;
        case NodeType::PARAM: {
            auto& param = static_cast<const ParamNode&>(node);
            dump += param.varName->str;
            dump += ": ";
            dump += print_node(*param.type);
        } break;
        case NodeType::BIN_OP: {
            auto& binOp = static_cast<const BinOpNode&>(node);
            dump += token_type_to_str(binOp.op);
            dump += "\n";
            dump += recur_dump(*binOp.left, indentCt + 1);
            dump += "\n";
            dump += recur_dump(*binOp.right, indentCt + 1);
        } break;
        case NodeType::UN_OP: {
            auto& unOp = static_cast<const UnOpNode&>(node);
            dump += token_type_to_str(unOp.op);
            dump += "\n";
            dump += recur_dump(*unOp.inner, indentCt + 1);
        } break;
        case NodeType::SUBSCRIPT: {
            auto& subNode = static_cast<const SubscriptNode&>(node);
            dump += "\n";
            dump += recur_dump(*subNode.pointer, indentCt + 1);
            dump += "\n";
            dump += recur_dump(*subNode.index, indentCt + 1);
        } break;
        case NodeType::CALL: {
            auto& call = static_cast<const CallNode&>(node);
            dump += get_ref_name(call.callRef.get());
            if (call.arguments.size()) {
                dump += "\n";
            }
            for (size_t i = 0; i < call.arguments.size(); i++) {
                dump += recur_dump(*call.arguments[i], indentCt + 1);
                if (i + 1 < call.arguments.size()) {
                    dump += "\n";
                }
            }
        } break;
        case NodeType::NAME:
        case NodeType::LIT: {
            dump += print_node(node);
        } break;
        default:
            fatal("Unknown ast node which can't be dumped");
    }
    return dump;
}
}  // namespace internal

void dump_ast(const Node& node) { printf("%s\n", internal::recur_dump(node, 0).c_str()); }
