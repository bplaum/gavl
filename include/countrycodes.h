typedef struct
  {
  const char * label;
  const char * code_alpha_2; // ISO 3166-1 alpha-2
  const char * code_alpha_3; // ISO 3166-1 alpha-3
  int code_numeric;
  } gavl_country_t;

extern const gavl_country_t gavl_countries[];


