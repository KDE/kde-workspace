/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2005 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
 *
 * obtain configuration data
 */

#include "dm.h"
#include "dm_error.h"

#include <sys/stat.h>

#define WANT_CORE_DEFS
#include <config.ci>

static char **originalArgv;

static GProc getter;
GTalk cnftalk;

static void
openGetter()
{
    gSet(&cnftalk);
    if (!getter.pid) {
        if (gOpen(&getter,
                  originalArgv, "_config", 0, strdup("config reader"),
                  "", 0, 0))
            logPanic("Cannot run config reader\n");
        debug("getter now ready\n");
    }
}

void
closeGetter()
{
    if (getter.pid) {
        gSet(&cnftalk);
        (void)gClose(&getter, 0, False);
        debug("getter now closed\n");
    }
}

/*
 * ref-counted, unique-instance strings
 */
static RcStr *strs;

/*
 * make a ref-counted string of the argument. the new string will
 * have a ref-count of 1. the passed string pointer is no longer valid.
 */
RcStr *
newStr(char *str)
{
    RcStr *cs;

    for (cs = strs; cs; cs = cs->next)
        if (!strcmp(str, cs->str)) {
            free(str);
            cs->cnt++;
            return cs;
        }
    if (!(cs = Malloc(sizeof(*cs))))
        return 0;
    cs->cnt = 1;
    cs->str = str;
    cs->next = strs;
    strs = cs;
    return cs;
}

/*
 * decrement ref-count and delete string when count drops to 0.
 */
void
delStr(RcStr *str)
{
    RcStr **cs;

    if (!str || --str->cnt)
        return;
    for (cs = &strs; *cs; cs = &((*cs)->next))
        if (str == *cs) {
            *cs = (*cs)->next;
            free((*cs)->str);
            free(*cs);
            break;
        }
}


typedef struct CfgFile {
    RcStr *name;
    int depidx;
    long deptime;
} CfgFile;

static int numCfgFiles;
static CfgFile *cfgFiles;

static int cfgMapT[] = {
    GC_gGlobal,
    GC_gDisplay,
#ifdef XDMCP
    GC_gXaccess,
#endif
};
static int cfgMap[as(cfgMapT)];

static int
getDeps()
{
    int ncf, i, dep, ret;
    CfgFile *cf;

    openGetter();
    gSendInt(GC_Files);
    ncf = gRecvInt();
    if (!(cf = Malloc(ncf * sizeof(*cf)))) {
        closeGetter();
        return False;
    }
    for (i = 0; i < ncf; i++) {
        cf[i].name = newStr(gRecvStr());
        if ((dep = cf[i].depidx = gRecvInt()) != -1)
            cf[i].deptime = mTime(cf[dep].name->str);
    }
    if (cfgFiles) {
        for (i = 0; i < numCfgFiles; i++)
            delStr(cfgFiles[i].name);
        free(cfgFiles);
    }
    ret = True;
    cfgFiles = cf;
    numCfgFiles = ncf;
    for (i = 0; i < as(cfgMapT); i++) {
        gSendInt(cfgMapT[i]);
        if ((cfgMap[i] = gRecvInt()) < 0) {
            logError("Config reader does not support config cathegory %#x\n",
                     cfgMapT[i]);
            ret = False;
        }
    }
    gSendInt(-1);
    return ret;
}

static int
checkDep(int idx)
{
    int dep;

    if ((dep = cfgFiles[idx].depidx) == -1)
        return False;
    if (checkDep(dep))
        return True;
    return mTime(cfgFiles[dep].name->str) != cfgFiles[idx].deptime;
}

