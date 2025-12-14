#include "s_LOG.hpp"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define LOG_ERROR_LEVEL 1
#define LOG_WARN_LEVEL 2
#define LOG_INFO_LEVEL 3

/**
 * @brief 打印信息日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void _INFO(const char* format, ...)
{
#if (LOG_DEBUG >= LOG_INFO_LEVEL)
    printf(ANSI_COLOR_BLUE "[INFO] ");

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf(ANSI_COLOR_RESET "\r\n");
#endif
}

/**
 * @brief 打印警告日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void _WARN(const char* format, ...)
{
#if (LOG_DEBUG >= LOG_WARN_LEVEL)
    printf(ANSI_COLOR_YELLOW "[WARN] ");

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf(ANSI_COLOR_RESET "\r\n");
#endif
}

/**
 * @brief 打印错误日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void _ERROR(const char* format, ...)
{
#if (LOG_DEBUG >= LOG_ERROR_LEVEL)
    printf(ANSI_COLOR_RED "[ERROR] ");

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf(ANSI_COLOR_RESET "\r\n");
#endif
}

namespace WaveLogger
{
    // 辅助函数：查找下一个参数的分隔符，跳过括号 () [] {}
    // 使用 string_view 避免拷贝
    std::string_view extract_next_name(std::string_view& names_view)
    {
        int depth = 0;
        size_t i = 0;
        for(; i < names_view.size(); ++i){
            char c = names_view[i];
            if(c == '(' || c == '[' || c == '{') depth++;
            else if(c == ')' || c == ']' || c == '}') depth--;
            else if(c == ',' && depth == 0){
                break; // 找到最外层的逗号
            }
        }

        // 截取当前参数名
        std::string_view current_name = names_view.substr(0, i);

        // 去除前导空格
        size_t start = current_name.find_first_not_of(' ');
        if(start != std::string_view::npos) current_name.remove_prefix(start);

        // 调整 names_view 指向下一个参数起点
        if(i < names_view.size()) names_view.remove_prefix(i + 1);
        else names_view.remove_prefix(i);

        return current_name;
    }
}
