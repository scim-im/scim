/** @file scim_utility.h
 *  @brief various utility functions.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_utility.h,v 1.36 2005/04/09 15:38:39 suzhe Exp $
 */

#ifndef __SCIM_UTILITY_H
#define __SCIM_UTILITY_H

namespace scim {
/**
 * @addtogroup Accessories
 * @{
 */

#define SCIM_PATH_DELIM_STRING "/"
#define SCIM_PATH_DELIM        '/'

// UTF-8 <-> ucs4_t convert

/* Return code if invalid. (xxx_mbtowc, xxx_wctomb) */
#define RET_ILSEQ      0
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)  (-1-(n))
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL   -1
/* Replacement character for invalid multibyte sequence or wide character. */
#define BAD_WCHAR ((ucs4_t) 0xfffd)
#define BAD_CHAR '?'

/**
 * @brief Convert an utf8 char sequence to ucs4.
 * 
 * @param pwc destination buffer to store the ucs4 code.
 * @param src source buffer contains the utf8 char sequence.
 * @param src_len the size of source buffer.
 *
 * @return number of chars in s actually converted.
 */ 
int utf8_mbtowc (ucs4_t *pwc, const unsigned char *src, int src_len);

/**
 * @brief Convert an ucs4 code to utf8 char sequence.
 *
 * @param dest destination buffer to store utf8 char sequence.
 * @param wc the ucs4 code to be converted.
 * @param dest_size the size of destination buffer.
 *
 * @return the number of bytes actually written into dest.
 */
int utf8_wctomb (unsigned char *dest, ucs4_t wc, int dest_size);

/**
 * @brief Convert an utf8 string to an ucs4 string.
 *
 * @param str source utf8 string.
 * @return the destination widestring.
 */
WideString utf8_mbstowcs (const String & str);

/**
 * @brief Convert an utf8 string to an ucs4 string.
 *
 * @param str source utf8 string.
 * @param len length of the source string.
 * @return the destination widestring.
 */
WideString utf8_mbstowcs (const char *str, int len = -1);

/**
 * @brief Convert an ucs4 string to an utf8 string.
 *
 * @param wstr source ucs4 string.
 *
 * @return the destination utf8 string.
 */
String utf8_wcstombs (const WideString & wstr);

/**
 * @brief Convert an ucs4 string to an utf8 string.
 *
 * @param wstr source ucs4 string.
 * @param len length of the source string.
 *
 * @return the destination utf8 string.
 */
String utf8_wcstombs (const ucs4_t *wstr, int len = -1);

/**
 * @brief Read a wide char from istream.
 *
 * The content in the istream are actually in utf-8 encoding.
 * 
 * @param is the stream to be read.
 *
 * @return if equal to 0 then got the end of the stream or error occurred.
 */
ucs4_t utf8_read_wchar (std::istream &is);

/**
 * @brief Write a wide char to ostream.
 *
 * The content written into the ostream will be converted into utf-8 encoding.
 *
 * @param os the stream to be written.
 * @param wc the wide char to be written to the stream.
 * @return the same stream object reference.
 */
std::ostream & utf8_write_wchar (std::ostream &os, ucs4_t wc);

/**
 * @brief Read a wide string from istream.
 *
 * The content in the istream are actually in utf-8 encoding.
 *
 * @param is the stream to be read.
 * @param delim the delimiter of the string.
 * @param rm_delim if the delim should be removed from the destination string.
 * @return the wide string read from the given stream.
 */
WideString utf8_read_wstring (std::istream &is, ucs4_t delim = (ucs4_t) '\n', bool rm_delim = true);

/**
 * @brief Write a wide string to ostream.
 *
 * The content written into the ostream will be converted into utf-8 encoding.
 *
 * @param os the stream to be written.
 * @param wstr the wide string to be written into the stream.
 * @return the same stream object reference.
 */
std::ostream & utf8_write_wstring (std::ostream &os, const WideString & wstr);

/**
 * @brief Convert an uint32 variable into a sequence of bytes.
 *
 * @param bytes the buffer to store the result.
 * @param n the variable to be converted.
 */
