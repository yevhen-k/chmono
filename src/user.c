#include "../thirdparty/json.h"
#include "user.h"
#include <stdio.h>
#include <stdlib.h>

#define NOB_STRIP_PREFIX
#include "../thirdparty/nob.h"

const char *currency_code_to_str(size_t currency_code) {
  if (currency_code == 980) {
    return "UAH";
  } else if (currency_code == 840) {
    return "USD";
  } else {
    UNREACHABLE("Invalid currency\n");
  }
}

struct json_value_s *get_val(struct json_object_s *dict, const char *key) {
  assert(dict);
  assert(key);
  // printf("num elems in acc object: %li\n", dict->length);
  struct json_object_element_s *item = NULL;
  for (size_t i = 0; i < dict->length; ++i) {
    if (i == 0) {
      item = dict->start;
    } else {
      item = item->next;
    }
    assert(item != NULL);
    const char *item_key = item->name->string;
    if (strcmp(item_key, key) == 0) {
      return item->value;
    }
  }
  return NULL;
}

User parse_user(String_Builder *sb) {
  User user = {0};

  // const char *json = wb.response;
  struct json_value_s *root = json_parse(sb->items, sb->count);
  assert(root->type == json_type_object);

  struct json_object_s *object = json_value_as_object(root);
  assert(object->length == 6);
  // printf("object len = %zu\n", object->length);

  const char *key = "name";
  struct json_value_s *val = get_val(object, key);
  assert(val);
  assert(val->type == json_type_string);
  // printf("%s: %s\n", key, ((struct json_string_s *)val->payload)->string);
  user.name = strdup(((struct json_string_s *)val->payload)->string);

  key = "accounts";
  val = get_val(object, key);
  assert(val);
  assert(val->type == json_type_array);
  // printf("%s: %lu\n", key, ((struct json_array_s *)val->payload)->length);
  struct json_array_s *array = json_value_as_array(val);
  struct json_array_element_s *array_elems;
  for (size_t i = 0; i < array->length; ++i) {
    if (i == 0) {
      array_elems = array->start;
    } else {
      array_elems = array_elems->next;
    }
    assert(array_elems != NULL);

    object = json_value_as_object(array_elems->value);
    assert(object != NULL);

    Account account = {0};

    key = "currencyCode";
    val = get_val(object, key);
    assert(val);
    assert(val->type == json_type_number);
    // printf("\t%s: %s\n", key, ((struct json_number_s
    // *)val->payload)->number);
    account.currencyCode = atoi(((struct json_number_s *)val->payload)->number);

    key = "balance";
    val = get_val(object, key);
    assert(val);
    assert(val->type == json_type_number);
    // printf("\t%s: %s\n", key, ((struct json_number_s
    // *)val->payload)->number);
    account.balance = atoi(((struct json_number_s *)val->payload)->number);

    key = "type";
    val = get_val(object, key);
    assert(val);
    assert(val->type == json_type_string);
    // printf("\t%s: %s\n", key, ((struct json_string_s
    // *)val->payload)->string);
    account.type = strdup(((struct json_string_s *)val->payload)->string);

    key = "maskedPan";
    val = get_val(object, key);
    assert(val);
    assert(val->type == json_type_array);
    size_t maskedPan_len = ((struct json_array_s *)val->payload)->length;
    // printf("\t%s len: %lu\n", key, maskedPan_len);
    if (maskedPan_len == 0) {
      // printf("-----------\n");
      account.maskedPan = NULL;
      da_append(&user.accounts, account);
      continue;
    }
    struct json_array_s *maskedPan_array = json_value_as_array(val);
    assert(maskedPan_array != NULL);
    struct json_array_element_s *maskedPan_elems = maskedPan_array->start;
    struct json_string_s *maskedPan_val =
        (struct json_string_s *)maskedPan_elems->value->payload;
    // printf("\t%s: %s\n", key, maskedPan_val->string);
    account.maskedPan = strdup(maskedPan_val->string);

    da_append(&user.accounts, account);

    // printf("-----------\n");
  }
  free(root);

  return user;
}

void print_user(User *user) {
  printf("User: %s:\n", user->name);
  printf("Accounts:\n");
  for (size_t i = 0; i < user->accounts.count; ++i) {
    printf("%zu)\tcurrencyCode: %s\n", i,
           currency_code_to_str(user->accounts.items[i].currencyCode));
    printf("\tbalance: %lu\n", user->accounts.items[i].balance);
    printf("\ttype: %s\n", user->accounts.items[i].type);
    if (user->accounts.items[i].maskedPan != NULL) {
      printf("\tmaskedPan: %s\n", user->accounts.items[i].maskedPan);
    } else {
      printf("\tmaskedPan: %s\n", "NOT AVAILABLE");
    }
  }
}

void free_user(User *user) {
  for (size_t i = 0; i < user->accounts.count; ++i) {
    free((void *)user->accounts.items[i].type);
    free((void *)user->accounts.items[i].maskedPan);
  }
  da_free(user->accounts);
  free((void *)(user->name));
}
