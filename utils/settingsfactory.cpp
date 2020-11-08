#include "settingsfactory.h"

#include <QString>

QMap<QString, QSharedPointer<QSettings> > SettingsFactory::g_ObjectCache;

QSharedPointer<QSettings> SettingsFactory::Create(const QString &widget)
{
    QSharedPointer<QSettings>& settings = g_ObjectCache[widget];
    if (!settings) {
        settings.reset(CreateNew());
        if (!widget.isEmpty()) {
            settings->beginGroup(widget);
        }
    }
    return settings;
}

QSettings *SettingsFactory::CreateNew()
{
    return new QSettings(QSettings::IniFormat, QSettings::UserScope, "MagicMirror", "MagicMirror");
}
