#ifndef __MODERNSYS_H
#define __MODERNSYS_H

#include <QBitmap>
#include <kcommondecoration.h>
#include <kdecorationfactory.h>

namespace ModernSystem {

class ModernSys;

class ModernButton : public KCommonDecorationButton
{
public:
    ModernButton(ButtonType type, ModernSys *parent, const char *name);
    void setBitmap(const unsigned char *bitmap);
    virtual void reset(unsigned long changed);
protected:
    void paintEvent(QPaintEvent *);
    virtual void drawButton(QPainter *p);
    void drawButtonLabel(QPainter *){;}
    QBitmap deco;
};

class ModernSys : public KCommonDecoration
{
public:
    ModernSys( KDecorationBridge* b, KDecorationFactory* f );
    ~ModernSys(){;}

    virtual QString visibleName() const;
    virtual QString defaultButtonsLeft();
    virtual QString defaultButtonsRight();
    virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;
    virtual int layoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;
    virtual KCommonDecorationButton *createButton(ButtonType type);

    virtual void updateWindowShape();
    virtual void updateCaption();

    void init();
protected:
    void drawRoundFrame(QPainter &p, int x, int y, int w, int h);
    void paintEvent( QPaintEvent* );
    void recalcTitleBuffer();
    void reset( unsigned long );
private:
    QPixmap titleBuffer;
    QString oldTitle;
    bool reverse;
};

class ModernSysFactory : public QObject, public KDecorationFactory
{
    Q_OBJECT

public:
    ModernSysFactory();
    virtual ~ModernSysFactory();
    virtual KDecoration* createDecoration( KDecorationBridge* );
    virtual bool reset( unsigned long changed );
    virtual bool supports( Ability ability ) const;
    QList< BorderSize > borderSizes() const;
private:
    void read_config();
};

}

#endif // __MODERNSYS_H