static int
needsReScan(int what, CfgDep *dep)
{
    int widx, idx;
    long mt;

    for (widx = 0; cfgMapT[widx] != what; widx++);
    idx = cfgMap[widx];
    if (checkDep(idx)) {
        if (!getDeps())
            return -1;
        idx = cfgMap[widx];
    }
    mt = mTime(cfgFiles[idx].name->str);
    if (dep->name != cfgFiles[idx].name) {
        if (dep->name)
            delStr(dep->name);
        dep->name = cfgFiles[idx].name;
        dep->name->cnt++;
        dep->time = mt;
        return 1;
    } else if (dep->time != mt) {
        dep->time = mt;
        return 1;
    } else {
        return 0;
    }
}

int
startConfig(int what, CfgDep *dep, int force)
{
    int ret;

    if ((ret = needsReScan(what, dep)) < 0 || (!ret && !force))
        return ret;
    openGetter();
    gSendInt(GC_GetConf);
    gSendInt(what);
    gSendStr(dep->name->str);
    return 1;
}

static void
loadResources(CfgArr *conf)
{
    char **vptr, **pptr, *cptr;
    int *iptr, i, id, nu, j, nptr, nint, nchr;

    free(conf->data);
    conf->numCfgEnt = gRecvInt();
    nptr = gRecvInt();
    nint = gRecvInt();
    nchr = gRecvInt();
    if (!(conf->data = Malloc(conf->numCfgEnt *
                                  (sizeof(int) +
                                   sizeof(char *)) +
                              nptr * sizeof(char *) +
                              nint * sizeof(int) +
                              nchr))) {
        closeGetter();
        return;
    }
    vptr = (char **)conf->data;
    pptr = vptr + conf->numCfgEnt;
    conf->idx = (int *)(pptr + nptr);
    iptr = conf->idx + conf->numCfgEnt;
    cptr = (char *)(iptr + nint);
    for (i = 0; i < conf->numCfgEnt; i++) {
        id = gRecvInt();
        conf->idx[i] = id;
        switch (id & C_TYPE_MASK) {
        case C_TYPE_INT:
            vptr[i] = (char *)((unsigned long)gRecvInt());
            break;
        case C_TYPE_STR:
            vptr[i] = cptr;
            cptr += gRecvStrBuf(cptr);
            break;
        case C_TYPE_ARGV:
            nu = gRecvInt();
            vptr[i] = (char *)pptr;
            for (j = 0; j < nu; j++) {
                *pptr++ = cptr;
                cptr += gRecvStrBuf(cptr);
            }
            *pptr++ = (char *)0;
            break;
        default:
            logError("Config reader supplied unknown data type in id %#x\n", id);
            break;
        }
    }
}

static void
applyResource(int id, void **src, void **dst)
{
    switch (id & C_TYPE_MASK) {
    case C_TYPE_INT:
        *(int *)dst = (int)((unsigned long)*src);
        break;
    case C_TYPE_STR:
    case C_TYPE_ARGV:
        *(char **)dst = *(char **)src;
        break;
    }
}


#define boffset(f) offsetof(struct display, f)

/* no global variables exported currently
struct globEnts {
        int id;
        char **off;
} globEnt[] = {
};
 */

/* no per-display variables exported currently
struct dpyEnts {
        int id;
        int off;
} dpyEnt[] = {
};
 */

CfgArr cfg;

void **
findCfgEnt(struct display *d, int id)
{
    int i;

/* no global variables exported currently
    for (i = 0; i < as(globEnt); i++)
        if (globEnt[i].id == id)
            return globEnt[i].off;
 */
    for (i = 0; i < cfg.numCfgEnt; i++)
        if (cfg.idx[i] == id)
            return ((void **)cfg.data) + i;
    if (d) {
/* no per-display variables exported currently
        for (i = 0; i < as(dpyEnt); i++)
            if (dpyEnt[i].id == id)
                return (char **)(((char *)d) + dpyEnt[i].off);
 */
        for (i = 0; i < d->cfg.numCfgEnt; i++)
            if (d->cfg.idx[i] == id)
                return ((void **)d->cfg.data) + i;
    }
    debug("unknown config entry %#x requested\n", id);
    return 0;
}


