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

#include <gavl/gavl.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

gavl_frame_table_t * gavl_frame_table_create()
  {
  gavl_frame_table_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
  }

void gavl_frame_table_destroy(gavl_frame_table_t * tab)
  {
  if(tab->entries)
    free(tab->entries);
  if(tab->timecodes)
    free(tab->timecodes);
  free(tab);
  }
  
void gavl_frame_table_append_entry(gavl_frame_table_t * t, int64_t duration)
  {
  if(t->num_entries && (t->entries[t->num_entries-1].duration == duration))
    {
    t->entries[t->num_entries-1].num_frames++;
    return;
    }
  
  if(t->entries_alloc <= t->num_entries)
    {
    t->entries_alloc = t->num_entries + 128;
    t->entries = realloc(t->entries,
                         t->entries_alloc * sizeof(*t->entries));
    memset(t->entries + t->num_entries, 0,
           (t->entries_alloc-t->num_entries) * sizeof(*t->entries));
    }
  t->entries[t->num_entries].duration = duration;
  t->entries[t->num_entries].num_frames = 1;
  t->num_entries++;
  }

void
gavl_frame_table_append_timecode(gavl_frame_table_t * t,
                                 int64_t pts, gavl_timecode_t tc)
  {
  if(t->num_timecodes + 1 > t->timecodes_alloc)
    {
    t->timecodes_alloc += 128;
    t->timecodes = realloc(t->timecodes,
                           t->timecodes_alloc * sizeof(*t->timecodes));
    }
  t->timecodes[t->num_timecodes].pts = pts;
  t->timecodes[t->num_timecodes].tc  = tc;
  t->num_timecodes++;
  }

int64_t gavl_frame_table_frame_to_time(const gavl_frame_table_t * t,
                                       int64_t frame, int * duration)
  {
  int i = 0;
  int64_t ret = t->offset;
  int counter = 0;
  
  while(1)
    {
    if(i >= t->num_entries)
      {
      if(duration)
        *duration = 0;
      return GAVL_TIME_UNDEFINED;
      }
    else if(frame - counter < t->entries[i].num_frames)
      {
      if(duration)
        *duration = t->entries[i].duration;
      
      ret += (frame-counter)*t->entries[i].duration;
      break;
      }

    counter += t->entries[i].num_frames;
    ret += t->entries[i].num_frames * t->entries[i].duration;
    i++;
    }
      
  return ret;
  }

int64_t gavl_frame_table_time_to_frame(const gavl_frame_table_t * t,
                                   int64_t time,
                                   int64_t * start_time)
  {
  int i = 0;
  int64_t ret = 0;
  int64_t counter = t->offset;
  int64_t off;
  
  if(time < counter)
    return -1;

  while(1)
    {
    if(i >= t->num_entries)
      {
      if(start_time)
        *start_time = GAVL_TIME_UNDEFINED;
      return -1;
      }
    else if(time - counter < t->entries[i].num_frames*t->entries[i].duration)
      {
      off = (time - counter) / t->entries[i].duration;
      ret += off;
      if(start_time)
        *start_time = counter + off * t->entries[i].duration;
      break;
      }
    
    ret += t->entries[i].num_frames;
    counter += t->entries[i].num_frames * t->entries[i].duration;
    i++;
    }
  return ret;
  }

/* time <-> timecode */

static gavl_timecode_t
get_timecode(const gavl_frame_table_t * t,
             int64_t frame,
             int64_t frame_time,
             int64_t * start_time,
             const gavl_timecode_format_t * fmt)
  {
  gavl_timecode_t ret;
  
  if(frame < 0)
    frame = gavl_frame_table_time_to_frame(t, frame_time, start_time);
  else
    {
    frame_time = gavl_frame_table_frame_to_time(t, frame, NULL);
    if(start_time)
      *start_time = frame_time;
    }
  
  if(!t->num_timecodes)
    {
    /* We assume the first timecode to be
       00:00:00:00 */
    ret = gavl_timecode_from_framecount(fmt, frame);
    }
  else
    {
    int pos;
    int64_t tc_frame;
    int64_t cnt;
    
    /* Get the last table entry */
    pos = t->num_timecodes - 1;
    
    while(1)
      {
      if(t->timecodes[pos].pts <= frame_time)
        break;
      pos--;
      if(pos < 0)
        break;
      }

    if(pos < 0)
      {
      /* Count backwards */
      tc_frame = gavl_frame_table_time_to_frame(t, t->timecodes[0].pts, NULL);
      cnt = gavl_timecode_to_framecount(fmt, t->timecodes[0].tc);
      cnt -= (tc_frame - frame);
      ret = gavl_timecode_from_framecount(fmt, cnt);
      }
    else
      {
      /* Count forward */
      tc_frame = gavl_frame_table_time_to_frame(t, t->timecodes[pos].pts, NULL);
      cnt = gavl_timecode_to_framecount(fmt, t->timecodes[0].tc);
      cnt += (frame - tc_frame);
      ret = gavl_timecode_from_framecount(fmt, cnt);
      }
    }
  return ret;
  }

