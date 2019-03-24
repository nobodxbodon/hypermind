//
// Created by 曹顺 on 2019/2/25.
//

#ifndef HYPERMIND_COMPILER_H
#define HYPERMIND_COMPILER_H

#include "hypermind.h"
#include "ast.h"
#include "obj/function.h"
#include "opcode.h"

// 操作栈改变 影响栈的深度
#define STACK_CHANGE(num) {mStackSlotNum += num;\
if (mStackSlotNum > mFn->maxStackSlotNum) mFn->maxStackSlotNum = mStackSlotNum;}

#define VT_TO_VALUE(VT) {VT, 0}
#define OBJ_TO_VALUE(obj) ({ \
   Value value; \
   value.type = ValueType::Object; \
   value.objval = obj; \
   value; \
})

namespace hypermind {
    class VM;

    enum class ScopeType {
        Invalid,
        Module,  // 模块变量
        Local, // 局部变量
        Upvalue
    };

    struct Upvalue {
        // 是否为外层直接局部变量
        bool isDirectOuterLocalVar;

        //外层函数中局部变量的索引或者外层函数中upvalue的索引
        HMInteger index;
    };  // upvalue结构

    // 变量 表示局部/模块/Upval变量
    struct Variable {
        // 作用域深度
        ScopeType scopeType;
        // 索引
        HMInteger index;
        explicit Variable(ScopeType type, HMInteger index) : scopeType(type), index(index) {};
        Variable() = default;
    };

    // 局部变量
    struct LocalVariable {
        const HMChar *name;
        HMUINT32 length;
        HMInteger scopeDepth;  //局部变量作用域
        bool isUpvalue;
    };

    // 方法签名
    struct MethodSignature {
        enum class SignatureType {
            Method,
            Getter,
            Setter,
            Subscript,
            SubscriptSetter
        };
        MethodSignature(SignatureType type, const HMChar *name, HMUINT32 length, HMInteger argNum) : type(type),
                                                                                                     name(name), length(length), argNum(argNum) {

        };
        MethodSignature() = default;

        SignatureType type;
        const HMChar *name;
        HMUINT32 length;
        HMInteger argNum;

    };

    class CompileUnit {
        friend Compiler;
        friend ASTFunctionStmt;
    protected:
        // 所属虚拟机
        VM *mVM{nullptr};
        // 作用域深度
        HMInteger mScopeDepth{-1};

        Upvalue mUpvalues[MAX_UPVALUE_NUMBER];

        LocalVariable mLocalVariables[MAX_LOCAL_VAR_NUMBER];
        HMUINT32 mLocalVarNumber{0}; // 局部变量个数

        // 最大操作栈数量
        HMUINT32 mStackSlotNum{0};

    public:
        explicit CompileUnit(VM *mVm);

        // 当前正在编译的函数
        HMFunction *mFn;

        // 外层编译单元
        CompileUnit *mOuter{nullptr};

        /**
         * 当前作用域声明局部变量 存在返回索引 不存在添加后返回索引
         * @param id
         * @return 返回索引
         */
        HMInteger AddLocalVariable(const Token &id) {
            HMInteger idx = FindLocal(id);
            // 此时变量已经存在了 返回存在的索引
            if (idx != -1)
                return idx;
            if (mLocalVarNumber >= MAX_LOCAL_VAR_NUMBER) {
                // TODO 错误 变量数目大于最大局部变量
            }
            mLocalVariables[mLocalVarNumber].name = id.start;
            mLocalVariables[mLocalVarNumber].length = id.length;
            mLocalVariables[mLocalVarNumber].scopeDepth = mScopeDepth;
            mLocalVariables[mLocalVarNumber].isUpvalue = false;
            return mLocalVarNumber++;
        };

        /**
         * 根据作用域声明变量/参数
         * @return
         */
        HMInteger AddVariable(const Token &id) {
            if (mScopeDepth == -1) {
                // TODO 模块变量
            }
            return AddLocalVariable(id);
        }

        /**
         * 定义变量   变量设置初始值
         * @param index
         */
        void DefineVariable(HMInteger index) {
            if (mScopeDepth == -1) {
                // 作用域为模块作用域
            }
            // 不是模块作用域 不用管
        }

        /**
         * 查找局部变量
         * @param id
         * @return
         */
        HMInteger FindLocal(const Token &id) {
            for (HMUINT32 i = 0; i < mLocalVarNumber; ++i) {
                if (id.length == mLocalVariables[i].length &&
                    hm_memcmp(mLocalVariables[i].name, id.start, id.length) == 0) {
                    return i;
                }
            }
            return -1;
        }

        // 查找局部变量 或者 Upvalue
        Variable FindLocalOrUpvalue(const Token &id) {
            // 先寻找局部变量
            HMInteger index = FindLocal(id);
            if (index != -1) {
                return Variable(ScopeType::Local, index);
            }

            // 没找到局部变量 查找Upvalue
            index = FindUpvalue(id);
            if (index != -1) {
                return Variable(ScopeType::Upvalue, index);
            }

            return Variable(ScopeType::Invalid, 0);
        }

        /**
         * 查找变量
         * @param id
         * @return
         */
        Variable FindVariable(const Token &id) {
            Variable var = FindLocalOrUpvalue(id);
            if (var.scopeType == ScopeType::Invalid) {
                // TODO 查找模块变量
            }
            return var;
        };

        /**
         * 添加Upvalue
         * @param isDirectOuterLocalVar 是否为直接外层局部变量
         * @param index 索引
         * @return
         */
        HMInteger AddUpvalue(bool isDirectOuterLocalVar, HMInteger index) {
            // 如果存在就返回索引
            for (HMInteger i = 0; i < mFn->upvalueNum; ++i) {
                if (mUpvalues[i].index == index && mUpvalues[i].isDirectOuterLocalVar == isDirectOuterLocalVar) {
                    return i;
                }
            }
            // 不存在就添加
            mUpvalues[mFn->upvalueNum].index = index;
            mUpvalues[mFn->upvalueNum].isDirectOuterLocalVar = isDirectOuterLocalVar;
            return mFn->upvalueNum++;
        };

