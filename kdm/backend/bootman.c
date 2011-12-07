/*

Copyright 2005 Stephan Kulow <coolo@kde.org>
Copyright 2005 Oswald Buddenhagen <ossi@kde.org>

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
 * Boot options
 */

#include "dm.h"
#include "dm_error.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

static int
getNull(char ***opts ATTR_UNUSED, int *def ATTR_UNUSED, int *cur ATTR_UNUSED)
{
    return BO_NOMAN;
}

static int
setNull(const char *opt ATTR_UNUSED, SdRec *sdr ATTR_UNUSED)
{
    return BO_NOMAN;
}

static char *
match(char *obuf, int *blen, const char *key, int klen)
{
    char *buf = obuf;
    if (memcmp(buf, key, klen) || !isspace(buf[klen]))
        return 0;
    buf += klen + 1;
    for (; isspace(*buf); buf++);
    if (!*buf)
        return 0;
    *blen -= buf - obuf;
    return buf;
}

#define GRUB_MENU "/boot/grub/menu.lst"

static char *grubSetDefault;
static char *grub;

static int
getGrub(char ***opts, int *def, int *cur)
{
    FILE *f;
    char *ptr, *linp;
    int len;
    char line[1000];

    if (!grubSetDefault && !grub &&
        !(grubSetDefault = locate("grub-set-default")) &&
        !(grub = locate("grub")))
        return BO_NOMAN;

    *def = 0;
    *cur = -1;
    *opts = initStrArr(0);

    if (!(f = fopen(GRUB_MENU, "r")))
        return errno == ENOENT ? BO_NOMAN : BO_IO;
    while ((len = fGets(line, sizeof(line), f)) != -1) {
        for (linp = line; isspace(*linp); linp++, len--);
        if ((ptr = match(linp, &len, "default", 7))) {
            *def = atoi(ptr);
        } else if ((ptr = match(linp, &len, "title", 5))) {
            for (; isspace(ptr[len - 1]); len--);
            *opts = addStrArr(*opts, ptr, len);
        }
    }
    fclose(f);

    return BO_OK;
}

static int
setGrub(const char *opt, SdRec *sdr)
{
    FILE *f;
    char *ptr;
    int len, i;
    char line[1000];

    if (!(f = fopen(GRUB_MENU, "r")))
        return errno == ENOENT ? BO_NOMAN : BO_IO;
    for (i = 0; (len = fGets(line, sizeof(line), f)) != -1;)
        if ((ptr = match(line, &len, "title", 5))) {
            if (!strcmp(ptr, opt)) {
                fclose(f);
                sdr->osindex = i;
                sdr->bmstamp = mTime(GRUB_MENU);
                return BO_OK;
            }
            i++;
        }
    fclose(f);
    return BO_NOENT;
}

static void
commitGrub(void)
{
    if (sdRec.bmstamp != mTime(GRUB_MENU) &&
            setGrub(sdRec.osname, &sdRec) != BO_OK)
        return;

    if (grubSetDefault) {
        /* The grub-set-default command must be used, which is
         * not so good because there is no way of setting an
         * entry for the next boot only. */
        char index[16];
        const char *args[] = { grubSetDefault, index, 0 };
        sprintf(index, "%d", sdRec.osindex);
        runAndWait((char **)args, environ);
    } else {
        /* The grub shell can be used with `savedefault'.
         * That requires a (widely distributed) patch to grub, e.g.
         * grub-0.97-once.patch. It won't work with a vanilla grub.*/
        FILE *f;
        int pid;
        static const char *args[] = { 0, "--batch", "--no-floppy", 0 };
        args[0] = grub;
        if ((f = pOpen((char **)args, 'w', &pid))) {
            fprintf(f, "savedefault --default=%d --once\n", sdRec.osindex);
            pClose(f, &pid);
        }
    }
}

static char *grubReboot;
static const char *grubConfig;

static int
getGrub2OrBurg(char ***opts, int *def, int *cur, const char *grubRebootExec)
{
    FILE *f;
    char *ptr, *linp;
    int len, ret = BO_NOMAN, i;
    char line[1000];

    if (!grubReboot && !(grubReboot = locate(grubRebootExec)))
        return BO_NOMAN;

    *def = -1;
    *cur = -1;
    *opts = initStrArr(0);

    if (!(f = fopen(grubConfig, "r")))
        return errno == ENOENT ? BO_NOMAN : BO_IO;
    while ((len = fGets(line, sizeof(line), f)) != -1) {
        for (linp = line; isspace(*linp); linp++, len--);
        if ((ptr = match(linp, &len, "set", 3)) && !memcmp(ptr, "default=\"${saved_entry}\"", 24)) {
            ret = BO_OK;
        } else if ((ptr = match(linp, &len, "menuentry", 9))) {
            linp = ptr;
            if (*linp == '\'') {
                for (i = 0, linp++; *linp && *linp != '\''; linp++)
                    ptr[i++] = *linp;
            } else if (*linp == '"') {
                for (i = 0, linp++; *linp && *linp != '"'; linp++) {
                    if (*linp == '\\') {
                        switch (*(++linp)) {
                        case 0:
                            return BO_IO; /* Unexpected end */
                        case '$':
                        case '"':
                        case '\\':
                            break;
                        default:
                            ptr[i++] = '\\';
                            break;
                        }
                    }
                    ptr[i++] = *linp;
                }
            } else {
                for (i = 0; *linp && !isspace(*linp); linp++) {
                    if (*linp == '\\' && !*(++linp))
                        return BO_IO; /* Unexpected end */
                    ptr[i++] = *linp;
                }
            }
            *opts = addStrArr(*opts, ptr, i);
        }
    }
    fclose(f);

    return ret;
}

