/******************************************************************************/
#include "cs-s7.h"

static int cs_type_tag = 0;
typedef struct {
  CSOUND *csound;
  CS_PERF_THREAD *perf;
  bool running;
  bool pause;
} cs_obj;
  
static s7_pointer create(s7_scheme *sc, s7_pointer args) {
  cs_obj *cs = (cs_obj *) calloc(1, sizeof(cs_obj));
  cs->csound = csoundCreate(cs, NULL);
  cs->perf = csoundCreatePerformanceThread(cs->csound);
  return  s7_make_c_object(sc, cs_type_tag, (void *) cs);
}

static s7_pointer compile(s7_scheme *sc, s7_pointer args) {
  cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(args));
  const char *argv[] = { "csound", s7_string(s7_cadr(args)) };
  return s7_make_integer(sc, csoundCompile(cs->csound,2,argv));
}

static s7_pointer start(s7_scheme *sc, s7_pointer args) {
  cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
  int32_t res;
  if(cs->running) res = CSOUND_SUCCESS;
  else {
    res = csoundStart(cs->csound);
    if(res == CSOUND_SUCCESS)
     cs->running = true;
  }
  if(cs->running) 
    csoundPerformanceThreadPlay(cs->perf);
  return s7_make_integer(sc, res);
}

static s7_pointer stop(s7_scheme *sc, s7_pointer args) {
  cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
  csoundPerformanceThreadStop(cs->perf);
  return s7_make_boolean(sc, false);
}

static s7_pointer toggle_pause(s7_scheme *sc, s7_pointer args) {
  cs_obj *cs  = (cs_obj *) s7_c_object_value(s7_car(args));
  csoundPerformanceThreadTogglePause(cs->perf);
  cs->pause = !cs->pause;
  return s7_make_boolean(sc, cs->pause);
}

static s7_pointer free_csobj(s7_scheme *sc, s7_pointer obj)
{
  cs_obj *cs = (cs_obj *) s7_c_object_value(s7_car(obj));
  csoundPerformanceThreadStop(cs->perf);
  csoundPerformanceThreadJoin(cs->perf);
  csoundDestroyPerformanceThread(cs->perf);
  csoundDestroy(cs->csound);
  free(s7_c_object_value(obj));
  return NULL;
}

static s7_pointer is_csobj(s7_scheme *sc, s7_pointer args)
{
  return s7_make_boolean(sc,
			 s7_is_c_object(s7_car(args)) &&
			 s7_c_object_type(s7_car(args)) == cs_type_tag);
}

static s7_pointer csobj_is_equal(s7_scheme *sc, s7_pointer args)
{
  cs_obj *d1, *d2;
  s7_pointer p1 = s7_car(args);
  s7_pointer p2 = s7_cadr(args);
  if (p1 == p2)
    return(s7_t(sc));
  if ((!s7_is_c_object(p2)) ||
      (s7_c_object_type(p2) != cs_type_tag))
    return s7_f(sc);
  d1 = (cs_obj *) s7_c_object_value(p1);
  d2 = (cs_obj *) s7_c_object_value(p2);
  return s7_make_boolean(sc, (d1->csound == d2->csound));
}


void csound_interface(s7_scheme *sc) {
   cs_type_tag = s7_make_c_type(sc, "csound");
   s7_c_type_set_gc_free(sc, cs_type_tag, free_csobj);
   s7_c_type_set_is_equal(sc, cs_type_tag, csobj_is_equal);
   s7_define_function(sc, "make-csound", create, 0, 0, false,
                      "(make-csound) creates a csound object");
   s7_define_function(sc, "compile-csound", compile, 2, 0, false,
                      "(compile-csound csound_obj str) compiles a CSD file");
   s7_define_function(sc, "start-csound", start, 1, 0, false,
                      "(start-csound csound-obj) starts csound performance");
   s7_define_function(sc, "stop-csound", stop, 1, 0, false,
                      "(stop-csound csound-obj) starts csound performance");
   s7_define_function(sc, "pause-csound", toggle_pause, 1, 0, false,
                      "(pause-csound csound-obj) pause/play csound performance");
   s7_define_function(sc, "csound?", is_csobj, 1, 0, false,
                      "(csound? anything) "
                      "returns #t if its argument is a csound object");

}
/******************************************************************************/