/** \brief Convert a timestamp to a timecode
 *  \param t A frame table
 *  \param time Time in stream timescale
 *  \param start_time If non NULL, returns the start time of that frame
 *  \param fmt Timecode format
 *  \returns The interpolated timecode of that frame or GAVL_TIMECODE_UNDEFINED if such frame doesn't exist.
 *
 * Since 1.1.2.
 */

gavl_timecode_t
gavl_frame_table_time_to_timecode(const gavl_frame_table_t * t,
                                  int64_t time,
                                  int64_t * start_time,
                                  const gavl_timecode_format_t * fmt)
  {
  return get_timecode(t, -1, time, start_time, fmt);
  }

gavl_timecode_t
gavl_frame_table_frame_to_timecode(const gavl_frame_table_t * t,
                                   int64_t frame,
                                   int64_t * start_time,
                                   const gavl_timecode_format_t * fmt)
  {
  return get_timecode(t, frame, GAVL_TIME_UNDEFINED, start_time, fmt);
  }


/** \brief Convert a timecode to a timestamp
 *  \param t A frame table
 *  \param tc Timecode
 *  \param fmt Timecode format
 *  \returns The pts corresponding to that timecode or GAVL_TIME_UNDEFINED if such frame doesn't exist.
 *
 * Since 1.1.2.
 */

int64_t
gavl_frame_table_timecode_to_time(const gavl_frame_table_t * t,
                                  gavl_timecode_t tc,
                                  const gavl_timecode_format_t * fmt)
  {
  int64_t ret;
  int64_t cnt;
  int64_t cnt1;
  int64_t frame;
  
  int pos = t->num_timecodes-1;

  cnt = gavl_timecode_to_framecount(fmt, tc);

  if(!t->num_timecodes)
    {
    return gavl_frame_table_frame_to_time(t, cnt, NULL);
    }
  
  while(1)
    {
    cnt1 = gavl_timecode_to_framecount(fmt, t->timecodes[pos].tc);
    if(cnt1 <= cnt)
      break;
    pos--;
    if(pos < 0)
      break;
    }

  if(pos < 0)
    {
    /* Count backwards */
    frame = gavl_frame_table_time_to_frame(t, t->timecodes[0].pts, NULL);
    frame -= cnt1 - cnt;

    if(frame < 0)
      ret = GAVL_TIME_UNDEFINED;
    else
      ret = gavl_frame_table_frame_to_time(t, frame, NULL);
    }
  else
    {
    /* Count forward */
    frame = gavl_frame_table_time_to_frame(t, t->timecodes[pos].pts, NULL);
    frame += cnt - cnt1;
    ret = gavl_frame_table_frame_to_time(t, frame, NULL);
    }
  return ret;
  }

int64_t gavl_frame_table_num_frames(const gavl_frame_table_t * t)
  {
  int i;
  int64_t ret = 0;
  for(i = 0; i < t->num_entries; i++)
    {
    ret += t->entries[i].num_frames;
    }
  return ret;
  }

int64_t gavl_frame_table_duration(const gavl_frame_table_t * t)
  {
  int i;
  int64_t ret = 0;
  for(i = 0; i < t->num_entries; i++)
    {
    ret += t->entries[i].num_frames * t->entries[i].duration;
    }
  return ret;
  }

int64_t gavl_frame_table_end_time(const gavl_frame_table_t * t)
  {
  return gavl_frame_table_duration(t) + t->offset;
  }

void gavl_frame_table_dump(const gavl_frame_table_t * t)
  {
  int i;
  char str[GAVL_TIMECODE_STRING_LEN];

  fprintf(stderr, "Entries: %"PRId64", total frames: %"PRId64", offset: %"PRId64"\n",
          t->num_entries, gavl_frame_table_num_frames(t), t->offset);
  
  for(i = 0; i < t->num_entries; i++)
    {
    fprintf(stderr, "  Frames: %"PRId64", duration: %"PRId64"\n",
            t->entries[i].num_frames, t->entries[i].duration);
    }

  fprintf(stderr, "Timecodes: %d\n", t->num_timecodes);
  for(i = 0; i < t->num_timecodes; i++)
    {
    gavl_timecode_prettyprint(NULL, t->timecodes[i].tc, str);
    fprintf(stderr, "  PTS: %"PRId64" TC: %s\n",
            t->timecodes[i].pts, str);
    }
  }

/* Load / save */

#define FRAMETABLE_VERSION 1

static const char sig[] = "GAVL_FRAMETABLE";
#define SIG_LEN 15

