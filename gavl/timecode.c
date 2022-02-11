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

#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <gavl.h>

#define BITMASK(nbits,shift) (((1LL<<nbits)-1)<<shift)

#define TC_FRAME_BITS    10
#define TC_FRAME_SHIFT    0
#define TC_FRAME_MASK  BITMASK(TC_FRAME_BITS, TC_FRAME_SHIFT)

#define TC_SECOND_BITS     6
#define TC_SECOND_SHIFT   10
#define TC_SECOND_MASK  BITMASK(TC_SECOND_BITS, TC_SECOND_SHIFT)

#define TC_MINUTE_BITS     6
#define TC_MINUTE_SHIFT   16
#define TC_MINUTE_MASK  BITMASK(TC_MINUTE_BITS, TC_MINUTE_SHIFT)

#define TC_HOUR_BITS       5
#define TC_HOUR_SHIFT     22
#define TC_HOUR_MASK  BITMASK(TC_HOUR_BITS, TC_HOUR_SHIFT)

#define TC_DAY_BITS        5
#define TC_DAY_SHIFT      27
#define TC_DAY_MASK  BITMASK(TC_DAY_BITS, TC_DAY_SHIFT)

#define TC_MONTH_BITS      4
#define TC_MONTH_SHIFT    32
#define TC_MONTH_MASK  BITMASK(TC_MONTH_BITS, TC_MONTH_SHIFT)

/* Year will overflow in more than 67 Million years */

#define TC_YEAR_BITS      26
#define TC_YEAR_SHIFT     36
#define TC_YEAR_MASK  BITMASK(TC_YEAR_BITS, TC_YEAR_SHIFT)

#ifdef _WIN32
/*  windows localtime returns the results in a per-thread buffer */  
struct tm *
gavl_localtime_r (const time_t *timer, struct tm *result)
{
   struct tm *local_result;
   local_result = localtime (timer);

   if (local_result == NULL || result == NULL)
     return NULL;

   memcpy (result, local_result, sizeof (result));
   return result;
} 
#else
#define gavl_localtime_r localtime_r
#endif

void gavl_timecode_to_hmsf(gavl_timecode_t tc,
                           int * hours,
                           int * minutes,
                           int * seconds,
                           int * frames)
  {
  if(hours)
    *hours = (tc & TC_HOUR_MASK) >> TC_HOUR_SHIFT;
  if(minutes)
    *minutes = (tc & TC_MINUTE_MASK) >> TC_MINUTE_SHIFT;
  if(seconds)
    *seconds = (tc & TC_SECOND_MASK) >> TC_SECOND_SHIFT;
  if(frames)
    *frames = (tc & TC_FRAME_MASK) >> TC_FRAME_SHIFT;
  }

/** \brief Extract the date part of the timecode
 *  \param tc A timecode
 *  \param year If non NULL, returns the year
 *  \param month If non NULL, returns the month
 *  \param day If non NULL, returns the day
 */
  
void gavl_timecode_to_ymd(gavl_timecode_t tc,
                          int * year,
                          int * month,
                          int * day)
  {
  if(year)
    *year = (tc & TC_YEAR_MASK) >> TC_YEAR_SHIFT;
  if(month)
    *month = (tc & TC_MONTH_MASK) >> TC_MONTH_SHIFT;
  if(day)
    *day = (tc & TC_DAY_MASK) >> TC_DAY_SHIFT;
  }

/** \brief Set the time part of the timecode
 *  \param tc A timecode
 *  \param hours The hours
 *  \param minutes The minutes
 *  \param seconds The seconds
 *  \param frames The frames
 */
  
void gavl_timecode_from_hmsf(gavl_timecode_t * tc,
                            int hours,
                            int minutes,
                            int seconds,
                            int frames)
  {
  *tc &= ~(TC_HOUR_MASK|TC_MINUTE_MASK|TC_SECOND_MASK|TC_FRAME_MASK|GAVL_TIMECODE_INVALID_MASK);
  *tc |= (((uint64_t)hours)   << TC_HOUR_SHIFT) & TC_HOUR_MASK;
  *tc |= (((uint64_t)minutes) << TC_MINUTE_SHIFT) & TC_MINUTE_MASK;
  *tc |= (((uint64_t)seconds) << TC_SECOND_SHIFT) & TC_SECOND_MASK;
  *tc |= (((uint64_t)frames) << TC_FRAME_SHIFT) & TC_FRAME_MASK;
  }

/** \brief Set the date part of the timecode
 *  \param tc A timecode
 *  \param year The year
 *  \param month The month
 *  \param day The day
 */
  
void gavl_timecode_from_ymd(gavl_timecode_t * tc,
                            int year,
                            int month,
                            int day)
  {
  *tc &= ~(TC_YEAR_MASK|TC_MONTH_MASK|TC_DAY_MASK|GAVL_TIMECODE_INVALID_MASK);
  *tc |= (((uint64_t)year)   << TC_YEAR_SHIFT) & TC_YEAR_MASK;
  *tc |= (((uint64_t)month) << TC_MONTH_SHIFT) & TC_MONTH_MASK;
  *tc |= (((uint64_t)day) << TC_DAY_SHIFT) & TC_DAY_MASK;
  }

/** \brief Get the frame count from the timecode
 *  \param tf The timecode format
 *  \param vf The video format
 *  \param tc A timecode
 *  \returns The frame count
 */
  
