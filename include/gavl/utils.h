/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
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
char ** gavl_strbreak(const char * str, char delim);

GAVL_PUBLIC
void gavl_strbreak_free(char ** retval);

GAVL_PUBLIC
const char * gavl_find_char_c(const char * start, char delim);

GAVL_PUBLIC
char * gavl_find_char(char * start, char delim);

/* Buffer */

typedef struct
  {
  uint8_t * buf;
  int len;
  int alloc;
  int alloc_static;
  int pos;
  } gavl_buffer_t;

GAVL_PUBLIC
void gavl_buffer_init(gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_buffer_init_static(gavl_buffer_t * buf, uint8_t * data, int size);

GAVL_PUBLIC
int gavl_buffer_alloc(gavl_buffer_t * buf,
                      int size);

GAVL_PUBLIC
void gavl_buffer_free(gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_buffer_reset(gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_buffer_copy(gavl_buffer_t * dst, const gavl_buffer_t * src);

GAVL_PUBLIC
void gavl_buffer_append(gavl_buffer_t * dst, const gavl_buffer_t * src);

GAVL_PUBLIC
void gavl_buffer_flush(gavl_buffer_t * buf, int len);


GAVL_PUBLIC
const char * gavl_tempdir();

#endif // GAVL_UTILS_H_INCLUDED


/* @} */

