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

#ifndef configurationmanager_h
#define configurationmanager_h

#include <QObject>
#include <QMutex>

#include <KConfigGroup>

/**
 * @brief The ConfigurationManager class contains allows to access/edit the application configuration.
 */
class ConfigurationManager : public QObject
{
    Q_OBJECT

protected:
    static const QString GENERAL_CONFIG_GROUP;
    static const QString SHOW_HIDDEN_ENTRIES_PROPERTY_NAME;
    static const QString SPLITTER_SIZES_PROPERTY_NAME;

    static ConfigurationManager *m_instance;

    KConfigGroup m_configGroup;

public:
    /**
     * @brief Return the singleton instance.
     *
     * @return Instance.
     */
    static ConfigurationManager *getInstance()
    {
        static QMutex mutex;
        if (!m_instance) {
            mutex.lock();
            if (!m_instance) {
                m_instance = new ConfigurationManager();
            }
            mutex.unlock();
        }
        return m_instance;;
    }

    /**
     * @brief Determine if hidden entries must be visible or not.
     *
     * @return True to show hidden entries, false else.
     */
    bool hiddenEntriesVisible() const;

    /**
     * @brief Replace the hidden entries visibility by the new one.
     *
     * @param visible True to show hidden entries, false else.
     */
    void setHiddenEntriesVisible(bool visible);

    /**
     * @brief Get the splitter sizes.
     *
     * @return Splitter sizes.
     */
    QList<int> getSplitterSizes() const;

    /**
     * @brief Replace splitter sizes by the new ones.
     *
     * @param sizes Splitter sizes.
     */
    void setSplitterSizes(const QList<int>& sizes);


protected:
    ConfigurationManager();
    ConfigurationManager(const ConfigurationManager &); // hide copy constructor
    ConfigurationManager& operator=(const ConfigurationManager &); // hide assign op
};

#endif
