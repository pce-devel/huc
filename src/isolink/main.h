
#define ISOLINK_VERSION "isolink (" GIT_VERSION ", " GIT_DATE ")"


#ifdef _WIN32
#define PATH_SEPARATOR         ';'
#define DIR_SEPARATOR          '\\'
#define DIR_SEPARATOR_STRING   "\\"
#else
#define PATH_SEPARATOR         ':'
#define DIR_SEPARATOR          '/'
#define DIR_SEPARATOR_STRING   "/"
#endif