int64_t gavl_timecode_to_framecount(const gavl_timecode_format_t * tf,
                                    gavl_timecode_t tc)
  {
  int hours, minutes, seconds, frames, sign;
  int year, month, day;
  int64_t total_hours;

  sign = (tc & GAVL_TIMECODE_SIGN_MASK) ? -1 : 1;
  gavl_timecode_to_hmsf(tc, &hours, &minutes, &seconds, &frames);
  gavl_timecode_to_ymd(tc, &year, &month, &day);

  total_hours = hours;
  
  if(month && day)
    {
    struct tm tm;
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 0;
    tm.tm_mday = day - 1;
    tm.tm_mon = month - 1;
    tm.tm_year  = year - 1900;
    total_hours += mktime(&tm) / 3600;
    }
  
  if(tf->flags & GAVL_TIMECODE_DROP_FRAME)
    {
    uint64_t total_minutes;
     
    /*
      http://www.andrewduncan.ws/Timecodes/Timecodes.html
      totalMinutes = 60 * hours + minutes
      frameNumber  = 108000 * hours + 1800 * minutes
      + 30 * seconds + frames
      - 2 * (totalMinutes - totalMinutes div 10)
       
      where div means integer division with no remainder.
    */
     
    total_minutes = 60 * total_hours + minutes;
     
    return sign * (1800 * total_minutes
                   + 30 * seconds + frames
                   - 2 * (total_minutes - total_minutes / 10));
    }
  else
    {
    return sign * ((int64_t)frames +
                   tf->int_framerate * ((int64_t)(seconds) +
                                        60 * ((int64_t)(minutes) +
                                              60 * (total_hours))));
    }
  }

/** \brief Get a timecode from the frame count
 *  \param tf The timecode format
 *  \param vf The video format
 *  \param tc A timecode
 *  \param fc The frame count
 */

gavl_timecode_t gavl_timecode_from_framecount(const gavl_timecode_format_t * tf,
                                              int64_t fc)
  {
  int hours, minutes, seconds, frames;
  //  int frames;
  struct tm tm;
  time_t ti;
  gavl_timecode_t ret;
  
  ret = 0;

  if(fc < 0)
    {
    fc = -fc;
    ret |= GAVL_TIMECODE_SIGN_MASK;
    }
  
  if(tf->flags & GAVL_TIMECODE_DROP_FRAME)
    {
    int64_t D, M;

    D = fc / 17982;
    M = fc % 17982;
    fc +=  18*D + 2*((M - 2) / 1798);
    }
  
  frames  = fc % tf->int_framerate;
  fc /= tf->int_framerate;

  if(fc < 24*3600)
    {
    seconds = fc % 60;
    fc /= 60;
    
    minutes = fc % 60;
    fc /= 60;
    
    hours   = fc % 24;
    
    gavl_timecode_from_hmsf(&ret, hours, minutes,
                            seconds, frames);
    }
  else
    {
    ti = fc;
    gavl_localtime_r(&ti, &tm);
    tm.tm_mon++;
    tm.tm_mday++;
    tm.tm_year += 1900;
    gavl_timecode_from_ymd(&ret, tm.tm_year, tm.tm_mon,
                           tm.tm_mday);
    gavl_timecode_from_hmsf(&ret, tm.tm_hour, tm.tm_min,
                            tm.tm_sec, frames);
    }
  
  return ret;
  }

/** \brief Dump a timecode to stderr
 *  \param tf The timecode format
 *  \param tc A timecode
 *
 *  This is used mainly for debugging
 */
  
void gavl_timecode_dump(const gavl_timecode_format_t * tf,
                        gavl_timecode_t tc)
  {
  char str[GAVL_TIMECODE_STRING_LEN];
  gavl_timecode_prettyprint(tf, tc, str);
  fprintf(stderr, "%s", str);
  }
  

/* -YYYY-MM-DD-HH:MM:SS.FFFF */

void gavl_timecode_prettyprint(const gavl_timecode_format_t * tf,
                               gavl_timecode_t tc,
                               char str[GAVL_TIMECODE_STRING_LEN])
  {
  char * ptr;
  int year, month, day, hours, minutes, seconds, frames;

  if(tc == GAVL_TIMECODE_UNDEFINED)
    {
    sprintf(str, "--:--:--:--");
    return;
    }
  
  gavl_timecode_to_hmsf(tc, &hours,
                        &minutes, &seconds, &frames);

  gavl_timecode_to_ymd(tc, &year, &month, &day);
  
  ptr = str;

  if(tc & GAVL_TIMECODE_SIGN_MASK)
    {
    sprintf(ptr, "-");
    ptr++;
    }

  if(month && day)
    {
    sprintf(ptr, "%04d-%02d-%02d ", year, month, day);
    ptr += strlen(ptr);
    }

  if(!tf || (tf->int_framerate < 100))
    sprintf(ptr, "%02d:%02d:%02d:%02d", hours, minutes, seconds, frames);
  else if(tf->int_framerate < 1000)
    sprintf(ptr, "%02d:%02d:%02d:%03d", hours, minutes, seconds, frames);
  else
    sprintf(ptr, "%02d:%02d:%02d:%04d", hours, minutes, seconds, frames);
  }
  
void gavl_timecode_prettyprint_short(gavl_timecode_t tc,
                                     char str[GAVL_TIMECODE_STRING_LEN_SHORT])
  {
  char * ptr;
  int hours, minutes, seconds, frames;

  ptr = str;
  
  if(tc == GAVL_TIMECODE_UNDEFINED)
    {
    sprintf(ptr, "--:--:--:--");
    return;
    }
  gavl_timecode_to_hmsf(tc, &hours,
                        &minutes, &seconds, &frames);
  
  
  if(tc & GAVL_TIMECODE_SIGN_MASK)
    {
    sprintf(ptr, "-");
    ptr++;
    }
  
  sprintf(ptr, "%02d:%02d:%02d:%02d", hours % 100, minutes, seconds, frames);
  }
  

GAVL_PUBLIC
void gavl_timecode_format_copy(gavl_timecode_format_t * dst,
                               const gavl_timecode_format_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }
