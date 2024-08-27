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


#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <language_table.h>

static const char * find_by_code(const char * code, const char ** label)
  {
  char code_str[4];
  int i;
  
  if((code[0] == '\0') ||
     (code[1] == '\0'))
    return NULL;

  code_str[0] = tolower(code[0]);
  code_str[1] = tolower(code[1]);
  
  if(code[2] == '\0')
    {
    /* 2 digit code */
    code_str[2] = '\0';
    
    for(i = 0; i < sizeof(languages)/sizeof(languages[0]); i++)
      {
      if(languages[i].ISO_639_1 && !strcmp(code_str, languages[i].ISO_639_1))
        {
        if(label)
          *label = languages[i].label;
        return languages[i].ISO_639_2_B;
        }
      }
    return NULL;
    }
  
  if(code[3] == '\0')
    {
    /* 3 digit code */
    code_str[2] = tolower(code[2]);
    code_str[3] = '\0';
    
    for(i = 0; i < sizeof(languages)/sizeof(languages[0]); i++)
      {
      if(!strcmp(code_str, languages[i].ISO_639_2_B) || (languages[i].ISO_639_2_T && !strcmp(code_str, languages[i].ISO_639_2_T)))
        {
        if(label)
          *label = languages[i].label;
        return languages[i].ISO_639_2_B;
        }
      }
    return NULL;
    }
  else
    return NULL;
  
  }

const char * gavl_language_get_iso639_2_b_from_code(const char * code)
  {
  return find_by_code(code, NULL);
  }

const char * gavl_language_get_iso639_2_b_from_label(const char * label)
  {
  int i;
  
  for(i = 0; i < sizeof(languages)/sizeof(languages[0]); i++)
    {
    if(!strcasecmp(label, languages[i].label))
      return languages[i].ISO_639_2_B;
    }
  return NULL;
  }

const char * gavl_language_get_label_from_code(const char * code)
  {
  const char * label = NULL;
  find_by_code(code, &label);
  return label;
  }
