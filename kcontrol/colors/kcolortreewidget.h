//
// C++ Interface: kcolortreewidget
//
// Description: 
//
//
// Author: Olivier Goffart
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KCOLORTREEWIDGET_H
#define KCOLORTREEWIDGET_H

#include <QTreeWidget>

class KColorTreeWidgetItem;

/**
	@author Olivier Goffart
*/
class KColorTreeWidget : public QTreeWidget{
Q_OBJECT
public:
    KColorTreeWidget(QWidget *parent=0l);

    ~KColorTreeWidget();
    
    void addRole(int idx, const QString &role);
    void setColor(int idx, const QColor &color);
    QColor color(int idx);
    
    bool edit( const QModelIndex & index, EditTrigger trigger, QEvent * event );
    
Q_SIGNALS:
    void colorChanged(int idx, const QColor & color);
    
private:
    QHash< int , KColorTreeWidgetItem* > m_items;
    
};

#endif
