#ifndef KEYBOARDPAINTER_H
#define KEYBOARDPAINTER_H

#include <QDialog>
#include"keyboardlayout.h"
#include"keysym.h"

namespace Ui {
class keyboardpainter;
}

class keyboardpainter : public QDialog
{
    Q_OBJECT
    
public:
    explicit keyboardpainter(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    Keyboardlayout kblayout;
    Keysym symbol;
    QString getvariant(QString variant,QString selectedLayout);
    void getkeyboardlayout(QString County,QString layoutvariant);
    ~keyboardpainter();
    
private:
    Ui::keyboardpainter *ui;
};

#endif // KEYBOARDPAINTER_H
