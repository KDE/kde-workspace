/*
 * Copyright 2013 Jörg Ehrichs <joerg.ehrichs@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

function updateTooltip() {
    var data = new Object
    data["image"] = "nepomuk"
    data["mainText"] = i18nc("tooltip", "Nepomuk Indexer");

    var text="";
    if( nepomukSource.data["FileWatch"]["isActive"] ) {
        text += "<b>" + nepomukSource.data["FileWatch"]["i18nName"] + "</b><br/>";
        text += nepomukSource.data["FileWatch"]["statusMessage"] + "<br/>";
    }
    if( nepomukSource.data["FileIndexer"]["isActive"] ) {
        text += "<b>" + nepomukSource.data["FileIndexer"]["i18nName"] + "</b><br/>";
        text += nepomukSource.data["FileIndexer"]["statusMessage"] + "<br/>";
    }
    if( nepomukSource.data["PIM"]["isActive"] ) {
        text += "<b>" + nepomukSource.data["PIM"]["i18nName"] + "</b><br/>";
        text += nepomukSource.data["PIM"]["statusMessage"] + "<br/>";
    }
    if( nepomukSource.data["WebMiner"]["isActive"] ) {
        text += "<b>" + nepomukSource.data["WebMiner"]["i18nName"] + "</b><br/>";
        text += nepomukSource.data["WebMiner"]["statusMessage"] + "<br/>";
    }

    if(text == "") {
        text = i18nc("tooltip", "No indexer active");
    }

    data["subText"] = text
    plasmoid.popupIconToolTip = data
}

function configChanged()
{
    fileWatcher.visible = plasmoid.readConfig("showFileWatch");
    fileIndexer.visible = plasmoid.readConfig("showFileIndexer");
    akonadiFeeder.visible = plasmoid.readConfig("showPIMIndexer");
    webMiner.visible = plasmoid.readConfig("showWebMiner");
}