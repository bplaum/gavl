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

#ifndef GAVL_CHAPTERLIST_H_INCLUDED
#define GAVL_CHAPTERLIST_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <gavl/gavldefs.h>
#include <gavl/value.h>

/** \defgroup chapterlist Chapter list
 *  \brief Chapter list
 *
 *  Chapters in gavl are simply 
 *  seekpoints with (optionally) associated names.
 *
 *  Since 1.5.0
 *
 * @{
 */

/** \brief Chapter list
 *
 */

#if 0  
typedef struct
  {
  uint32_t num_chapters;       //!< Number of chapters
  uint32_t timescale;          //!< Scale of the timestamps
  struct
    {
    int64_t time;         //!< Start time (seekpoint) of this chapter
    char * name;          //!< Name for this chapter (or NULL if unavailable)
    } * chapters;         //!< Chapters
  } gavl_chapter_list_t;
#else

#define GAVL_CHAPTERLIST_CHAPTERLIST  "chapterlist"
#define GAVL_CHAPTERLIST_CHAPTERS     "chap"
#define GAVL_CHAPTERLIST_TIME         "time"
#define GAVL_CHAPTERLIST_TIMESCALE    "timescale"

typedef gavl_dictionary_t gavl_chapter_list_t;


#endif
 
  
/** \brief Insert a chapter into a chapter list
 *  \param list A chapter list
 *  \param index Position (starting with 0) where the new chapter will be placed
 *  \param time Start time of the chapter
 *  \param name Chapter name (or NULL)
 */

GAVL_PUBLIC
gavl_dictionary_t * gavl_chapter_list_insert(gavl_chapter_list_t * list, int index,
                                             int64_t time, const char * name);

/** \brief Delete a chapter from a chapter list
 *  \param list A chapter list
 *  \param index Position (starting with 0) of the chapter to delete
 */

GAVL_PUBLIC
void gavl_chapter_list_delete(gavl_chapter_list_t * list, int index);

/** \brief Get current chapter
 *  \param list A chapter list
 *  \param time Playback time
 *  \returns The current chapter index
 *
 *  Use this function after seeking to signal a
 *  chapter change
 */

GAVL_PUBLIC
int gavl_chapter_list_get_current(const gavl_chapter_list_t * list,
                                  gavl_time_t time);

/* Check if the list is valid at all */  

GAVL_PUBLIC
int gavl_chapter_list_is_valid(const gavl_chapter_list_t * list);

GAVL_PUBLIC
void gavl_chapter_list_set_timescale(gavl_chapter_list_t * list, int timescale);

GAVL_PUBLIC
int gavl_chapter_list_get_timescale(const gavl_chapter_list_t * list);

GAVL_PUBLIC
int gavl_chapter_list_get_num(const gavl_chapter_list_t * list);

GAVL_PUBLIC
gavl_dictionary_t * gavl_chapter_list_get_nc(gavl_chapter_list_t * list, int idx);
  
GAVL_PUBLIC
const gavl_dictionary_t * gavl_chapter_list_get(const gavl_chapter_list_t * list, int idx);

GAVL_PUBLIC
int64_t gavl_chapter_list_get_time(const gavl_chapter_list_t * list, int idx);

GAVL_PUBLIC
const char * gavl_chapter_list_get_label(const gavl_chapter_list_t * list, int idx);

GAVL_PUBLIC
gavl_dictionary_t *
gavl_dictionary_add_chapter_list(gavl_dictionary_t * m, int timescale);
  
GAVL_PUBLIC
gavl_dictionary_t *
gavl_dictionary_get_chapter_list_nc(gavl_dictionary_t * m);

GAVL_PUBLIC
const gavl_dictionary_t *
gavl_dictionary_get_chapter_list(const gavl_dictionary_t * m);

  
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // GAVL_CHAPTERLIST_H_INCLUDED
