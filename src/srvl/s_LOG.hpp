#ifndef _S_LOG_H_
#define _S_LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <cstring>
#include <cctype>
#include <string_view>
#include <type_traits>

/**
  * @name   日志级别定义
  * @note   1：只打印错误日志
  * @note   2：打印错误和警告日志
  * @note   3：打印所有日志
  */
#define LOG_DEBUG 3

void _INFO(const char* format, ...) __attribute__((format(printf, 1, 2)));
void _WARN(const char* format, ...) __attribute__((format(printf, 1, 2)));
void _ERROR(const char* format, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief 波形数据打印宏
 * @note 支持可变参数、任意类型参数、传入值或地址或表达式或函数, 自动打印变量名和值
 * @note 使用示例: _WAVE(&float1, int2, expr1 + expr2, func(a, d))
 */
#define _WAVE(...) WaveLogger::WavePrintf(#__VA_ARGS__, __VA_ARGS__)
namespace WaveLogger
{
    // 辅助函数：查找下一个参数的分隔符，跳过括号 () [] {}
    // 使用 string_view 避免拷贝
    std::string_view extract_next_name(std::string_view& names_view);

    // 核心打印函数
    template<typename... Args>
    void WavePrintf(const char* names_str, Args... args)
    {
        printf("[WAVE] {");

        std::string_view names(names_str);
        size_t index = 0;

        ([&](auto& arg){
            // 打印变量名
            (void)arg;
            if(index > 0) printf(", ");
            std::string_view name = extract_next_name(names);
            printf("%.*s", static_cast<int>(name.length()), name.data());
            index++;
            }(args), ...); // 逗号折叠

        printf("} : ");
        index = 0;

        // 打印数值
        ([&](const auto& arg){
            if(index++ > 0) printf(", ");
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::is_arithmetic_v<T>){
                // 如果是算术类型 (int, float, bool等)，统一转 float 打印
                printf("%.4f", static_cast<float>(arg));
            }
            else{
                // 如果传入了结构体或字符串，打印 NaN
                printf("NaN");
            }
            }(args), ...);

        printf("\r\n");
    }
}

#endif