static int load_32(FILE * f, int32_t * ret)
  {
  uint8_t buf[4];
  if(fread(buf, 1, 4, f) < 4)
    return 0;
  *ret = buf[0]; *ret <<= 8;
  *ret |= buf[1]; *ret <<= 8;
  *ret |= buf[2]; *ret <<= 8;
  *ret |= buf[3];
  return 1;
  }

static int save_32(FILE * f, int32_t val)
  {
  uint8_t buf[4];
  buf[0] = (val >> 24) & 0xff;
  buf[1] = (val >> 16) & 0xff;
  buf[2] = (val >> 8) & 0xff;
  buf[3] = val & 0xff;
  if(fwrite(buf, 1, 4, f) < 4)
    return 0;
  return 1;
  }

static int load_64(FILE * f, int64_t * ret)
  {
  uint8_t buf[8];
  if(fread(buf, 1, 8, f) < 8)
    return 0;
  *ret = buf[0]; *ret <<= 8;
  *ret |= buf[1]; *ret <<= 8;
  *ret |= buf[2]; *ret <<= 8;
  *ret |= buf[3]; *ret <<= 8;
  *ret |= buf[4]; *ret <<= 8;
  *ret |= buf[5]; *ret <<= 8;
  *ret |= buf[6]; *ret <<= 8;
  *ret |= buf[7];
  return 1;
  }

static int save_64(FILE * f, int64_t val)
  {
  uint8_t buf[8];
  buf[0] = (val >> 56) & 0xff;
  buf[1] = (val >> 48) & 0xff;
  buf[2] = (val >> 40) & 0xff;
  buf[3] = (val >> 32) & 0xff;
  buf[4] = (val >> 24) & 0xff;
  buf[5] = (val >> 16) & 0xff;
  buf[6] = (val >> 8) & 0xff;
  buf[7] = val & 0xff;
  if(fwrite(buf, 1, 8, f) < 8)
    return 0;
  return 1;
  }

gavl_frame_table_t * gavl_frame_table_load(const char * filename)
  {
  FILE * f = NULL;
  gavl_frame_table_t * ret = NULL;
  char sig_buf[SIG_LEN];
  int32_t version;
  int i;
  
  /* Open file */
  f = fopen(filename, "rb");
  if(!f)
    return 0;

  /* Check for signature */
  if(fread(sig_buf, 1, SIG_LEN, f) < SIG_LEN)
    goto fail;

  if(memcmp(sig, sig_buf, SIG_LEN))
    goto fail;

  /* Check version */
  if(!load_32(f, &version))
    goto fail;
  if(version != FRAMETABLE_VERSION)
    goto fail;

  ret = calloc(1, sizeof(*ret));
  
  /* Offset */
  if(!load_64(f, &ret->offset))
    goto fail;

  /* Entries */
  if(!load_64(f, &ret->num_entries))
    goto fail;
  ret->entries_alloc = ret->num_entries;
  ret->entries = calloc(ret->entries_alloc, sizeof(*ret->entries));

  for(i = 0; i < ret->num_entries; i++)
    {
    if(!load_64(f, &ret->entries[i].num_frames) ||
       !load_64(f, &ret->entries[i].duration))
      goto fail;
    }

  /* Timecodes */
  if(!load_32(f, &ret->num_timecodes))
    goto fail;

  if(ret->num_timecodes)
    {
    ret->timecodes_alloc = ret->num_timecodes;
    ret->timecodes = calloc(ret->timecodes_alloc, sizeof(*ret->timecodes));

    for(i = 0; i < ret->num_timecodes; i++)
      {
      if(!load_64(f, &ret->timecodes[i].pts) ||
         !load_64(f, (int64_t*)&ret->timecodes[i].tc))
        goto fail;
      }
    }
  
  fclose(f);
  return ret;
  
  fail:
  if(f)
    fclose(f);
  if(ret)
    gavl_frame_table_destroy(ret);
  return NULL;
  
  }

int gavl_frame_table_save(const gavl_frame_table_t * tab,
                          const char * filename)
  {
  FILE * f = NULL;
  int i;
  /* Open file */
  f = fopen(filename, "wb");
  if(!f)
    return 0;
  
  /* Write signature */
  if(fwrite(sig, 1, SIG_LEN, f) < SIG_LEN)
    goto fail;

  /* Write version */
  if(!save_32(f, FRAMETABLE_VERSION))
    goto fail;

  /* Offset */
  if(!save_64(f, tab->offset))
    goto fail;

  /* Entries */
  if(!save_64(f, tab->num_entries))
    goto fail;

  for(i = 0; i < tab->num_entries; i++)
    {
    if(!save_64(f, tab->entries[i].num_frames) ||
       !save_64(f, tab->entries[i].duration))
      goto fail;
    }

  /* Timecodes */
  if(!save_32(f, tab->num_timecodes))
    goto fail;

  for(i = 0; i < tab->num_timecodes; i++)
    {
    if(!save_64(f, tab->timecodes[i].pts) ||
       !save_64(f, tab->timecodes[i].tc))
      goto fail;
    }
  
  fclose(f);
  return 1;
  
  fail:
  if(f)
    fclose(f);
  return 0;
  
  }



