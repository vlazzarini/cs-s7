/******************************************************************************/
//
// cs-s7.c: Csound S7 scheme interface
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
#include "cs-s7.h"

#ifndef BUILDING_MODULE

static int32_t append_opcodes(CSOUND *csound, s7_scheme *s7); 
static int cs_type_tag = 0;
typedef struct {
  CSOUND *csound;
  CS_PERF_THREAD *perf;  
  bool pause; // perf pause flag
} cs_obj;

static inline bool cs_check(s7_pointer obj){
  cs_obj *cs = (cs_obj *) s7_c_object_value(obj);
  return (s7_is_c_object(obj) &&
          s7_c_object_type(obj) == cs_type_tag);
}

static inline s7_pointer cs_type_err(s7_scheme *sc, s7_pointer args,
                                     const char* caller) {
  return s7_wrong_type_arg_error(sc, caller, 0, s7_car(args), "csound-obj");
}
  
static s7_pointer create(s7_scheme *sc, s7_pointer args) {
  int32_t res;
  cs_obj *cs = (cs_obj *) calloc(1, sizeof(cs_obj));
  cs->csound = csoundCreate(cs, NULL);
  if(cs->csound != NULL) {
    if((res = append_opcodes(cs->csound, sc)) == CSOUND_SUCCESS) {
      csoundSetOption(cs->csound, "-odac");
      cs->perf = NULL;
      cs->pause = false;
      return s7_make_c_object(sc, cs_type_tag, (void *) cs);
    }
  } else res = -1;
  return s7_error(sc, s7_make_symbol(sc, "failed-csound-create"),
                  s7_list(sc, 1,  s7_car(s7_make_integer(sc, res))));
}

static s7_pointer compile(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    if(cs->perf) return s7_make_integer(sc, -1);
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-compile",1,s7_car(args),
                                     "string");
    const char *argv[] = {"csound", s7_string(s7_cadr(args))};
    return s7_make_integer(sc, csoundCompile(cs->csound,2,argv));
  } else return cs_type_err(sc, args, "csound-compile");
}

static s7_pointer options(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-options",1,s7_car(args),
                                     "string"); 
    return s7_make_integer(sc,csoundSetOption(cs->csound,
                                              s7_string(s7_cadr(args))));
  } else return cs_type_err(sc, args,"csound-options");
}

static s7_pointer event_string(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-event-string",1,s7_car(args),
                                     "string");
    csoundEventString(cs->csound,s7_string(s7_cadr(args)), 1);
    return s7_cadr(args);
  } else return cs_type_err(sc, args,"csound-event-string");
}

static s7_pointer event(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    s7_pointer s7p, argp;
    int32_t type = s7_integer(s7_cadr(args));
    MYFLT p[16] = {0};
    int32_t n = 0;
    argp = s7_cdr(s7_cdr(args));
    while((s7p = s7_car(argp))) {
      p[n++] = (MYFLT) s7_real(s7p);
      argp = s7_cdr(argp);
    }
    csoundEvent(cs->csound, type, p, n-1, 1);
    return s7_cdr(args);
  } else return cs_type_err(sc, args,"csound-event");
}

static s7_pointer compile_string(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    int32_t asc;
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-compile-string", 1 ,s7_car(args),
                                     "string");
    return s7_make_integer(sc, csoundCompileOrc(cs->csound,
                                                s7_string(s7_cadr(args)), 1));
  } else return cs_type_err(sc, args, "csound-compile-string");
}

static s7_pointer get_channel(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-get-channel",1,s7_car(args),
                                     "string");
    return s7_make_real(sc, csoundGetControlChannel(cs->csound,
                                                   s7_string(s7_cadr(args)),
                                                   NULL));
  } else return cs_type_err(sc, args,"csound-get-channel");
}

static s7_pointer set_channel(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    s7_pointer pval = s7_cadr(s7_cdr(args));
    MYFLT val;
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-set-channel",1,s7_car(args),
                                     "string");
    val = (MYFLT) s7_number_to_real_with_caller(sc, pval, "csound-set-channel");  
    csoundSetControlChannel(cs->csound, s7_string(s7_cadr(args)), val);   
    return s7_make_real(sc,s7_real(pval));
    
  } else return cs_type_err(sc, args,"csound-set-channel");
}

