#ifndef KBPREVIEWFRAME_H
#define KBPREVIEWFRAME_H

#include"keyboardlayout.h"
#include"keysym.h"
#include"keyaliases.h"
#include <QFrame>

class KbPreviewFrame : public QFrame
{
    Q_OBJECT
public:
    explicit KbPreviewFrame(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    KeyboardLayout kblayout;
    KeySym symbol;
    Aliases alias;
    void getKeyboardLayout(QString country, QString layoutvariant);

    
};

#endif // KBPREVIEWFRAME_H 
