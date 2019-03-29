//
// Created by 曹顺 on 2019/3/1.
//

#include "ast.h"

namespace hypermind {
    // 语法块Dump
    AST_DUMP(ASTBlock) {
        os << _HM_C("  {  ") << std::endl;
        for (auto &stmt : stmts) {
            os << _HM_C("    ");
            stmt->dump(os);
            os << std::endl;
        }
        os << _HM_C(" } ");

    }

    // 二元表达式Dump
    AST_DUMP(ASTBinary) {
        os << _HM_C(" line< ") << line << _HM_C("> ");

        os << _HM_C(" { (binary)  ");
        lhs->dump(os);
        DumpTokenType(os, op);
        rhs->dump(os);
        os << _HM_C("  } ") ;
    }

    // 字面量Dump
    AST_DUMP(ASTLiteral) {
        value.dump(os);
    }

    AST_DUMP(ASTVariable) {
        // 字面量Dump
        var.dump(os);

    }

    AST_DUMP(ASTIfStmt) {
        os << _HM_C("if  condition : ");
        condition->dump(os);
        os << std::endl;
        thenBlock->dump(os);
        if (elseBlock != nullptr) {
            os << _HM_C(" else ");
            elseBlock->dump(os);
        }

    }

    AST_DUMP(ASTWhileStmt) {
        os << _HM_C("while  condition : ");
        condition->dump(os);
        os << std::endl;
        block->dump(os);
    }

    AST_DUMP(ASTContinueStmt) {
        os << _HM_C("continue");
    }

    AST_DUMP(ASTBreakStmt) {
        os << _HM_C("break");
    }
    AST_DUMP(ASTReturnStmt) {
        os << _HM_C("return ");
        retvalue->dump(os);

    }

    AST_DUMP(ASTVarStmt) {
        os << _HM_C(" define var : ") ;
        identifier.dump(os);
        if (value != nullptr) {
            os << _HM_C(" = ");
            value->dump(os);
        }
        os << _HM_C("  ") << std::endl;
    }

    AST_DUMP(ASTParamStmt) {
        os << _HM_C("param : ") ;
        identifier.dump(os);
        if (value != nullptr) {
            os << _HM_C(" = ");
            value->dump(os);
        }
    }

    AST_DUMP(ASTList) {
        os << _HM_C("  ( ");
        for (auto &element : elements) {
            os << _HM_C("   ");
            element->dump(os);
            os << _HM_C(", ");
        }
        os << _HM_C(" ) ");
    }

    AST_DUMP(ASTFunctionStmt) {
        os << _HM_C(" func  name : ");
        name.dump(os);
        params->dump(os);
        body->dump(os);
        os << std::endl;

    }

    AST_DUMP(ASTClassStmt) {

    }

}