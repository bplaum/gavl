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

#ifndef GAVL_PARAMETER_H_INCLUDED
#define GAVL_PARAMETER_H_INCLUDED

#include <gavl/gavl.h>
#include <gavl/value.h>
#include <gavl/metatags.h>



typedef enum
  {
    GAVL_PARAMETER_SECTION       = 0, //!< Dummy type. It contains no data but acts as a separator in notebook style configuration windows
    GAVL_PARAMETER_CHECKBUTTON   = 1, //!< Bool
    GAVL_PARAMETER_INT           = 2, //!< Integer spinbutton
    GAVL_PARAMETER_FLOAT         = 3, //!< Float spinbutton
    GAVL_PARAMETER_SLIDER_INT    = 4, //!< Integer slider
    GAVL_PARAMETER_SLIDER_FLOAT  = 5, //!< Float slider
    GAVL_PARAMETER_STRING        = 6, //!< String (one line only)
    GAVL_PARAMETER_STRING_HIDDEN = 7, //!< Encrypted string (displays as ***)
    GAVL_PARAMETER_STRINGLIST    = 8, //!< Popdown menu with string values
    GAVL_PARAMETER_COLOR_RGB     = 9, //!< RGB Color
    GAVL_PARAMETER_COLOR_RGBA    = 10, //!< RGBA Color
    GAVL_PARAMETER_FONT          = 11, //!< Font (contains fontconfig compatible fontname)
    GAVL_PARAMETER_FILE          = 12, //!< File
    GAVL_PARAMETER_DIRECTORY     = 13, //!< Directory
    GAVL_PARAMETER_MULTI_MENU    = 14, //!< Menu with config- and infobutton
    GAVL_PARAMETER_MULTI_LIST    = 15, //!< List with config- and infobutton
    GAVL_PARAMETER_MULTI_CHAIN   = 16, //!< Several subitems (including suboptions) can be arranged in a chain
    GAVL_PARAMETER_TIME          = 17, //!< Time
    GAVL_PARAMETER_POSITION      = 18, //!< Position (x/y coordinates, scaled 0..1)
    GAVL_PARAMETER_BUTTON        = 19, //!< Pressing the button causes set_parameter to be called with NULL value
    GAVL_PARAMETER_DIRLIST       = 20, //!< List of directories
  } gavl_parameter_type_t;


GAVL_PUBLIC
gavl_type_t gavl_parameter_type_to_gavl(gavl_parameter_type_t type);


/* Parameter info (for static initialization) */

/** \brief Parameter description
 *
 *  Usually, parameter infos are passed around as NULL-terminated arrays.
 */

struct gavl_parameter_info_s
  {
  char * name; //!< Unique name. Can contain alphanumeric characters plus underscore.
  char * long_name; //!< Long name (for labels)

  char * gettext_domain; //!< First argument for bindtextdomain(). In an array, it's valid for subsequent entries too.
  char * gettext_directory; //!< Second argument for bindtextdomain(). In an array, it's valid for subsequent entries too.
  
  gavl_parameter_type_t type; //!< Type

  int flags; //!< Mask of BG_PARAMETER_* defines
  
  gavl_value_t val_default; //!< Default value
  gavl_value_t val_min; //!< Minimum value (for arithmetic types)
  gavl_value_t val_max; //!< Maximum value (for arithmetic types)
  
  /* Names which can be passed to set_parameter (NULL terminated) */

  char const * const * multi_names; //!< Names for multi option parameters (NULL terminated)

  /* Long names are optional, if they are NULL,
     the short names are used */

  char const * const * multi_labels; //!< Optional labels for multi option parameters
  char const * const * multi_descriptions; //!< Optional descriptions (will be displayed by info buttons)
  
  /*
   *  These are parameters for each codec.
   *  The name members of these MUST be unique with respect to the rest
   *  of the parameters passed to the same set_parameter func
   */

  struct gavl_parameter_info_s const * const * multi_parameters; //!< Parameters for each option. The name members of these MUST be unique with respect to the rest of the parameters passed to the same set_parameter func
  
  int num_digits; //!< Number of digits for floating point parameters
  
  char * help_string; //!< Help strings for tooltips or --help option 

  char ** multi_names_nc; //!< When allocating dynamically, use this instead of multi_names and call \ref bg_parameter_info_set_const_ptrs at the end

  char ** multi_labels_nc; //!< When allocating dynamically, use this instead of multi_labels and call \ref bg_parameter_info_set_const_ptrs at the end

  char ** multi_descriptions_nc; //!< When allocating dynamically, use this instead of multi_descriptions and call \ref bg_parameter_info_set_const_ptrs at the end 

  struct gavl_parameter_info_s ** multi_parameters_nc; //!< When allocating dynamically, use this instead of multi_parameters and call \ref bg_parameter_info_set_const_ptrs at the end 

  };