/** \brief Create a frame table for an audio stream
 *  \param tab A frame table
 *  \param samplerate Samplerate for this stream
 *  \param offset PTS offset of this stream in samples
 *  \param duration Sample count
 *  \param fmt_ret If non-null, returns the timecode format
 *  \returns A newly allocated frame table
 *
 * Since 1.1.2.
 */

GAVL_PUBLIC gavl_frame_table_t *
gavl_frame_table_create_audio(int samplerate, int64_t offset, int64_t duration,
                              gavl_timecode_format_t * fmt_ret)
  {
  int64_t num;
  int samples_per_frame;
  
  gavl_frame_table_t * ret = gavl_frame_table_create();
  
  ret->offset = offset;
  if(fmt_ret)
    {
    memset(fmt_ret, 0, sizeof(*fmt_ret));
    fmt_ret->int_framerate = 100;
    }
  
  if(!(samplerate % 100))
    {
    /* We make frames 10 ms long */
    samples_per_frame = samplerate / 100;
    
    ret->entries_alloc = 2;
    ret->entries = calloc(2, sizeof(*ret->entries));

    num = duration / samples_per_frame;
    
    if(num)
      {
      ret->entries[ret->num_entries].num_frames = num;
      ret->entries[ret->num_entries].duration = samples_per_frame;
      ret->num_entries++;
      }
    
    num = duration % samples_per_frame;
    if(num)
      {
      ret->entries[ret->num_entries].num_frames = 1;
      ret->entries[ret->num_entries].duration = num;
      ret->num_entries++;
      }
    }
  else
    {
    /* Yea that's extremely disgusting. But odd framerates are too! */
    gavl_time_t time_cnt = 0;
    int64_t count, last_count = 0;
    
    while(1)
      {
      time_cnt += GAVL_TIME_SCALE / 100;
      count = gavl_time_scale(samplerate, time_cnt);

      if(count > duration)
        count = duration;
      
      gavl_frame_table_append_entry(ret, count - last_count);

      if(count >= duration)
        break;
      last_count = count;
      }
    }
  return ret;
  }

/** \brief Set a frame table for constant framerate video
 *  \param tab A frame table
 *  \param offset Timestamp of the first frame
 *  \param duration Duration of each frame
 *  \param num_frames Number of frames
 *  \param fmt Timecode format (or NULL)
 *  \param start_timecode Timecode of the first frame (or GAVL_TIMECODE_UNDEFINED)
 *
 * Since 1.1.2.
 */

GAVL_PUBLIC gavl_frame_table_t *
gavl_frame_table_create_cfr(int64_t offset, int64_t frame_duration,
                            int64_t num_frames,
                            gavl_timecode_t start_timecode)
  {
  gavl_frame_table_t * ret = gavl_frame_table_create();
  ret->offset = offset;
  
  /* Make frames */
  ret->entries_alloc = 1;
  ret->entries = calloc(ret->entries_alloc, sizeof(*ret->entries));
  ret->entries[0].duration = frame_duration;
  ret->entries[0].num_frames = num_frames;
  ret->num_entries = 1;

  /* Make timecodes */
  if(start_timecode == GAVL_TIMECODE_UNDEFINED)
    {
    ret->timecodes_alloc = 1;
    ret->timecodes = calloc(ret->timecodes_alloc, sizeof(*ret->timecodes));
    ret->timecodes[0].pts = 0;
    ret->timecodes[0].tc = start_timecode;
    ret->num_timecodes = 1;
    }
  return ret;
  }
  
/** \brief Copy a frame table to another
 *  \param tab A frame table
 *  \returns A newly allocated copy
 *
 * Since 1.1.2.
 */

GAVL_PUBLIC gavl_frame_table_t *
gavl_frame_table_copy(const gavl_frame_table_t * tab)
  {
  gavl_frame_table_t * ret = malloc(sizeof(*ret));
  memcpy(ret, tab, sizeof(*ret));

  if(tab->num_entries)
    {
    ret->entries = malloc(tab->num_entries * sizeof(*ret->entries));
    memcpy(ret->entries, tab->entries, tab->num_entries * sizeof(*ret->entries));
    }
  
  if(tab->num_timecodes)
    {
    ret->timecodes = malloc(tab->num_timecodes * sizeof(*ret->timecodes));
    memcpy(ret->timecodes, tab->timecodes, tab->num_timecodes * sizeof(*ret->timecodes));
    }
  return ret;
  }