inline
void scim_uint32tobytes (unsigned char *bytes, uint32 n)
{
    bytes [0] = (unsigned char) ((n & 0xFF));
    bytes [1] = (unsigned char) ((n >> 8) & 0xFF);
    bytes [2] = (unsigned char) ((n >> 16) & 0xFF);
    bytes [3] = (unsigned char) ((n >> 24) & 0xFF);
}

/**
 * @brief Convert a sequence of bytes into an uint32 value.
 *
 * @param bytes the buffer contains the bytes to be converted.
 * @return the result uint32 value.
 */
inline
uint32 scim_bytestouint32 (const unsigned char *bytes)
{
    return  ((uint32) bytes [0])
            | (((uint32) bytes [1]) << 8)
            | (((uint32) bytes [2]) << 16)
            | (((uint32) bytes [3]) << 24);
}

/**
 * @brief Convert an uint16 variable into a sequence of bytes.
 *
 * @param bytes the buffer to store the result.
 * @param n the variable to be converted.
 */
inline
void scim_uint16tobytes (unsigned char *bytes, uint16 n)
{
    bytes [0] = (unsigned char) ((n & 0xFF));
    bytes [1] = (unsigned char) ((n >> 8) & 0xFF);
}

/**
 * @brief Convert a sequence of bytes into an uint16 value.
 *
 * @param bytes the buffer contains the bytes to be converted.
 * @return the result uint16 value.
 */
inline
uint16 scim_bytestouint16 (const unsigned char *bytes)
{
    return  ((uint16) bytes [0]) | (((uint16) bytes [1]) << 8);
}

/**
 * @brief Test if the locale is valid, and return the good locale name.
 *
 * @param locale the locale to be tested.
 * @return If the locale is valid, it's the good locale name, otherwise empty.
 */
String scim_validate_locale (const String& locale);

/**
 * @brief Get the encoding for a locale.
 *
 * @param locale the name of the locale.
 * @return The encoding used by the given locale.
 */
String scim_get_locale_encoding (const String& locale);

/**
 * @brief Get current system locale.
 * @return The current system locale.
 */
String scim_get_current_locale ();

/**
 * @brief Get current system language.
 * @return The current system language.
 */
String scim_get_current_language ();

/**
 * @brief Get the max length of the multibyte char of a locale.
 *
 * @param locale the name of the locale.
 * @return the maxlen of this locale.
 */
int scim_get_locale_maxlen (const String& locale);

/**
 * @brief Split string list into a string vector according to the delim char.
 *
 * @param vec the string vector to store the result.
 * @param str the string to be splitted.
 * @param delim the delimiter to split the strings.
 * @return the number of the strings in the result list.
 */
int scim_split_string_list (std::vector<String>& vec, const String& str, char delim = ',');

/**
 * @brief Combine a string vector into one string list, separated by char delim.
 *
 * @param vec the string vector which contains the strings to be combined.
 * @param delim the delimiter which should be put between two strings.
 * @return the result string.
 */
String scim_combine_string_list (const std::vector<String>& vec, char delim = ',');

/**
 * @brief Get machine endian type
 * @return 1 little endian, 0 big endian
 */
bool scim_is_little_endian ();

/**
 * @brief Test if wchar_t is using UCS4 encoding.
 */
bool scim_if_wchar_ucs4_equal ();

/**
 * @brief Convert a half width unicode char to its full width counterpart. 
 */
ucs4_t scim_wchar_to_full_width (ucs4_t code);

/**
 * @brief Convert a full width unicode char to its half width counterpart.
 */
ucs4_t scim_wchar_to_half_width (ucs4_t code);

/**
 * @brief Get the home dir of current user.
 */
String scim_get_home_dir ();

/**
 * @brief Get the name of current user.
 */
String scim_get_user_name ();

/**
 * @brief Get SCIM data dir of current user.
 */
String scim_get_user_data_dir ();