typedef struct gavl_parameter_info_s  gavl_parameter_info_t;

/* Dictionary keys */

#define GAVL_PARAMETER_TYPE              "ptype"

#define GAVL_PARAMETER_DEFAULT           "default"
#define GAVL_PARAMETER_MIN               "min"
#define GAVL_PARAMETER_MAX               "max"

#define GAVL_PARAMETER_OPTIONS           "options"
#define GAVL_PARAMETER_PARAMS            "params"
#define GAVL_PARAMETER_SECTIONS          "sections"

#define GAVL_PARAMETER_HELP              "help"
#define GAVL_PARAMETER_NUM_DIGITS        "num_digits"

#define GAVL_PARAMETER_GETTEXT_DOMAIN    "gettext_domain"
#define GAVL_PARAMETER_GETTEXT_DIRECTORY "gettext_directory"

/*
 *  Structure of a parameter info
 *
 *  $params = [
 *    param_1 (dict)
 *    param_2 (dict)
 *    param_3 (dict)
 *  ];
 *
 *  $sections = [
 *    section_1 (dict)
 *    section_2 (dict)
 *    section_3 (dict)
 *  ];
 *  
 */

GAVL_PUBLIC
int gavl_parameter_num_sections(const gavl_dictionary_t* params);

GAVL_PUBLIC
int gavl_parameter_num_params(const gavl_dictionary_t* params);

GAVL_PUBLIC
int gavl_parameter_num_options(const gavl_dictionary_t* params);


GAVL_PUBLIC
const gavl_dictionary_t* gavl_parameter_get_section(const gavl_dictionary_t* params, int idx);

GAVL_PUBLIC
const gavl_dictionary_t* gavl_parameter_get_param(const gavl_dictionary_t* params, int idx);

GAVL_PUBLIC
const gavl_dictionary_t* gavl_parameter_get_option(const gavl_dictionary_t* param, int idx);

GAVL_PUBLIC
const gavl_dictionary_t* gavl_parameter_get_param_by_name(const gavl_dictionary_t* params, const char * name);

GAVL_PUBLIC
const gavl_dictionary_t* gavl_parameter_get_option_by_name(const gavl_dictionary_t* param, const char * name);

GAVL_PUBLIC gavl_dictionary_t*
gavl_parameter_append_section(gavl_dictionary_t* params, const char * name,
                              const char * label);
GAVL_PUBLIC gavl_dictionary_t*
gavl_parameter_append_option(gavl_dictionary_t* param, const char * name,
                             const char * label);

GAVL_PUBLIC gavl_dictionary_t*
gavl_parameter_append_param(gavl_dictionary_t* params, const char * name,
                            const char * label, gavl_parameter_type_t type);

GAVL_PUBLIC gavl_parameter_type_t
gavl_parameter_info_get_type(const gavl_dictionary_t* info);

GAVL_PUBLIC void
gavl_parameter_init(gavl_dictionary_t* param,
                    const char * name,
                    const char * label,
                    gavl_parameter_type_t type);

GAVL_PUBLIC void
gavl_parameter_info_append_static(gavl_dictionary_t* params,
                                  const gavl_parameter_info_t * p);

GAVL_PUBLIC void
gavl_parameter_from_static(gavl_dictionary_t* param,
                                const gavl_parameter_info_t * p);

/*
 * Value type for GAVL_PARAMETER_MULTI_MENU, GAVL_PARAMETER_MULTI_LIST and
 * GAVL_PARAMETER_MULTI_CHAIN:
 * {
 * $name: name
 * $cfg: Configuration associated with name
 * }
 *  
 */

GAVL_PUBLIC gavl_dictionary_t *
gavl_parameter_multi_get_config_nc(gavl_dictionary_t*);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_parameter_multi_get_config(const gavl_dictionary_t*);

GAVL_PUBLIC int
gavl_parameter_parameter_init_value(const gavl_dictionary_t * param,
                                    gavl_value_t * val);


#endif // GAVL_PARAMETER_H_INCLUDED
