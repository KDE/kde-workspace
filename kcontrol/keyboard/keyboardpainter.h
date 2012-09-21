#ifndef KEYBOARDPAINTER_H
#define KEYBOARDPAINTER_H

#include <QDialog>
#include<QPushButton>

#include"kbpreviewframe.h"

namespace Ui {
class keyboardpainter;
}

class KeyboardPainter : public QDialog
{
    Q_OBJECT
    
public:
    explicit KeyboardPainter();
    KbPreviewFrame *kbframe;
    QPushButton *exitButton;
    QString getVariant(QString variant,QString selectedLayout);
    ~KeyboardPainter();
    
private:
    Ui::keyboardpainter *ui;
};

#endif // KEYBOARDPAINTER_H
