/*
   10-opensuse-nepomukcontroller-qml.js - Add nepomukcontroller-qml to the systray
   lifted and adapted from fedora by Alin M Elena <alinm.elena@gmail.com>
   from the ktp-presence one...
   Copyright (C) 2010 Kevin Kofler <kevin.kofler@chello.at>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
   With contributions from 
   Dan Vrátil <dvratil@redhat.com>

   Portions lifted from 01-kubuntu-10.04.js:
   Harald Sitter, apachelogger@ubuntu.com 2010-04-02
   Jonathan Riddell, jriddell@ubuntu.com 2010-02-18
   Copyright Canonical Ltd, may be copied under the GNU GPL 2 or later
*/

systrayFound = false;
pids = panelIds;
for (i = 0; i < pids.length; ++i) {
  p = panelById(pids[i]);
  if (!p) continue;
  ids = p.widgetIds;
  for (j = 0; j < ids.length; ++j) {
    w = p.widgetById(ids[j]);
    if (!w || w.type != "systemtray") continue;
    systrayFound = true;
    var shown = w.readConfig("alwaysShown", Array()); 
    shown.push("nepomukcontroller-qml"); 
    w.writeConfig("alwaysShown", shown);
    w.currentConfigGroup = new Array("Applets");
    max = 0;
    for (k = 0; k < w.configGroups.length; ++k)
      if (parseInt(w.configGroups[k]) > max)
        max = parseInt(w.configGroups[k]);
    w.currentConfigGroup = new Array("Applets", max + 1);
    w.writeConfig("plugin", "nepomukcontroller-qml");
    print("nepomukcontroler-qml added to the systray");
    break;
  }
  if (systrayFound) break;
}
if (!systrayFound)
  print("No systray found");
