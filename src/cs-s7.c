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
    int32_t async = cs->perf ? 1 : 0;
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-compile",1,s7_car(args),
                                     "string");
    return s7_make_integer(sc, csoundCompileCSD(cs->csound,
                                                s7_string(s7_cadr(args)), 0,
                                                          async));
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
    int32_t async = cs->perf ? 1 : 0;
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-event-string",1,s7_car(args),
                                     "string");
    csoundEventString(cs->csound,s7_string(s7_cadr(args)), async);
    return s7_cadr(args);
  } else return cs_type_err(sc, args,"csound-event-string");
}

static s7_pointer event(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    int32_t async = cs->perf ? 1 : 0;
    s7_pointer s7p, argp;
    int32_t type = s7_integer(s7_cadr(args));
    MYFLT p[16] = {0};
    int32_t n = 0;
    argp = s7_cdr(s7_cdr(args));
    while((s7p = s7_car(argp))) {
      p[n++] = (MYFLT) s7_real(s7p);
      argp = s7_cdr(argp);
    }
    csoundEvent(cs->csound, type, p, n-1, async);
    return s7_cdr(args);
  } else return cs_type_err(sc, args,"csound-event");
}

static s7_pointer compile_string(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
    int32_t async = cs->perf ? 1 : 0;
    if(!s7_is_string(s7_cadr(args)))
      return s7_wrong_type_arg_error(sc,"csound-compile-string", 1 ,s7_car(args),
                                     "string");
    return s7_make_integer(sc, csoundCompileOrc(cs->csound,
                                                s7_string(s7_cadr(args)), async));
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

static s7_pointer perf_ksmps(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    int32_t res = -1; 
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    if(cs->perf == NULL) res = csoundPerformKsmps(cs->csound);
    return s7_make_integer(sc, res);
  } else return cs_type_err(sc, args,"csound-ksmps");
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
    bool async;
    if(cs->perf) return s7_make_integer(sc, -1);
    res = csoundStart(cs->csound);
    async = s7_boolean(sc, s7_cadr(args));
    if(res == CSOUND_SUCCESS && async){
       cs->perf = csoundCreatePerformanceThread(cs->csound);
       if(cs->perf) csoundPerformanceThreadPlay(cs->perf);
    } 
    return s7_make_integer(sc, res);
  } return cs_type_err(sc, args,"csound-start");
}

static s7_pointer stop(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    int32_t res;
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    if(cs->perf) {
      csoundPerformanceThreadStop(cs->perf);
      csoundPerformanceThreadJoin(cs->perf);
      csoundDestroyPerformanceThread(cs->perf);
      cs->perf = NULL;
    }
    csoundReset(cs->csound);
    csoundSetOption(cs->csound, "-odac");
    if((res = append_opcodes(cs->csound, sc))
       != CSOUND_SUCCESS)
      return s7_error(sc, s7_make_symbol(sc, "failed-csound-reset"),
                      s7_list(sc, 1,  s7_car(s7_make_integer(sc, res)))); 
    return s7_car(args);
  } return cs_type_err(sc, args,"csound-stop");
}

static s7_pointer is_async(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    bool async = false;
    if(cs->perf) async = true;
    return s7_make_boolean(sc, async);
  } return cs_type_err(sc, args,"csound-async?");
}

static s7_pointer toggle_pause(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    if(cs->perf) {
     csoundPerformanceThreadTogglePause(cs->perf);
     cs->pause = !cs->pause;
    }
    return s7_make_integer(sc, cs->pause ? 1 : 0);
  } return cs_type_err(sc, args,"csound-pause");
}

static s7_pointer is_paused(s7_scheme *sc, s7_pointer args) {
  if(cs_check(s7_car(args))) {
    cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
    return s7_make_boolean(sc, cs->pause);
  } return cs_type_err(sc, args,"csound-paused?");
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
    s7_define_function_star(sc,"csound-start", start,
                            "csound-obj (async #t)",
                            "(csound-start csound-obj (async 1)) "
                            "starts csound performance "
                            "(defaults to asynchronous)");
    s7_define_function(sc,"csound-stop",stop,1,0,false,
                       "(csound-stop csound-obj) starts csound performance");
    s7_define_function(sc,"csound-async?", is_async,1,0,false,
                       "(csound-async? csound-obj) returns async status");
    s7_define_function(sc,"csound-pause",toggle_pause,1,0,false,
                       "(csound-pause csound-obj) toggles performance pause");
    s7_define_function(sc,"csound-paused?",is_paused,1,0,false,
                       "(csound-paused? csound-obj) returns performance status");
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
   s7_define_function(sc,"csound-perform-ksmps", perf_ksmps, 1, 0, false,
                      "(csound-perform-ksmps csound-obj) "
                      "perform a ksmps-block of frames, synchronously.");
   s7_define_function(sc, "csound?", is_csobj, 1, 0, false,
                       "(csound? anything) "
                       "returns #t if its argument is a csound object");
  }
  return res;
}

