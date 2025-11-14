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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <gavl/parameter.h>


int gavl_parameter_num_sections(const gavl_dictionary_t* params)
  {
  const gavl_array_t * arr = gavl_dictionary_get_array(params, GAVL_PARAMETER_SECTIONS);
  return arr ? arr->num_entries : 0;
  }

int gavl_parameter_num_params(const gavl_dictionary_t* params)
  {
  const gavl_array_t * arr = gavl_dictionary_get_array(params, GAVL_PARAMETER_PARAMS);
  return arr ? arr->num_entries : 0;
  }

int gavl_parameter_num_options(const gavl_dictionary_t* params)
  {
  const gavl_array_t * arr = gavl_dictionary_get_array(params, GAVL_PARAMETER_OPTIONS);
  return arr ? arr->num_entries : 0;
  }

const gavl_dictionary_t* gavl_parameter_get_section(const gavl_dictionary_t* params,
                                                    int idx)
  {
  const gavl_value_t * val;
  const gavl_array_t * arr;

  if((arr= gavl_dictionary_get_array(params, GAVL_PARAMETER_SECTIONS)) &&
     (val = gavl_array_get(arr, idx)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  }

const gavl_dictionary_t* gavl_parameter_get_option(const gavl_dictionary_t* params,
                                                   int idx)
  {
  const gavl_value_t * val;
  const gavl_array_t * arr;

  if((arr= gavl_dictionary_get_array(params, GAVL_PARAMETER_OPTIONS)) &&
     (val = gavl_array_get(arr, idx)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  }

const gavl_dictionary_t* gavl_parameter_get_param(const gavl_dictionary_t* params,
                                                  int idx)
  {
  const gavl_value_t * val;
  const gavl_array_t * arr;

  if((arr= gavl_dictionary_get_array(params, GAVL_PARAMETER_PARAMS)) &&
     (val = gavl_array_get(arr, idx)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  
  }

gavl_dictionary_t* gavl_parameter_append_section(gavl_dictionary_t* params,
                                                 const char * name,
                                                 const char * label)
  {
  gavl_array_t * arr;
  gavl_dictionary_t * dict;
  gavl_value_t val;
  gavl_value_init(&val);
  dict = gavl_value_set_dictionary(&val);

  gavl_dictionary_set_string(dict, GAVL_META_NAME, name);
  gavl_dictionary_set_string(dict, GAVL_META_LABEL, label);
  
  arr= gavl_dictionary_get_array_create(params, GAVL_PARAMETER_SECTIONS);

  gavl_array_splice_val_nocopy(arr, -1, 0, &val);
  return gavl_value_get_dictionary_nc(&arr->entries[arr->num_entries-1]);
  }

gavl_dictionary_t * gavl_parameter_append_option(gavl_dictionary_t* params,
                                                const char * name,
                                                const char * label)
  {
  gavl_array_t * arr;
  gavl_dictionary_t * dict;
  gavl_value_t val;
  gavl_value_init(&val);
  dict = gavl_value_set_dictionary(&val);

  gavl_dictionary_set_string(dict, GAVL_META_NAME, name);
  gavl_dictionary_set_string(dict, GAVL_META_LABEL, label);
  
  arr= gavl_dictionary_get_array_create(params, GAVL_PARAMETER_OPTIONS);
  
  gavl_array_splice_val_nocopy(arr, -1, 0, &val);
  return gavl_value_get_dictionary_nc(&arr->entries[arr->num_entries-1]);
  }

void gavl_parameter_init(gavl_dictionary_t* param,
                         const char * name,
                         const char * label,
                         gavl_parameter_type_t type)
  {
  gavl_dictionary_set_string(param, GAVL_META_NAME, name);
  gavl_dictionary_set_string(param, GAVL_META_LABEL, label);
  gavl_dictionary_set_int(param, GAVL_PARAMETER_TYPE, type);
  }
  
gavl_dictionary_t * gavl_parameter_append_param(gavl_dictionary_t* params,
                                               const char * name,
                                               const char * label,
                                               gavl_parameter_type_t type)
  {
  gavl_array_t * arr;
  gavl_dictionary_t * dict;
  gavl_value_t val;
  gavl_value_init(&val);
  dict = gavl_value_set_dictionary(&val);

  gavl_parameter_init(dict, name, label, type);
  
  
  arr= gavl_dictionary_get_array_create(params, GAVL_PARAMETER_PARAMS);

  gavl_array_splice_val_nocopy(arr, -1, 0, &val);
  return gavl_value_get_dictionary_nc(&arr->entries[arr->num_entries-1]);
  }

gavl_parameter_type_t gavl_parameter_info_get_type(const gavl_dictionary_t* info)
  {
  int type = 0;
  gavl_dictionary_get_int(info, GAVL_PARAMETER_TYPE, &type);
  return type;
  }

void
gavl_parameter_from_static(gavl_dictionary_t* param,
                           const gavl_parameter_info_t * p)
  {
  int i = 0;
  const char *const * label = NULL;
  const gavl_parameter_info_t *const* sub_p = NULL;
  gavl_dictionary_t * opt;

  if(p->val_default.type != GAVL_TYPE_UNDEFINED)
    gavl_dictionary_set(param, GAVL_PARAMETER_DEFAULT, &p->val_default);
  
  if(p->val_min.type != GAVL_TYPE_UNDEFINED)
    gavl_dictionary_set(param, GAVL_PARAMETER_MIN,     &p->val_min);

  if(p->val_max.type != GAVL_TYPE_UNDEFINED)
    gavl_dictionary_set(param, GAVL_PARAMETER_MAX,     &p->val_max);

  if(p->num_digits)
    gavl_dictionary_set_int(param, GAVL_PARAMETER_NUM_DIGITS, p->num_digits);
  
  gavl_dictionary_set_string(param, GAVL_PARAMETER_HELP, p->help_string);
  
  /* Options */
  
  if(p->multi_names)
    {
    if(p->multi_labels)
      label = p->multi_labels;
        
    if(p->multi_parameters)
      sub_p = p->multi_parameters;
    
    while(p->multi_names[i])
      {
      opt = gavl_parameter_append_option(param, p->multi_names[i], label ? *label : NULL);
      
      if(sub_p && *sub_p)
        {
        gavl_parameter_info_append_static(opt, *sub_p);
        }
      if(label)
        {
        label++;
        if(!(*label))
          label = NULL;
        }

      if(sub_p)
        sub_p++;
      
      i++;
      }
    }
  }

void gavl_parameter_info_append_static(gavl_dictionary_t * params,
                                       const gavl_parameter_info_t * p)
  {
  int idx = 0;
  gavl_dictionary_t * section = params;
  
  if(!p)
    return;

  while(p[idx].name)
    {
    if(p[idx].gettext_domain && p[idx].gettext_directory)
      {
      gavl_dictionary_set_string(params, GAVL_PARAMETER_GETTEXT_DOMAIN,
                                 p[idx].gettext_domain);

      gavl_dictionary_set_string(params, GAVL_PARAMETER_GETTEXT_DIRECTORY,
                                 p[idx].gettext_directory);
      }
    
    if(p[idx].type == GAVL_PARAMETER_SECTION)
      {
      section = gavl_parameter_append_section(params, p[idx].name, p[idx].long_name);
      
      gavl_dictionary_copy_value(section, params, GAVL_PARAMETER_GETTEXT_DOMAIN);
      gavl_dictionary_copy_value(section, params, GAVL_PARAMETER_GETTEXT_DIRECTORY);
      }
    else
      {
      gavl_dictionary_t * param =
        gavl_parameter_append_param(section, p[idx].name, p[idx].long_name, p[idx].type);
      
      gavl_parameter_from_static(param, &p[idx]);
      }
    idx++;
    }
  }

static const gavl_dictionary_t* get_item_by_name(const gavl_dictionary_t* dict, const char * arr_name, const char * name)
  {
  int i;
  const gavl_array_t * arr;
  const gavl_dictionary_t * child;
  const char * var;

  if(!(arr = gavl_dictionary_get_array(dict, arr_name)))
    return NULL;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((child = gavl_value_get_dictionary(&arr->entries[i])) &&
       (var = gavl_dictionary_get_string(child, GAVL_META_NAME)) &&
       !strcmp(var, name))
      return child;
    }
  return NULL;
  }

const gavl_dictionary_t* gavl_parameter_get_param_by_name(const gavl_dictionary_t* params, const char * name)
  {
  return get_item_by_name(params, GAVL_PARAMETER_PARAMS, name);
  }

const gavl_dictionary_t* gavl_parameter_get_option_by_name(const gavl_dictionary_t* param, const char * name)
  {
  return get_item_by_name(param, GAVL_PARAMETER_OPTIONS, name);
  }

gavl_type_t gavl_parameter_type_to_gavl(gavl_parameter_type_t type)
  {
  switch(type)
    {
    case GAVL_PARAMETER_SECTION:       //!< Dummy type. It contains no data but acts as a separator in notebook style configuration windows
      return GAVL_TYPE_UNDEFINED;
      break;
    case GAVL_PARAMETER_CHECKBUTTON:   //!< Bool
    case GAVL_PARAMETER_INT:           //!< Integer spinbutton
    case GAVL_PARAMETER_SLIDER_INT:    //!< Integer slider
      return GAVL_TYPE_INT;
      break;
    case GAVL_PARAMETER_FLOAT:         //!< Float spinbutton
    case GAVL_PARAMETER_SLIDER_FLOAT:  //!< Float slider
      return GAVL_TYPE_FLOAT;
      break;
    case GAVL_PARAMETER_STRING:        //!< String (one line only)
    case GAVL_PARAMETER_STRING_HIDDEN: //!< Encrypted string (displays as ***)
    case GAVL_PARAMETER_STRINGLIST:    //!< Popdown menu with string values
    case GAVL_PARAMETER_FONT:          //!< Font (contains fontconfig compatible fontname)
    case GAVL_PARAMETER_FILE:          //!< File
    case GAVL_PARAMETER_DIRECTORY:     //!< Directory
      return GAVL_TYPE_STRING;
      break;
    case GAVL_PARAMETER_MULTI_MENU:    //!< Menu with config- and infobutton
      return GAVL_TYPE_DICTIONARY;
      break;
    case GAVL_PARAMETER_DIRLIST:
    case GAVL_PARAMETER_MULTI_LIST:    //!< List with config- and infobutton
    case GAVL_PARAMETER_MULTI_CHAIN:   //!< Several subitems (including suboptions) can be arranged in a chain
      return GAVL_TYPE_ARRAY;
      break;
    case GAVL_PARAMETER_COLOR_RGB:     //!< RGB Color
      return GAVL_TYPE_COLOR_RGB;
      break;
    case GAVL_PARAMETER_COLOR_RGBA:    //!< RGBA Color
      return GAVL_TYPE_COLOR_RGBA;
      break;
    case GAVL_PARAMETER_TIME:          //!< Time
      return GAVL_TYPE_LONG;
      break;
    case GAVL_PARAMETER_POSITION:      //!< Position (x/y coordinates, scaled 0..1)
      return GAVL_TYPE_POSITION;
      break;
    case GAVL_PARAMETER_BUTTON:        //!< Pressing the button causes set_parameter to be called with NULL value
      return GAVL_TYPE_STRING;
      break;
    }
  return GAVL_TYPE_UNDEFINED;
  }

