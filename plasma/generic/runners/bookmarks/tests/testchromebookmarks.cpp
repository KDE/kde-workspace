/*
 *   Copyright 2007 Glenn Ergeerts <glenn.ergeerts@telenet.be>
 *   Copyright 2012 Glenn Ergeerts <marco.gulino@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "testchromebookmarks.h"
#include <QTest>
#include <QDir>
#include "browsers/chrome.h"
#include "browsers/chromebookmarksfinder.h"

void TestChromeBookmarks::testBookmarksFinder()
{
  ChromeBookmarksFinder findChrome("chrome");
  QString actualTemplate = QString("%1/.config/%2/Default/Bookmarks").arg(QDir::homePath());
  QCOMPARE(findChrome.find(), QStringList(actualTemplate.arg("chrome")));
  ChromeBookmarksFinder findChromium("chromium");
  QCOMPARE(findChromium.find(), QStringList(actualTemplate.arg("chromium")));
}


void TestChromeBookmarks::itShouldFindNothingWhenPrepareIsNotCalled()
{
  FakeBookmarksFinder finder(QStringList("Bookmarks"));
  Chrome *chrome = new Chrome(&finder, this);
  QCOMPARE(chrome->match("any", true).size(), 0);
}

#include "testchromebookmarks.moc"

QTEST_MAIN(TestChromeBookmarks);