#endif

/******************************************************************************/
/**
 * opcodes
 **/ 

typedef struct {
  s7_pointer obj;
} S7OBJ;

static void var_init_mem(CSOUND *csound, CS_VARIABLE* var, MYFLT* memblock) {
  memset(memblock, 0, var->memBlockSize);
}

static void s7obj_copy_value(CSOUND* csound, const CS_TYPE* cstype, void* dest,
                        const void* src, INSDS *ctx) {
  memcpy(dest, src, sizeof(S7OBJ));
}

static CS_VARIABLE* create_s7obj(void* cs, void* p, INSDS *ctx) {
    CSOUND* csound = (CSOUND*) cs;
    CS_VARIABLE* var = (CS_VARIABLE *)
      csound->Calloc(csound, sizeof(CS_VARIABLE));
    IGN(p);
    var->memBlockSize = CS_FLOAT_ALIGN(sizeof(S7OBJ));
    var->initializeVariableMemory = var_init_mem;
    var->ctx = ctx;
    return var;
}

CS_TYPE CS_VAR_TYPE_S7OBJ = {
   "S7obj", "S7obj", CS_ARG_TYPE_BOTH, create_s7obj, s7obj_copy_value,
    NULL, NULL, 0
};

static void add_s7obj(CSOUND *csound) {
  csound->AddVariableType(csound, csound->GetTypePool(csound),
                               &CS_VAR_TYPE_S7OBJ);                           
}

typedef struct {
  OPDS h;
  S7OBJ *out;
  STRINGDAT *code;
  s7_scheme *s7;
} OPCO;

typedef struct {
  OPDS h;
  STRINGDAT *code;
  S7OBJ *in;
  s7_scheme *s7;
} OPCI;

static int32_t  interp_call_myflt(CSOUND *csound, OPCO *p) {
  MYFLT *out = (MYFLT *) p->out;
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  s7_pointer res = s7_eval_c_string(p->s7, (const char*) p->code->data);
  if(s7_is_real(res)) *out = s7_real(res);
  else if(s7_is_integer(res)) *out = (MYFLT) s7_integer(res);
  else *out = 0.;
  return OK;
}

static int32_t define_var_myflt(CSOUND *csound, OPCI *p) {
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  s7_define_variable(p->s7, (const char*) p->code->data,
                     s7_make_real(p->s7, *((MYFLT *)p->in)));
  return OK;
}

static int32_t  interp_call(CSOUND *csound, OPCO *p) {
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  p->out->obj = s7_eval_c_string(p->s7, (const char*) p->code->data);
  return OK;
}

static int32_t define_var(CSOUND *csound, OPCI *p) {
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  if(p->in->obj)
    s7_define_variable(p->s7, (const char*) p->code->data,
                     p->in->obj);
  return OK;
}

typedef struct {
  OPDS h;
  S7OBJ *out;
  S7OBJ *in;
  s7_scheme *s7;
} OPCIO;

static int32_t car(CSOUND *csound, OPCIO *p) {
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  if(p->in->obj)
   p->out->obj = s7_car(p->in->obj);
  return OK;
}

static int32_t cdr(CSOUND *csound, OPCIO *p) {
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  if(p->in->obj)
   p->out->obj = s7_cdr(p->in->obj);
  return OK;
}

static int32_t real(CSOUND *csound, OPCIO *p) {
  MYFLT *out = (MYFLT *) p->out;
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  if(p->in->obj)
  *out = s7_real(p->in->obj);
  else *out = 0.;
  return OK;
}