static s7_pointer perf_time(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    int64_t time_frames = csoundGetCurrentTimeSamples(cs->csound);
    double time_secs = time_frames/csoundGetSr(cs->csound);                
    return s7_list(sc, 2, s7_make_real(sc,time_secs),
                   s7_make_integer(sc,time_frames));
  } else return cs_type_err(sc, args,"csound-time");
}

static s7_pointer start(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    int32_t res;
    if(cs->perf) return s7_make_integer(sc, -1);
    res = csoundStart(cs->csound);
    if(s7_cadr(args)) printf("extra arg\n");
    if(res == CSOUND_SUCCESS){
       cs->perf = csoundCreatePerformanceThread(cs->csound);
       if(cs->perf) csoundPerformanceThreadPlay(cs->perf);
    } else return NULL;
    return s7_make_integer(sc, res);
  } return cs_type_err(sc, args,"csound-start");
}

static s7_pointer stop(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    if(cs->perf) {
      int32_t res;
      csoundPerformanceThreadStop(cs->perf);
      csoundPerformanceThreadJoin(cs->perf);
      csoundDestroyPerformanceThread(cs->perf);
      cs->perf = NULL;
      csoundReset(cs->csound);
      csoundSetOption(cs->csound, "-odac");
      if((res = append_opcodes(cs->csound, sc))
         != CSOUND_SUCCESS)
        return s7_error(sc, s7_make_symbol(sc, "failed-csound-reset"),
                  s7_list(sc, 1,  s7_car(s7_make_integer(sc, res))));
    } 
    return s7_car(args);
  } return cs_type_err(sc, args,"csound-stop");
}

static s7_pointer toggle_pause(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    if(cs->perf) {
     csoundPerformanceThreadTogglePause(cs->perf);
     cs->pause = !cs->pause;
    }
    return s7_make_boolean(sc, cs->pause);
  } return cs_type_err(sc, args,"csound-pause");
}

static s7_pointer free_csobj(s7_scheme *sc, s7_pointer obj){
  cs_obj *cs = (cs_obj *) s7_c_object_value(obj);
  if(cs->perf) {
    csoundPerformanceThreadStop(cs->perf);
    csoundPerformanceThreadJoin(cs->perf);
    csoundDestroyPerformanceThread(cs->perf);
  }
  csoundDestroy(cs->csound);
  free(s7_c_object_value(obj));
  return NULL;
}

static s7_pointer is_csobj(s7_scheme *sc, s7_pointer args){
  return s7_make_boolean(sc,s7_is_c_object(s7_car(args)) &&
			 s7_c_object_type(s7_car(args)) == cs_type_tag);
}

static s7_pointer csobj_is_equal(s7_scheme *sc, s7_pointer args){
  cs_obj *obj1, *obj2;
  s7_pointer p1 = s7_car(args);
  s7_pointer p2 = s7_cadr(args);
  if (p1 == p2)
    return(s7_t(sc));
  if ((!s7_is_c_object(p2)) ||
      (s7_c_object_type(p2) != cs_type_tag))
    return s7_f(sc);
  obj1 = (cs_obj *) s7_c_object_value(p1);
  obj2 = (cs_obj *) s7_c_object_value(p2);
  return s7_make_boolean(sc, (obj1->csound == obj2->csound));
}



/**
 *   s7 interpreter interface 
 **/
