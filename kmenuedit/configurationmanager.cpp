/*
 *   Copyright (C) 2013 Julien Borderie <frajibe@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "configurationmanager.h"

#include <KGlobal>
#include <KLocale>

const QString ConfigurationManager::GENERAL_CONFIG_GROUP = "General";
const QString ConfigurationManager::SHOW_HIDDEN_ENTRIES_PROPERTY_NAME = "ShowHidden";
const QString ConfigurationManager::SPLITTER_SIZES_PROPERTY_NAME = "SplitterSizes";

ConfigurationManager* ConfigurationManager::m_instance = 0;

ConfigurationManager::ConfigurationManager() :
    m_configGroup(KGlobal::config(), GENERAL_CONFIG_GROUP)
{
}

bool ConfigurationManager::hiddenEntriesVisible() const
{
    return m_configGroup.readEntry(SHOW_HIDDEN_ENTRIES_PROPERTY_NAME, false);
}

void ConfigurationManager::setHiddenEntriesVisible(bool visible)
{
    m_configGroup.writeEntry(SHOW_HIDDEN_ENTRIES_PROPERTY_NAME, visible);
    m_configGroup.sync();
}

QList<int> ConfigurationManager::getSplitterSizes() const
{
    return m_configGroup.readEntry(SPLITTER_SIZES_PROPERTY_NAME, QList<int>());
}

void ConfigurationManager::setSplitterSizes(const QList<int>& sizes)
{
    m_configGroup.writeEntry(SPLITTER_SIZES_PROPERTY_NAME, sizes);
    m_configGroup.sync();
}
