#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "style_amber.h"

#include "user.h"
#include <assert.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../thirdparty/nob.h"

static size_t sb_write_callback(char *data, size_t size, size_t nmemb,
                                void *clientp) {
  size_t realsize = size * nmemb;
  String_Builder *sb = (String_Builder *)clientp;
  sb_append_buf(sb, data, realsize);
  return realsize;
}

CURLcode request_client_info(String_Builder *sb,
                             size_t (*callback)(char *, size_t, size_t,
                                                void *)) {

  CURLcode res;
  CURL *curl = curl_easy_init();

  const char *MONO_X_TOKEN_HEADER = getenv("MONO_X_TOKEN_HEADER");
  const char *CLIENT_INFO_URL = getenv("CLIENT_INFO_URL");
  if (MONO_X_TOKEN_HEADER == NULL || CLIENT_INFO_URL == NULL) {
    fprintf(stderr, "ERROR: either MONO_X_TOKEN_HEADER or CLIENT_INFO_URL env "
                    "var undefined. Exiting.\n");
    exit(1);
  }

  struct curl_slist *slist;
  slist = NULL;
  slist = curl_slist_append(slist, MONO_X_TOKEN_HEADER);

  if (curl) {

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)sb);

    curl_easy_setopt(curl, CURLOPT_URL, CLIENT_INFO_URL);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "chmono/0.0.1");
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return res;
  } else {
    fprintf(stderr, "ERROR: failed to initialize CURL");
    exit(1);
  }
}

void debug_user() {
  User user = {0};
  const char *filename = "./client_data.json";
  String_Builder sb = {0};
  if (!read_entire_file(filename, &sb))
    exit(1);
  user = parse_user(&sb);
  print_user(&user);
  sb_free(sb);
  free_user(&user);
}

char *get_current_time() {
  time_t raw_time;
  time(&raw_time);
  char *last_update = asctime(localtime(&raw_time));
  // remove \n
  last_update[strlen(last_update) - 1] = '\0';
  return last_update;
}

int main() {

  User user = {0};
  String_Builder sb = {0};
  CURLcode ret = request_client_info(&sb, sb_write_callback);
  if (ret != 0) {
    fprintf(stderr, "ERROR: failed to get API response.\n");
    return ret;
  }
  user = parse_user(&sb);

  int screen_width = 400;
  int screen_height = 384;

  float margin = 5.0;

  // SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(screen_width, screen_height, "ChMono - Can't do shit");

  int font_size = 24;
  float spacing = 2.0f;
  int codepoints[512] = {0};
  int codepointCount = 0;
  // Add basic ASCII characters (32-126)
  for (int i = 0; i < 95; i++) {
    codepoints[codepointCount++] = 32 + i;
  }

  // Add Cyrillic characters (0x400 - 0x4FF)
  for (int i = 0; i < 256; i++) // 0x400 to 0x4FF covers 256 characters
  {
    codepoints[codepointCount++] = 0x400 + i;
  }
  Font font = LoadFontEx("./assets/UbuntuMono-B.ttf", font_size, codepoints,
                         codepointCount);

  const char *user_label = temp_sprintf("[%s]", user.name);
  size_t num_accounts = user.accounts.count;
  int curr_acc_idx = 0;
  const char *account_balance_group =
      temp_sprintf("Account Balance[%i/%zu]", curr_acc_idx + 1, num_accounts);

  char *last_update = get_current_time();

  bool hide_balance = false;
  bool update_button = false;
  bool prev_card = false;
  bool next_card = false;

  GuiLoadStyleAmber();
  GuiSetFont(font);
  GuiSetStyle(DEFAULT, TEXT_SIZE, 22);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    DrawTextEx(font, user_label,
               (Vector2){
                   8,
                   0 + margin,
               },
               font_size, spacing, RAYWHITE);

    GuiLine(
        (Rectangle){0 + margin, 16 + 2 * margin, screen_width - 2 * margin, 16},
        NULL);

    GuiCheckBox((Rectangle){8, 32 + 2 * margin, 24, 24},
                temp_sprintf("#%s# Hide Balance", hide_balance ? "45" : "44"),
                &hide_balance);

    update_button = GuiButton((Rectangle){8, 72 + margin, 24, 24}, "#211#");
    GuiLabel((Rectangle){40, 72 + margin, 120, 24}, "Update");
    if (update_button) {
      last_update = get_current_time();
      sb.count = 0;
      free_user(&user);
      ret = request_client_info(&sb, sb_write_callback);
      if (ret != 0) {
        fprintf(stderr, "ERROR: failed to get API response.\n");
        return ret;
      }
      user = parse_user(&sb);
    }

    GuiGroupBox((Rectangle){0 + margin, 128, screen_width - 2 * margin, 224},
                account_balance_group);
    next_card = GuiButton((Rectangle){screen_width - 50, 112, 32, 32}, "#119#");
    if (next_card) {
      curr_acc_idx++;
      curr_acc_idx = curr_acc_idx % num_accounts;
      account_balance_group = temp_sprintf("Account Balance[%i/%zu]",
                                           curr_acc_idx + 1, num_accounts);
    }
    prev_card = GuiButton((Rectangle){screen_width - 90, 112, 32, 32}, "#118#");
    if (prev_card) {
      curr_acc_idx--;
      if (curr_acc_idx == -1) {
        curr_acc_idx = num_accounts - 1;
      }
      account_balance_group = temp_sprintf("Account Balance[%i/%zu]",
                                           curr_acc_idx + 1, num_accounts);
    }

    GuiLabel((Rectangle){24, 168, 288, 24},
             temp_sprintf("Type: %s", user.accounts.items[curr_acc_idx].type));

    if (hide_balance) {
      GuiLabel((Rectangle){24, 216, 288, 24}, "Balance: XXX'XXX'XXX.XX");
    } else {
      GuiLabel(
          (Rectangle){24, 216, 288, 24},
          temp_sprintf("Balance: %i.%zu %s",
                       (int)(user.accounts.items[curr_acc_idx].balance / 100),
                       user.accounts.items[curr_acc_idx].balance % 100,
                       currency_code_to_str(
                           user.accounts.items[curr_acc_idx].currencyCode)));
    }

    if (strcmp(user.accounts.items[curr_acc_idx].type, "fop") != 0) {
      GuiLabel((Rectangle){24, 264, 288, 24},
               temp_sprintf("Card: %s",
                            user.accounts.items[curr_acc_idx].maskedPan));
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    GuiStatusBar((Rectangle){0, 360, screen_width, 24},
                 temp_sprintf("Last Update: %s", last_update));
    GuiSetStyle(DEFAULT, TEXT_SIZE, 22);

    EndDrawing();
  }

  CloseWindow();

  UnloadFont(font);
  sb_free(sb);
  free_user(&user);

  return 0;
}
