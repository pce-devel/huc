#ifndef _INCLUDE_LEX_H
#define _INCLUDE_LEX_H

void ns (void);

void junk (void);

intptr_t endst (void);

void needbrack (char *str);

intptr_t alpha (char c);

intptr_t numeric (char c);

intptr_t an (char c);

intptr_t sstreq (char *str1);

intptr_t streq (char *str1, char *str2);

intptr_t astreq (char *str1, char *str2, intptr_t len);

intptr_t match (char *lit);

intptr_t amatch (char *lit, intptr_t len);

void blanks (void);

extern int lex_stop_at_eol;

#endif