int32_t cs_s7(s7_scheme *sc) {
  int32_t res = csoundInitialize(0);
  if(res == CSOUND_SUCCESS) {
    cs_type_tag = s7_make_c_type(sc, "csound-obj");
    s7_c_type_set_gc_free(sc,cs_type_tag,free_csobj);
    s7_c_type_set_is_equal(sc,cs_type_tag,csobj_is_equal);
    s7_define_function(sc,"make-csound",create,0,0,false,
                       "(make-csound) creates a csound-obj");
    s7_define_function(sc,"csound-start",start,1,1,false,
                       "(csound-start csound-obj) starts csound performance");
    s7_define_function(sc,"csound-stop",stop,1,0,false,
                       "(csound-stop csound-obj) starts csound performance");
    s7_define_function(sc,"csound-pause",toggle_pause,1,0,false,
                       "(csound-pause csound-obj) toggles performance pause");
    s7_define_function(sc,"csound-compile",compile,2,0,false,
                       "(csound-compile csound_obj filename) "
                       "compiles a CSD file");
    s7_define_function(sc,"csound-options", options, 2, 0, false,
                       "(csound-options csound_obj opt-string) "
                       "sets engine options");
    s7_define_function(sc,"csound-event-string",event_string,2,0,false,
                       "(csound-event-string csound_obj evt-string) "
                       "sends an event string");
    s7_define_function(sc,"csound-event",event,5, 251, false,
                       "(csound-event csound_obj type p1 p2 p3 ...) "
                       "sends an event");
    s7_define_function(sc,"csound-compile-string",compile_string,2,0,false,
                       "(csound-compile-string csound_obj code-string) "
                       "compiles a code string");
    s7_define_function(sc,"csound-get-channel",get_channel,2,0,false,
                       "(csound-get-channel csound_obj channel)"
                       " gets data from bus channel");
    s7_define_function(sc,"csound-set-channel",set_channel,3,0,false,
                       "(csound-set-channel csound_obj channel val)"
                       " sets bus channel to val");
   s7_define_function(sc,"csound-time", perf_time, 1, 0, false,
                       "(csound-time csound-obj) "
                       "returns the current performance time "
                       "as a list (secs frames)");
   s7_define_function(sc, "csound?", is_csobj, 1, 0, false,
                       "(csound? anything) "
                       "returns #t if its argument is a csound object");
  }
  return res;
}

#endif

/******************************************************************************/
// opcodes
// 
//
/******************************************************************************/

typedef struct {
  OPDS h;
  MYFLT *out;
  STRINGDAT *code;
} OPCO;

typedef struct {
  OPDS h;
  STRINGDAT *code;
  MYFLT *in;
} OPCI;

static int32_t  interp_call(CSOUND *csound, OPCO *p) {
  s7_scheme *s7 = *((s7_scheme **)
                     csound->QueryGlobalVariable(csound, "_S7_"));
  s7_pointer res = s7_eval_c_string(s7, (const char*) p->code->data);
  if(s7_is_real(res)) *p->out = s7_real(res);
  else if(s7_is_integer(res)) *p->out = (MYFLT) s7_integer(res);
  else *p->out = 0.;
  return OK;
}

static int32_t define_var(CSOUND *csound, OPCI *p) {
  s7_scheme *s7 = *((s7_scheme **)
                     csound->QueryGlobalVariable(csound, "_S7_"));
  s7_define_variable(s7, (const char*) p->code->data, s7_make_real(s7, *p->in));
  return OK;
}


int32_t append_opcodes(CSOUND *csound, s7_scheme *s7) {
  int32_t res;
  res = csound->AppendOpcode(csound, "s7eval", sizeof(OPCO), 0,
                           "i", "S", (SUBR) interp_call, NULL, NULL);
  res = csound->AppendOpcode(csound, "s7definevar", sizeof(OPCI), 0,
                           "", "Si", (SUBR) define_var, NULL, NULL);

  if(csound->CreateGlobalVariable(csound, "_S7_", sizeof(s7_scheme *))
     == CSOUND_SUCCESS) {
    s7_scheme **s7p = (s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_");
    *s7p = s7;
  } else {
    csound->ErrorMsg(csound, "could not allocate global var for s7 interpreter\n");
    return NOTOK;
  }
  return OK;
}

int32_t csoundModuleCreate(CSOUND *csound) {
  // check to see if module has already been loaded
  if(csound->QueryGlobalVariable(csound, "_S7_") != NULL) return CSOUND_ERROR;
  return OK;
}

int32_t csoundModuleInit(CSOUND *csound) {
    return append_opcodes(csound, s7_init());
}

int32_t csoundModuleDestroy(CSOUND *csound)
{
    s7_scheme *s7 = *((s7_scheme **)
                     csound->QueryGlobalVariable(csound, "_S7_"));
    if(s7 != NULL) s7_free(s7);
    csound->DestroyGlobalVariable(csound, "_S7_");
    return OK;
}

int32_t csoundModuleInfo(void)
{
    return ((CS_VERSION << 16) + (CS_SUBVER << 8) + (int32_t) sizeof(MYFLT));
}
