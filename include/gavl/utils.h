/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/



#ifndef GAVL_UTILS_H_INCLUDED
#define GAVL_UTILS_H_INCLUDED

#include <gavl/value.h>

/** \defgroup utils Utilities
 *  \brief Utility functions
 *
 *  These are some utility functions, which should be used, whereever
 *  possible, instead of their libc counterparts (if any). That's because
 *  they also handle portability issues, ans it's easier to make one single
 *  function portable (with \#ifdefs if necessary) and use it throughout
 *  the code.
 *
 *  @{
 */

/** \brief Dump to stderr
 *  \param format Format (printf compatible)
 */

GAVL_PUBLIC
void gavl_dprintf(const char * format, ...)
  __attribute__ ((format (printf, 1, 2)));

/** \brief Dump to stderr with intendation
 *  \param indent How many spaces to prepend
 *  \param format Format (printf compatible)
 */

GAVL_PUBLIC
void gavl_diprintf(int indent, const char * format, ...)
  __attribute__ ((format (printf, 2, 3)));

/** \brief Do a hexdump of binary data
 *  \param data Data
 *  \param len Length
 *  \param linebreak How many bytes to print in each line before a linebreak
 *  \param indent How many spaces to prepend
 *
 *  This is mostly for debugging
 */

GAVL_PUBLIC
void gavl_hexdumpi(const uint8_t * data, int len, int linebreak, int indent);

/** \brief Do a hexdump of binary data
 *  \param data Data
 *  \param len Length
 *  \param linebreak How many bytes to print in each line before a linebreak
 *
 *  This is mostly for debugging
 */

GAVL_PUBLIC
void gavl_hexdump(const uint8_t * data, int len, int linebreak);


/** \brief Print into a string
 *  \param format printf like format
 *
 *  All other arguments must match the format like in printf.
 *  This function allocates the returned string, thus it must be
 *  freed by the caller.
 */

GAVL_PUBLIC
char * gavl_sprintf(const char * format,...)
  __attribute__ ((format (printf, 1, 2)));


/** \brief Replace a string
 *  \param old_string Old string (will be freed)
 *  \param new_string How the new string will look like
 *  \returns A copy of new_string
 *
 *  This function handles correctly the cases where either argument
 *  is NULL. The emtpy string is treated like a NULL (non-existant)
 *  string.
 */


GAVL_PUBLIC
char * gavl_strrep(char * old_string,
                   const char * new_string);

/** \brief Replace a string
 *  \param old_string Old string (will be freed)
 *  \param new_string How the new string will look like
 *  \param new_string_end End pointer of the new string
 *  \returns A copy of new_string
 *
 *  Like \ref gavl_strrep but the end of the string is also given
 */


GAVL_PUBLIC
char * gavl_strnrep(char * old_string,
                    const char * new_string_start,
                    const char * new_string_end);

/** \brief Duplicate a string
 *  \param new_string How the new string will look like
 *  \returns A copy of new_string
 *
 *  This function handles correctly the cases where either argument
 *  is NULL. The emtpy string is treated like a NULL (non-existant)
 *  string.
 */

GAVL_PUBLIC
char * gavl_strdup(const char * new_string);

/** \brief Duplicate a string
 *  \param new_string How the new string will look like
 *  \param new_string_end End pointer of the new string
 *  \returns A copy of new_string
 *
 *  Like \ref gavl_strdup but the end of the string is also given
 */


GAVL_PUBLIC
char * gavl_strndup(const char * new_string,
                    const char * new_string_end);

/** \brief Concatenate two strings
 *  \param old Old string (will be freed)
 *  \param tail Will be appended to old_string
 *  \returns A newly allocated string.
 */

GAVL_PUBLIC
char * gavl_strcat(char * old, const char * tail);

/** \brief Append a part of a string to another string
 *  \param old Old string (will be freed)
 *  \param start Start of the string to be appended
 *  \param end Points to the first character after the end of the string to be appended
 *  \returns A newly allocated string.
 */

GAVL_PUBLIC
char * gavl_strncat(char * old, const char * start, const char * end);

/** \brief Cut leading and trailing spaces
 *  \param str The string to trim inplace
 *  \returns str
 * 
 */

GAVL_PUBLIC
char * gavl_strtrim(char * str);

/** \brief Escape a string
 *  \param old Old string (will be freed)
 *  \param escape_chars Characters to escape
 *
 *  This returns a newly allocated string with all characters
 *  occuring in escape_chars preceded by a backslash
 */

GAVL_PUBLIC
char * gavl_escape_string(char * old, const char * escape_chars);

/** \brief Escape a string
 *  \param old Old string
 *  \param escape_chars Characters to escape
 *
 *  This returns the same string with each backslash in
 *  front of one of the escape_chars removed
 */

GAVL_PUBLIC
char * gavl_unescape_string(char * old_string, const char * escape_chars);


/** \brief Check if a string starts with a substring
 *  \param str String
 *  \param start Head to test against
 */

GAVL_PUBLIC
int gavl_string_starts_with(const char * str, const char * start);

/** \brief Check if a string starts with a substring (ignoring case)
 *  \param str String
 *  \param start Head to test against
 */

GAVL_PUBLIC
int gavl_string_starts_with_i(const char * str, const char * start);

/** \brief Check if a string ends with a substring
 *  \param str String
 *  \param end Tail to test against
 */

GAVL_PUBLIC
int gavl_string_ends_with(const char * str, const char * end);

/** \brief Check if a string ends with a substring (ignoring case)
 *  \param str String
 *  \param end Tail to test against
 */

