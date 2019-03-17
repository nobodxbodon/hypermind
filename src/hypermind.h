//
// Created by 曹顺 on 2019/2/11.
//

#ifndef HYPERMIND_HYPERMIND_H
#define HYPERMIND_HYPERMIND_H

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#define DEBUG
// 最大标识符长度
#define MAX_IDENTIFIER_LENTH 255
// 最大局部变量个数
#define MAX_LOCAL_VAR_NUMBER 128
#define MAX_UPVALUE_NUMBER 128
// 字符宏
#define _HM_C(ch) L##ch
// 字符通用字符类型
typedef wchar_t HMChar;
// 数据类型
typedef char HMByte;
typedef int HMInteger;
typedef uint32_t HMUINT32;
typedef double HMDouble;
typedef float HMFloat;
typedef std::wstring String;
typedef HMUINT32 HMHash;
typedef bool HMBool;
typedef std::wostream Ostream;
#define Vector std::vector

#define HMCout std::wcout
#define hm_memcpy wmemcpy
#define hm_memcmp wmemcmp
namespace hypermind {
    struct HMClass;
    struct HMObject;
}
#define UNUSED __attribute__ ((unused))

#ifdef DEBUG
    #define ASSERT(condition, err) \
        do { \
            if (!(condition)) { \
                fprintf(stderr, "断言失败 !  %s:%d  in %s -> Message: %s", __FILE__, __LINE__, __func__, err); \
            } \
        } while (0);
#else
    #define ASSERT(condition, err) ((void) 0)
#endif

#endif //HYPERMIND_HYPERMIND_H
