#pragma once
#include "./gptl-customized/gptl.c"

int MYSTsetoption (const int option, const int val)
{
    return GPTLsetoption(option, val);
}

int MYSTinitialize (void)
{
    return GPTLinitialize();
}

int MYSTstart (const char *name)
{
    return GPTLstart(name);
}

int MYSTinit_handle (const char *name, int *handle)
{
    return GPTLinit_handle(name, handle);
}

int MYSTstart_handle (const char *name, int *handle)
{
    return GPTLstart_handle(name, handle);
}

int MYSTstop (const char *name)
{
    return GPTLstop(name);
}

int MYSTstop_handle (const char *name, int *handle)
{
    return GPTLstop_handle(name, handle);
}

int MYSTstamp (double *wall, double *usr, double *sys)
{
    return GPTLstamp(wall, usr, sys);
}

int MYSTpr (const int id)
{
    return GPTLpr(id);
}

int MYSTpr_file (const char *outfile)
{
    return GPTLpr_file(outfile);
}

int MYSTreset (void)
{
    return GPTLreset();
}

int MYSTreset_timer (const char *name)
{
    return GPTLreset_timer(name);
}

int MYSTfinalize (void)
{
    return GPTLfinalize();
}

int MYSTenable (void)
{
    return GPTLenable();
}

int MYSTdisable (void)
{
    return GPTLdisable();
}

int MYSTsetutr (const int option)
{
    return GPTLsetutr(option);
}

int MYSTquery (const char *name, int t, int *count, int *onflg, double *wallclock, double *dusr, double *dsys, long long *papicounters_out, const int maxcounters)
{
    return GPTLquery(name, t, count, onflg, wallclock, dusr, dsys, papicounters_out, maxcounters);
}

int MYSTget_wallclock (const char *timername, int t, double *value)
{
    return GPTLget_wallclock(timername, t, value);
}

int MYSTget_wallclock_latest (const char *timername, int t, double *value)
{
    return GPTLget_wallclock_latest(timername, t, value);
}

int MYSTget_threadwork (const char *name, double *maxwork, double *imbal)
{
    return GPTLget_threadwork(name, maxwork, imbal);
}

int MYSTstartstop_val (const char *name, double value)
{
    return GPTLstartstop_val(name, value);
}

int MYSTget_nregions (int t, int *nregions)
{
    return GPTLget_nregions(t, nregions);
}

int MYSTget_regionname (int t, int region, char *name, int nc)
{
    return GPTLget_regionname(t, region, name, nc);
}

int MYSTget_eventvalue (const char *timername, const char *eventname, int t, double *value)
{
    return GPTLget_eventvalue(timername, eventname, t, value);
}

int MYSTget_count (const char *timername, int t, int *count)
{
    return GPTLget_count(timername, t, count);
}