CONF_CORE_GLOBAL_DEFS

struct globVals {
    int id;
    void *ptr;
} globVal[] = {
    CONF_CORE_GLOBALS
};

int
loadDMResources(int force)
{
    int i, ret;
    void **ent;

    if (Setjmp(cnftalk.errjmp))
        return -1; /* may memleak, but we probably have to abort anyway */
    if ((ret = startConfig(GC_gGlobal, &cfg.dep, force)) <= 0)
        return ret;
    loadResources(&cfg);
/*    debug("manager resources: %[*x\n",
            cfg.numCfgEnt, ((char **)cfg.data) + cfg.numCfgEnt);*/
    ret = 1;
    for (i = 0; i < as(globVal); i++) {
        if (!(ent = findCfgEnt(0, globVal[i].id)))
            ret = -1;
        else
            applyResource(globVal[i].id, ent, (void **)globVal[i].ptr);
    }
    if (ret < 0)
        logError("Internal error: config reader supplied incomplete data\n");
    return ret;
}


struct dpyVals {
    int id;
    int off;
} dpyVal[] = {
    CONF_CORE_LOCALS
};

int
loadDisplayResources(struct display *d)
{
    int i, ret;
    void **ent;

    if (Setjmp(cnftalk.errjmp))
        return -1; /* may memleak */
    if ((ret = startConfig(GC_gDisplay, &d->cfg.dep, False)) <= 0)
        return ret;
    gSendStr(d->name);
    gSendStr(d->class2);
    loadResources(&d->cfg);
/*    debug("display(%s, %s) resources: %[*x\n", d->name, d->class2,
            d->cfg.numCfgEnt, ((char **)d->cfg.data) + d->cfg.numCfgEnt);*/
    ret = 1;
    for (i = 0; i < as(dpyVal); i++) {
        if (!(ent = findCfgEnt(d, dpyVal[i].id)))
            ret = -1;
        else
            applyResource(dpyVal[i].id, ent,
                          (void **)(((char *)d) + dpyVal[i].off));
    }
    if (ret < 0)
        logError("Internal error: config reader supplied incomplete data\n");
    return ret;
}

int
initResources(char **argv)
{
    originalArgv = argv;
    cnftalk.pipe = &getter.pipe;
    if (Setjmp(cnftalk.errjmp))
        return False; /* may memleak */
    return getDeps();
}

static void
addServers(char **srv, int bType)
{
    char *name, *class2;
    const char *dtx, *cls;
    struct display *d;

    for (; *srv; srv++) {
        if ((cls = strchr(*srv, '_'))) {
            if (!strNDup(&name, *srv, cls - *srv))
                return;
            if (!strDup(&class2, cls + 1)) {
                free(name);
                return;
            }
        } else {
            if (!strDup(&name, *srv))
                return;
            class2 = 0;
        }
        if ((d = findDisplayByName(name))) {
            free(d->class2);
            dtx = "existing";
        } else {
            if (!(d = newDisplay(name))) {
                free(name);
                free(class2);
                return;
            }
            dtx = "new";
        }
        d->stillThere = True;
        d->class2 = class2;
        d->displayType = (*name == ':' ? dLocal : dForeign) | bType;
        if ((bType & d_lifetime) == dReserve) {
            if (d->status == notRunning)
                d->status = reserve;
        } else {
            if (d->status == reserve)
                d->status = notRunning;
        }
        debug("found %s %s%s display: %s %s\n", dtx,
              ((d->displayType & d_location) == dLocal) ? "local" : "foreign",
              ((d->displayType & d_lifetime) == dReserve) ? " reserve" : "",
              d->name, d->class2);
        free(name);
    }
}

void
scanServers(void)
{
    debug("scanServers\n");
    addServers(staticServers, dFromFile | dPermanent);
    addServers(reserveServers, dFromFile | dReserve);
}

