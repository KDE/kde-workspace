#include "keyboardpainter.h"
#include "ui_keyboardpainter.h"
#include<QHBoxLayout>
#include<QVBoxLayout>

KeyboardPainter::KeyboardPainter() :
    kbframe(new KbPreviewFrame(this)),
    exitButton(new QPushButton(tr("CLOSE"),this)),
    ui(new Ui::keyboardpainter)
{
    ui->setupUi(this);
    kbframe->setFixedSize( 1030, 490 );
    exitButton->setFixedSize(120,30);
    connect(exitButton,SIGNAL(clicked()),this,SLOT(close()));
    QVBoxLayout*const vLayout = new QVBoxLayout( this );
    vLayout->addWidget(kbframe);
    vLayout->addWidget(exitButton);
    setWindowTitle(kbframe->kblayout.Layoutname);
}

KeyboardPainter::~KeyboardPainter()
{
    delete ui;
    delete kbframe;
    delete exitButton;
}

QString KeyboardPainter::getVariant(QString variant,QString selectedLayout){
    if (variant==""){
        variant="basic";
        if(selectedLayout=="ma")
            variant="arabic";
        if(selectedLayout=="az")
            variant="latin";
        if(selectedLayout=="bg")
            variant="bds";
        if(selectedLayout=="fi")
            variant="kotoistus";
        if(selectedLayout=="ca")
            variant="fr";
        if(selectedLayout=="in")
            variant="deva";
        if(selectedLayout=="jp")
            variant="106";
        if(selectedLayout=="ir")
            variant="pes";
        if(selectedLayout=="kr")
            variant="kr106";
        if(selectedLayout=="ru")
            variant="winkeys";
        if(selectedLayout=="lk")
            variant="sin_phonetic";
        if(selectedLayout=="ke")
            variant="swa";
        if(selectedLayout=="tz")
            variant="swa";
        if(selectedLayout=="tw")
            variant="tw";
        if(selectedLayout=="bw")
            variant="tswana";
        if(selectedLayout=="ua")
            variant="unicode";
        if(selectedLayout=="pk")
            variant="urd-phonetic";
        if(selectedLayout=="uz")
            variant="cyrillic";
    }
    return variant;
}
