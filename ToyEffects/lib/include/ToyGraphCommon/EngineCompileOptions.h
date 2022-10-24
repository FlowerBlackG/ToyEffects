/*
    编译期属性。
*/

#pragma once


/* ------------ 开关。 ------------ */

/**
 * 标记操作系统的文件编码格式是否为国标格式。
 */
#define TG_OS_ENCODING_IS_CHINESE_GB true


/* ------------ 工具。 ------------ */

/**
 * 标记操作系统是否为微软Windows。
 */
#if (defined(_WIN32) || defined(_WIN64))
    #define TG_OS_IS_WINDOWS true
#else
    #define TG_OS_IS_WINDOWS false
#endif
