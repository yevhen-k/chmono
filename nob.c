#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "thirdparty/nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

void usage() {
  printf("One of the following must be specified as an argument:\n"
         "     - -ggdb\n"
         "     - -O1\n"
         "     - -O2\n"
         "     - -O3\n");
}

bool is_valid_arg(const char *arg) {
  bool res = false;
  const size_t size = 4;
  const char *valid[] = {"-ggdb", "-O1", "-O2", "-O3"};
  for (size_t i = 0; i < size; ++i) {
    if (strcmp(arg, valid[i]) == 0)
      return true;
  }
  return res;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  shift(argv, argc);
  if (argc != 1 || !is_valid_arg(argv[0])) {
    usage();
    return 1;
  }
  if (!mkdir_if_not_exists(BUILD_DIR))
    return 1;
  Cmd cmd = {0};
  nob_log(INFO, "Using %s", argv[0]);
  cmd_append(&cmd, "cc", "-Wall", "-Wextra", argv[0], "-o", BUILD_DIR "chmono",
             SRC_DIR "main.c", SRC_DIR "user.c", "-lcurl",
             "-I../raylib-5.5_linux_amd64/include/",
             "-L../raylib-5.5_linux_amd64/lib/", "-l:libraylib.a",
             "-I../raygui/src/", "-L../raygui/", "-l:raygui.so",
             "-Wl,-rpath=../raygui/", "-I../raygui/styles/amber/", "-lm");
  if (!cmd_run_sync(cmd))
    return 1;

  // Just because my CS prof forced me:
  cmd_free(cmd);
  return 0;
}
