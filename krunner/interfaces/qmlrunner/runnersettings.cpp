#include "runnersettings.h"
#include <settings.h>
#include <kconfig.h>
#include <KConfigGroup>

RunnerSettings::RunnerSettings(QObject* parent)
    : QObject(parent), m_settings(new Settings)
{}

RunnerSettings::~RunnerSettings()
{
    delete m_settings;
}

void RunnerSettings::setUsedRunners(const QStringList& r)
{
    m_settings->setUsedRunners(r);
}

QStringList RunnerSettings::usedRunners() const
{
    return m_settings->usedRunners();
}
