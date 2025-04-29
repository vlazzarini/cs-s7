/******************************************************************************/
//
// main.c: Csound s7 scheme interpreter main program
// (c) V Lazzarini, 2025
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef USE_TECLA
#include <libtecla.h>
#endif

#include "cs-s7.h"

static s7_scheme *s7;
void *tp = NULL;

static void bye() {
  s7_free(s7);
#ifdef USE_TECLA  
  if(tp != NULL) del_GetLine(tp);
#endif  
  fprintf(stderr, "cs-s7: finished.\n");
}

int main(int argc, char **argv) {

#ifdef USE_TECLA
  GetLine *gl = new_GetLine(500, 5000);
  tp = gl;
#endif  
  s7 = s7_init();
  if(cs_s7(s7) == CSOUND_SUCCESS) {
    atexit(bye);
    fprintf(stdout, "cs-s7: Csound s7 scheme interpreter");
    while (1) {
#ifdef USE_TECLA
      char *buffer;
      fprintf(stdout, "\n");
      buffer = gl_get_line(gl, "cs-s7> ", NULL, 0);
#else      
      char buffer[512]; 
      fprintf(stdout, "\ncs-s7>");
      fgets(buffer, 512, stdin);
#endif      
      if ((buffer[0] != '\n') ||
          (strlen(buffer) > 1)) {
        char response[1024];
        snprintf(response, 1024, "(write %s)", buffer);
        s7_eval_c_string(s7, response);
      }
    }
  }
  return 0;
}
