/*
 * Class to generate support information for plasma shells
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "supportinformation.h"

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PackageMetadata>

//this is deliberately _not_ in i18n, the information is for uploading to a bug report so should always be in
//English so as to be useful for developers

QString SupportInformation::generateSupportInformation(Plasma::Corona *corona)
{
    QString infoString;
    QDebug stream(&infoString);
    SupportInformation info(stream);

    info.addHeader();;
    info.addInformationForCorona(corona);

    return infoString;
}

SupportInformation::SupportInformation(const QDebug &outputStream) :
    m_stream(outputStream)
{
}

void SupportInformation::addHeader()
{
    m_stream << "Plasma-desktop Support Information:" << endl
             << "The following information should be used when requesting support on e.g. http://forum.kde.org" << endl
             << "It provides information about the currently running instance and which applets are used." << endl
             << "Please post the information provided underneath this introductory text to a paste bin service "
             << "like http://paste.kde.org instead of pasting into support threads." << endl << endl;

    m_stream << "Version" << endl;
    m_stream << "=======" << endl;
    m_stream << "Plasma version: " << endl;
    m_stream << PLASMA_VERSION_STRING << endl;
    m_stream << "KDE SC version (runtime): " << endl;
    m_stream << KDE::versionString() << endl;
    m_stream << "KDE SC version (compile): " << endl;
    m_stream << KDE_VERSION_STRING << endl;
    m_stream << "Qt Version: " << endl;
    m_stream << qVersion() << endl;

    addSeperator();
}

void SupportInformation::addInformationForCorona(Plasma::Corona *corona)
{
    foreach (Plasma::Containment *containment, corona->containments()) {
        addInformationForContainment(containment);
    }
}

void SupportInformation::addInformationForContainment(Plasma::Containment *containment)
{
    //a containment is also an applet so print standard applet information out
    addInformationForApplet(containment);

    foreach (Plasma::Applet *applet, containment->applets()) {
        addInformationForApplet(applet);
    }
}

void SupportInformation::addInformationForApplet(Plasma::Applet *applet)
{
    if (applet->isContainment()) {
        m_stream << "Containment - ";
    } else {
        m_stream << "Applet - ";
    }
    m_stream << applet->name() << ':' << endl;

    m_stream << "Plugin Name: " << applet->pluginName() << endl;
    m_stream << "Category: " << applet->category() << endl;


    if (applet->package()) {
        m_stream << "API: " << applet->package()->metadata().implementationApi() << endl;
        m_stream << "Type: " << applet->package()->metadata().type() << endl;
        m_stream << "Version: " << applet->package()->metadata().version() << endl;
        m_stream << "Author: " << applet->package()->metadata().author() << endl;
        m_stream << "Hash: " << applet->package()->contentsHash() << endl;
    }

    //runtime info
    m_stream << "Failed To Launch: " << applet->hasFailedToLaunch() << endl;
    m_stream << "ScreenRect: " << applet->screenRect() << endl;
    m_stream << "FormFactor: " << applet->formFactor() << endl;

    m_stream << "Config Group Name: " << applet->config().name() << endl;

    m_stream << endl; //insert a blank line
}

void SupportInformation::addSeperator()
{
    m_stream << endl << "=========" << endl;
}

