#ifndef USER_H_
#define USER_H_
#include <stdlib.h>
#define NOB_STRIP_PREFIX
#include "../thirdparty/nob.h"

typedef struct Account {
  size_t currencyCode;
  size_t balance;
  const char *type;
  const char *maskedPan /*nullable*/;
} Account;

typedef struct Accounts {
  Account *items;
  size_t count;
  size_t capacity;
} Accounts;

typedef struct User {
  const char *name;
  Accounts accounts;
} User;

const char *currency_code_to_str(size_t currency_code);

User parse_user(String_Builder *sb);

void print_user(User *user);

void free_user(User *user);

#endif // USER_H_