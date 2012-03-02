/***************************************************************************
 *   Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef QMLRUNNERVIEW_H
#define QMLRUNNERVIEW_H

#include <QtGui/QWidget>
#include <QGraphicsView>

#include <QtGui/QLabel>
#include <QtDeclarative/QDeclarativeView>
#include <krunnerdialog.h>

class QLineEdit;
class RunnerSettings;
class Dialog;
class qmlrunnerView : public KRunnerDialog
{
    Q_OBJECT
public:
    qmlrunnerView(Plasma::RunnerManager* manager, QWidget* parent);
    virtual ~qmlrunnerView();
    virtual void clearHistory();
    virtual void display(const QString& term = QString());
    virtual void setConfigWidget(QWidget* w);

public slots:
    void configure();
    void closeSettings();
    void changeGeometry();

signals:
    void changeTerm(const QString& term);

private:
    RunnerSettings* settings();
    
    RunnerSettings* m_settings;
    QDeclarativeView* view;
};

#endif // QMLRUNNERVIEW_H
