/*****************************************************************
kwin - the KDE window manager

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/
#ifndef KWIN_KILLWINDOW_H
#define KWIN_KILLWINDOW_H

#include "workspace.h"

namespace KWinInternal {

class KillWindow 
{
public:
    
    KillWindow( Workspace* ws );
    ~KillWindow();
    
    void start();
    
private:
    Workspace* workspace;
};

};

#endif
