/*
    ���������ԡ�
*/

#pragma once


/* ------------ ���ء� ------------ */

/**
 * ��ǲ���ϵͳ���ļ������ʽ�Ƿ�Ϊ�����ʽ��
 */
#define TG_OS_ENCODING_IS_CHINESE_GB true


/* ------------ ���ߡ� ------------ */

/**
 * ��ǲ���ϵͳ�Ƿ�Ϊ΢��Windows��
 */
#if (defined(_WIN32) || defined(_WIN64))
    #define TG_OS_IS_WINDOWS true
#else
    #define TG_OS_IS_WINDOWS false
#endif