        HMInteger FindUpvalue(const Token &id) {
            if (mOuter == nullptr) {
                // 未找到
                return -1;
            }
            HMInteger index = mOuter->FindLocal(id);
            if (index != -1) {
                // 找到Upvalue 将局部变量Upvalue
                mOuter->SetLocalVariableUpvalue(index);
                return AddUpvalue(true, index);
            }
            // 递归查询添加
            index = mOuter->FindUpvalue(id);
            if (index != -1) {
                return AddUpvalue(false, index);
            }
            return -1;
        };

        /**
         * 设置变量为Upval
         * @param index
         */
        void SetLocalVariableUpvalue(HMInteger index) {
            mLocalVariables[index].isUpvalue = true;
        };

        // 进入作用域
        void EnterScope() {
            mScopeDepth++;
        };

        // 离开作用域
        void LeaveScope() {
            DiscardLocalVariable();
            mScopeDepth--;
        }

        /**
         * 丢弃作用域内的局部变量
         */
        void DiscardLocalVariable() {

        }

        /**
         * 添加常量
         * @param value
         * @return  索引值
         */
        HMInteger AddConstant(const Value &value) {
            // TODO 现在直接将value存到constants中了
            //  相同的文本或者数值会造成资源重复 可以先取hash
            return mFn->constants.Append(value);
        };

        /**
         * 操作栈中压入Null
         */
        void EmitPushNull() {
            STACK_CHANGE(1);
            mFn->WriteOpcode(Opcode::PushNull);
        };

        /**
         * 栈中压入True
         */
        void EmitPushTrue() {
            STACK_CHANGE(1);
            mFn->WriteOpcode(Opcode::PushTrue);
        };

        /**
         * 栈中压入False
         */
        void EmitPushFalse() {
            STACK_CHANGE(1);
            mFn->WriteOpcode(Opcode::PushFalse);
        };

        /**
         * 弹出栈顶参数
         * @param argNum
         */
        void EmitCall(HMUINT32 methodIndex, HMUINT32 argNum) {
            STACK_CHANGE(argNum);
            if (argNum <= 7)
                mFn->WriteByte(static_cast<HMByte>((HMByte) Opcode::Call + argNum));
            else {
                mFn->WriteOpcode(Opcode::Call);
                mFn->WriteShortOperand(methodIndex); // 方法索引
                mFn->WriteShortOperand(argNum); // 实参数量
            }
        }

        /**
         * 操作栈中压入变量
         * @param var 变量信息(作用域和索引)
         */
        void EmitLoadVariable(const Variable &var) {
            STACK_CHANGE(1);
            switch (var.scopeType) {
                case ScopeType::Module:
                    mFn->WriteOpcode(Opcode::LoadModuleVariable);
                    break;
                case ScopeType::Local:
                    mFn->WriteOpcode(Opcode::LoadLocalVariable);
                    break;
                case ScopeType::Upvalue:
                    mFn->WriteOpcode(Opcode::LoadUpvalue);
                    break;
                case ScopeType::Invalid:
                    // FIXME
                    break;
            }
            mFn->WriteShortOperand(var.index);
        };

        /**
         * 储存变量
         * @param var
         */
        void EmitStoreVariable(const Variable &var) {
            STACK_CHANGE(-1);
            switch (var.scopeType) {
                case ScopeType::Module:
                    mFn->WriteOpcode(Opcode::StoreModuleVariable);
                    break;
                case ScopeType::Local:
                    mFn->WriteOpcode(Opcode::StoreLocalVariable);
                    break;
                case ScopeType::Upvalue:
                    mFn->WriteOpcode(Opcode::StoreUpvalue);
                    break;
                case ScopeType::Invalid:
                    // FIXME
                    break;
            }
            mFn->WriteShortOperand(var.index);
        }

        /**
         * 操作栈中压入常量
         * @param index
         */
        void EmitLoadConstant(HMInteger index) {
            STACK_CHANGE(1);
            mFn->WriteOpcode(Opcode::LoadConstant);
            mFn->WriteShortOperand(index);
        }

        void EmitAdd() {
            STACK_CHANGE(-1);
            mFn->WriteOpcode(Opcode::Add);
        }

        void EmitSub() {
            STACK_CHANGE(-1);
            mFn->WriteOpcode(Opcode::Sub);
        }

        void EmitDiv() {
            STACK_CHANGE(-1);
            mFn->WriteOpcode(Opcode::Div);
        }

        void EmitMul() {
            STACK_CHANGE(-1);
            mFn->WriteOpcode(Opcode::Mul);
        }

        void EmitCreateClosure(HMInteger index) {
            mFn->WriteOpcode(Opcode::CreateClosure);
            mFn->WriteShortOperand(index);
        }

        void EmitEnd() {
            mFn->WriteOpcode(Opcode::End);
        }

    };

    class Compiler {

    private:

    public:
        // 当前虚拟机
        VM *mVM;

        // 当前正在编译的模块
        HMModule *mCurModule;
        // 当前正在编译的函数
        CompileUnit *mCurCompileUnit;

        explicit Compiler(VM *mVM);

#ifdef HMDebug
        CompileUnit CreateCompileUnit(FunctionDebug *debug);
#else
        CompileUnit CreateCompileUnit();
#endif

        void LeaveCompileUnit(const CompileUnit &cu);

    };

}

#endif //HYPERMIND_COMPILER_H
