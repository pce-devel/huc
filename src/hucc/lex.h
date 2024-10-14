#ifndef _INCLUDE_LEX_H
#define _INCLUDE_LEX_H

void needsemicolon (void);

void junk (void);

int endst (void);

void needbracket (const char *str);

int alpha (char c);

int numeric (char c);

int alphanum (char c);

int sstreq (const char *str1);

int streq (const char *str1, const char *str2);

int astreq (const char *str1, const char *str2, int len);

int match (const char *lit);

int amatch (const char *lit, int len);

void blanks (void);

extern int lex_stop_at_eol;

#endif