static int
getGrub2(char ***opts, int *def, int *cur)
{
    grubConfig = "/boot/grub/grub.cfg";
    return getGrub2OrBurg(opts, def, cur, "grub-reboot");
}

static int
setGrub2(const char *opt, SdRec *sdr)
{
    char **opts;
    int def, cur, ret, i;

    if ((ret = getGrub2(&opts, &def, &cur)) != BO_OK)
        return ret;
    for (i = 0; opts[i]; i++) {
        if (!strcmp(opts[i], opt)) {
            sdr->osindex = i;
            sdr->bmstamp = mTime(grubConfig);
            freeStrArr(opts);
            return BO_OK;
        }
    }
    freeStrArr(opts);
    return BO_NOENT;
}

static void
commitGrub2(void)
{
    if (sdRec.bmstamp != mTime(grubConfig) &&
        setGrub2(sdRec.osname, &sdRec) != BO_OK)
        return;

    if (grubReboot) {
        char index[16];
        const char *args[] = { grubReboot, index, 0 };
        sprintf(index, "%d", sdRec.osindex);
        runAndWait((char **)args, environ);
    }
}

static int
getBurg(char ***opts, int *def, int *cur)
{
    grubConfig = "/boot/burg/burg.cfg";
    return getGrub2OrBurg(opts, def, cur, "burg-reboot");
}

static char *lilo;

static int
getLilo(char ***opts, int *def, int *cur)
{
    FILE *f;
    int cdef, pid, len, ret = BO_OK;
    static const char *args[5] = { 0, "-w", "-v", "-q", 0 };
    char buf[256], next[256];

    if (!lilo && !(lilo = locate("lilo")))
        return BO_NOMAN;

    args[0] = lilo;
    if (!(f = pOpen((char **)args, 'r', &pid)))
        return BO_IO;
    *opts = 0;
    next[0] = 0;
    for (;;) {
        if ((len = fGets(buf, sizeof(buf), f)) == -1) {
            ret = BO_NOMAN;
            goto out;
        }
        if (!memcmp(buf, "Images:", 7))
            break;
#define Ldeflin "  Default boot command line:"
        if (!memcmp(buf, Ldeflin, strlen(Ldeflin))) {
            memcpy(next, buf + strlen(Ldeflin) + 2, len - strlen(Ldeflin) - 3);
            next[len - strlen(Ldeflin) - 3] = 0;
        }
    }
    cdef = *def = 0;
    *cur = -1;
    *opts = initStrArr(0);
    while ((len = fGets(buf, sizeof(buf), f)) != -1)
        if (buf[0] == ' ' && buf[1] == ' ' && buf[2] != ' ') {
            if (buf[len - 1] == '*') {
                *def = cdef;
                len--;
            }
            for (; buf[len - 1] == ' '; len--);
            *opts = addStrArr(*opts, buf + 2, len - 2);
            if (!strcmp((*opts)[cdef], next))
                *cur = cdef;
            cdef++;
        }
  out:
    if (pClose(f, &pid)) {
        if (*opts)
            freeStrArr(*opts);
        return BO_IO;
    }
    return ret;
}

static int
setLilo(const char *opt, SdRec *sdr ATTR_UNUSED)
{
    char **opts;
    int def, cur, ret, i;

    if ((ret = getLilo(&opts, &def, &cur)) != BO_OK)
        return ret;
    if (!*opt) {
        opt = 0;
    } else {
        for (i = 0; opts[i]; i++)
            if (!strcmp(opts[i], opt))
                goto oke;
        freeStrArr(opts);
        return BO_NOENT;
    }
  oke:
    freeStrArr(opts);
    return BO_OK;
}

static void
commitLilo(void)
{
    static const char *args[5] = { 0, "-w", "-R", 0, 0 };

    args[0] = lilo;
    args[3] = sdRec.osname;
    runAndWait((char **)args, environ);
}

static const struct {
    int (*get)(char ***, int *, int *);
    int (*set)(const char *, SdRec *);
    void (*commit)(void);
} bootOpts[] = {
    { getNull, setNull, 0 },
    { getGrub, setGrub, commitGrub },
    { getGrub2, setGrub2, commitGrub2 },
    { getBurg, setGrub2, commitGrub2 },
    { getLilo, setLilo, commitLilo },
};

int
getBootOptions(char ***opts, int *def, int *cur)
{
    return bootOpts[bootManager].get(opts, def, cur);
}

int
setBootOption(const char *opt, SdRec *sdr)
{
    int ret;

    free(sdr->osname);
    sdr->osname = 0;
    if (opt) {
        if ((ret = bootOpts[bootManager].set(opt, sdr)) != BO_OK)
            return ret;
        if (!strDup(&sdr->osname, opt))
            return BO_IO; /* BO_NOMEM */
    }
    return BO_OK;
}

void
commitBootOption(void)
{
    if (sdRec.osname) {
        bootOpts[bootManager].commit();
/*
        free(sdRec.osname);
        sdRec.osname = 0;
*/
    }
}

