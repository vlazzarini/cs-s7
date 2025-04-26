#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cs-s7.h"

int main(int argc, char **argv) {
  s7_scheme *s7 = s7_init();
  csound_interface(s7);

  while (1) {
    char buffer[512];
    fprintf(stdout, "\ncs-s7>");
    fgets(buffer, 512, stdin);
    if ((buffer[0] != '\n') ||
        (strlen(buffer) > 1)) {
      char response[1024];
      snprintf(response, 1024, "(write %s)", buffer);
      s7_eval_c_string(s7, response); 
    }
  }
  free(s7);
  return 0;
}