/**
 * @brief Load a file into memory.
 *
 * @param filename the name of the file to be loaded.
 * @param bufptr the place to store the newly allocated buffer pointer,
 *        if bufptr == NULL then the file is not actually loaded,
 *        just return the file size.
 *        The pointer *bufptr must be deleted afterwards.
 * @return the size of the data actually loaded (mostly, it's the file size),
 *         zero means load failed.
 */
size_t scim_load_file (const String &filename, char **bufptr);

/**
 * @brief Make a directory.
 *
 * @param dir the dir path to be created.
 *
 * @return true if sucess.
 */
bool scim_make_dir (const String &dir);

/**
 * @brief Get the localized name of a language id.
 * @param lang the language id.
 * @return the localized name of this language, in utf8 encoding.
 */
String scim_get_language_name (const String &lang);

/**
 * @brief Get the English name of a language id.
 * @param lang the language id.
 * @return the English name of this language, in utf8 encoding.
 */
String scim_get_language_name_english (const String &lang);

/**
 * @brief Get the untranslated name of a language id.
 * @param lang the language id.
 * @return the untranslated name of this language, in utf8 encoding.
 */
String scim_get_language_name_untranslated (const String &lang);

/**
 * @brief Get the supported locales for a language.
 *
 * For example language zh_CN may support zh_CN.UTF-8, zh_CN.GB18030, zh_CN.GBK, zh_CN.GB2312 locales.
 * 
 * @param lang the language id.
 * @return the supported locales separated by comma.
 */
String scim_get_language_locales (const String &lang);

/**
 * @brief Get the language id for a locale.
 * @param locale the locale name
 * @return the language id for this locale.
 */
String scim_get_locale_language (const String &locale);

/**
 * @brief Test if the language is valid, and return the good language code.
 * @param lang the language to be tested.
 * @return If the language is valid, return the good language id, otherwise return "~other".
 */
String scim_validate_language (const String &lang);

/**
 * @brief Get the normalized language id of a language.
 *
 * Some short language id will be normalized to it's full id, for example:
 * "ja" -> "ja_JP"
 * "ko" -> "ko_KR"
 * "zh" -> "zh_CN"
 *
 * furthermore, zh_HK will be normalized to zh_TW, zh_SG will be normalized to zh_CN.
 *
 * @param lang the original language
 * @return the normalized language code.
 */
String scim_get_normalized_language (const String &lang);

/**
 * @brief Launch a SCIM process with specific options.
 * 
 * @param daemon        If true then launch scim in a daemon process,
 *                      otherwise the current process will be stopped until 
 *                      the newly created process exit.
 * @param config        The Config module to be used.
 * @param imengines     The IMEngines to be loaded, separated by comma.
 * @param frontend      The FrontEnd module to be used.
 * @param argv          Additional arguments passed to the new process's FrontEnd. Must
 *                      terminated by a NULL pointer.
 *
 * @return Return 0 means the process started/exited without any problem, otherwise
 *         means an error occurred.
 */
int  scim_launch (bool          daemon,
                  const String &config,
                  const String &imengines,
                  const String &frontend,
                  char  * const argv [] = 0);

/**
 * @brief Launch a SCIM Panel process with specific options.
 * 
 * @param daemon        If true then launch scim in a daemon process,
 *                      otherwise the current process will be stopped until 
 *                      the newly created process exit.
 * @param config        The Config module to be used.
 * @param display       The display name on which the panel runs.
 *                      eg. for X11 : localhost:0.0
 * @param argv          Additional arguments passed to the new process's FrontEnd. Must
 *                      terminated by a NULL pointer.
 *
 * @return Return 0 means the process started/exited without any problem, otherwise
 *         means an error occurred.
 */
int scim_launch_panel (bool          daemon,
                       const String &config,
                       const String &display,
                       char * const  argv [] = 0);

/**
 * @brief Sleep some microseconds.
 *
 * @param usec The amount of microseconds to be sleeped.
 */
void scim_usleep (unsigned int usec);

/**
 * @brief Switch process into daemon mode.
 */
void scim_daemon ();
        
/** @} */
} // namespace scim

#endif //__SCIM_UTILITY_H
/*
vi:ts=4:nowrap:ai:expandtab
*/