GAVL_PUBLIC
int gavl_string_ends_with_i(const char * str, const char * end);

GAVL_PUBLIC
const char * gavl_detect_episode_tag(const char * filename, const char * end, 
                                     int * season_p, int * idx_p);


GAVL_PUBLIC
char * gavl_strip_space(char * str);

GAVL_PUBLIC
void gavl_strip_space_inplace(char * str);


GAVL_PUBLIC
char ** gavl_strbreak(const char * str, char delim);

GAVL_PUBLIC
void gavl_strbreak_free(char ** retval);

GAVL_PUBLIC
const char * gavl_find_char_c(const char * start, char delim);

GAVL_PUBLIC
char * gavl_find_char(char * start, char delim);



GAVL_PUBLIC
const char * gavl_tempdir();

GAVL_PUBLIC
int gavl_url_split(const char * url,
                   char ** protocol,
                   char ** user,
                   char ** password,
                   char ** hostname,
                   int * port,
                   char ** path);

GAVL_PUBLIC
char * 
gavl_base64_encode_data(const void * data, int length);

GAVL_PUBLIC
int
gavl_base64_decode_data(const char * str, gavl_buffer_t * ret);

GAVL_PUBLIC
char * 
gavl_base64_encode_data_urlsafe(const void * data, int length);

GAVL_PUBLIC
int
gavl_base64_decode_data_urlsafe(const char * str, gavl_buffer_t * ret);

GAVL_PUBLIC
char * gavl_get_absolute_uri(const char * rel_uri, const char * abs_uri);

GAVL_PUBLIC
const char * gavl_language_get_iso639_2_b_from_code(const char * code);

GAVL_PUBLIC
const char * gavl_language_get_iso639_2_b_from_label(const char * label);

GAVL_PUBLIC
const char * gavl_language_get_label_from_code(const char * label);

GAVL_PUBLIC
void gavl_simplify_rational(int * num, int * den);

GAVL_PUBLIC
int gavl_num_cpus();

/* URL variables */

#define GAVL_URL_VAR_TRACK      "track"
#define GAVL_URL_VAR_VARIANT    "variant"

#define GAVL_URL_VAR_CLOCK_TIME "clocktime"

/* URL variables */

GAVL_PUBLIC
void gavl_url_get_vars_c(const char * path,
                         gavl_dictionary_t * vars);

GAVL_PUBLIC
void gavl_url_get_vars(char * path,
                       gavl_dictionary_t * vars);

GAVL_PUBLIC
char * gavl_url_append_vars(char * path,
                            const gavl_dictionary_t * vars);

/* Append url variables */
GAVL_PUBLIC
char * gavl_url_add_var_string(char * uri, const char * var, const char * val);

GAVL_PUBLIC
char * gavl_url_add_var_long(char * uri, const char * var, int64_t val);

GAVL_PUBLIC
char * gavl_url_add_var_double(char * uri, const char * var, double val);

/* Extract url variables */

/* *val = NULL if variable doesn't exist */
GAVL_PUBLIC
char * gavl_url_extract_var_string(char * uri, const char * var, char ** val);

GAVL_PUBLIC
char * gavl_url_extract_var_long(char * uri, const char * var, int64_t * val);

/* *val = -1 if variable doesn't exist */
GAVL_PUBLIC
char * gavl_url_extract_var_double(char * uri, const char * var, double * val);


/* Append/Remove http variables from the URL */

GAVL_PUBLIC
char * gavl_url_append_http_vars(char * url, const gavl_dictionary_t * vars);

GAVL_PUBLIC
char * gavl_url_extract_http_vars(char * url, gavl_dictionary_t * vars);

/* MD5 routines */

#define GAVL_MD5_SIZE 16
#define GAVL_MD5_LENGTH 33 // 32 + \0

GAVL_PUBLIC
void * gavl_md5_buffer(const void *buffer, int len, void *resblock);

GAVL_PUBLIC
char * gavl_md5_buffer_str(const void *buffer, int len, char * ret);

GAVL_PUBLIC
char * gavl_md5_2_string(const void * md5v, char * str);

GAVL_PUBLIC
int gavl_string_2_md5(const char * str, void * md5v);

GAVL_PUBLIC
int gavl_fd_can_read(int fd, int milliseconds);

GAVL_PUBLIC
int gavl_fd_can_write(int fd, int milliseconds);

GAVL_PUBLIC
int gavl_host_is_us(const char * hostname);


GAVL_PUBLIC
int gavl_is_directory(const char * dir, int wr);
/** \brief Ensure that a directory exists
 *  \param dir Directory
 *  \returns 1 if the directory exists after the function call, 0 else
 *
 *  Non-existing directories will be created if possible
 */

GAVL_PUBLIC
int gavl_ensure_directory(const char * dir, int priv);

GAVL_PUBLIC
char * gavl_search_cache_dir(const char * package, const char * directory);

GAVL_PUBLIC
char * gavl_search_config_dir(const char * package, const char * directory);

/* Character set conversion usiong iconv */

typedef struct gavl_charset_converter_s gavl_charset_converter_t;

GAVL_PUBLIC
gavl_charset_converter_t * gavl_charset_converter_create(const char * from, const char * to);

GAVL_PUBLIC
char * gavl_convert_string(gavl_charset_converter_t * cnv,
                           const char * str, int len,
                           int * out_len);

GAVL_PUBLIC
void gavl_charset_converter_destroy(gavl_charset_converter_t * cnv);

#endif // GAVL_UTILS_H_INCLUDED


/* @} */