static int32_t make_real(CSOUND *csound, OPCIO *p) {
  if(p->s7 == NULL)
    p->s7 = *((s7_scheme **) csound->QueryGlobalVariable(csound, "_S7_"));
  p->out->obj = s7_make_real(p->s7, *((MYFLT *)p->in));
  return OK;
}

static int32_t save_to_global(CSOUND *csound, s7_scheme *s7) {
  if(csound->QueryGlobalVariable(csound, "_S7_") == NULL) {
    if(csound->CreateGlobalVariable(csound, "_S7_", sizeof(s7_scheme *))
       == CSOUND_SUCCESS) {
      s7_scheme **s7p = (s7_scheme **)
        csound->QueryGlobalVariable(csound, "_S7_");
      *s7p = s7;
      return OK;
    }
    else return NOTOK;
  }
  return OK;
}

int32_t append_opcodes(CSOUND *csound, s7_scheme *s7) {
  int32_t res;
  add_s7obj(csound);
  res = csound->AppendOpcode(csound, "s7definevar", sizeof(OPCI), 0,
                           "", "Si", (SUBR) define_var_myflt, NULL, NULL);
  res += csound->AppendOpcode(csound, "s7eval", sizeof(OPCO), 0,
                           "i", "S", (SUBR) interp_call_myflt, NULL, NULL);
  res += csound->AppendOpcode(csound, "s7definevar", sizeof(OPCI), 0,
                          "", "Sk", NULL, (SUBR) define_var_myflt, NULL);
  res += csound->AppendOpcode(csound, "s7eval", sizeof(OPCO), 0,
                             "k", "S", NULL, (SUBR) interp_call_myflt, NULL);
  res += csound->AppendOpcode(csound, "s7eval", sizeof(OPCO), 0,
                             ":S7obj;", "S", (SUBR) interp_call,
                              (SUBR) interp_call, NULL);
  res += csound->AppendOpcode(csound, "s7definevar", sizeof(OPCI), 0,
                            "", "S:S7obj;", (SUBR) define_var,
                              (SUBR) define_var, NULL);
  res += csound->AppendOpcode(csound, "s7car", sizeof(OPCIO), 0,
                             ":S7obj;", ":S7obj;", (SUBR) car, (SUBR) car,
                              NULL);
  res += csound->AppendOpcode(csound, "s7cdr", sizeof(OPCIO), 0,
                             ":S7obj;", ":S7obj;", (SUBR) car, (SUBR) car,
                              NULL);
  res += csound->AppendOpcode(csound, "s7real", sizeof(OPCIO), 0,
                             "i", ":S7obj;", (SUBR) real, NULL, NULL);
  res += csound->AppendOpcode(csound, "s7real", sizeof(OPCIO), 0,
                             "k", ":S7obj;", NULL, (SUBR) real, NULL);  
  res += csound->AppendOpcode(csound, "s7real", sizeof(OPCIO), 0,
                             ":S7obj;", "i", (SUBR) make_real, NULL, NULL);
  res += csound->AppendOpcode(csound, "s7real", sizeof(OPCIO), 0,
                             ":S7obj;", "k", NULL, (SUBR) make_real, NULL);
  if(s7 != NULL)
    res += save_to_global(csound, s7);  
  return res;
}

int32_t csoundModuleCreate(CSOUND *csound) {
  return OK;
}

int32_t csoundModuleInit(CSOUND *csound) {
  if(csound->QueryGlobalVariable(csound, "_S7_") != NULL) return OK;
  if(csound->CreateGlobalVariable(csound, "_S7MOD_", 1) != CSOUND_SUCCESS) {
    return NOTOK;
  }
  if(save_to_global(csound, s7_init()) == CSOUND_SUCCESS){
    return append_opcodes(csound, NULL);
  }
  else return NOTOK;
}

int32_t csoundModuleDestroy(CSOUND *csound)
{
  if(csound->QueryGlobalVariable(csound, "_S7MOD_")) {
    s7_scheme *s7 = *((s7_scheme **)
                     csound->QueryGlobalVariable(csound, "_S7_"));
    if(s7 != NULL) s7_free(s7);
    csound->DestroyGlobalVariable(csound, "_S7_");
    csound->DestroyGlobalVariable(csound, "_S7MOD_");
   }
  return OK;
}

int32_t csoundModuleInfo(void)
{
    return ((CS_VERSION << 16) + (CS_SUBVER << 8) + (int32_t) sizeof(MYFLT));
}
