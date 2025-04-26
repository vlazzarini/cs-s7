#if defined(__APPLE__)
#include <CsoundLib64/csound.h>
#include <CsoundLib64/csPerfThread.h>
#else
#include <csound/csound.h>
#include <csound/csPerfThread.h>
#endif
#include "s7.h"

void csound_interface(s7_scheme *sc); 
