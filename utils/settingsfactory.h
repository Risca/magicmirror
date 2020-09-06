#ifndef SETTINGSFACTORY_H
#define SETTINGSFACTORY_H

#include <QMap>
#include <QSettings>
#include <QSharedPointer>

class QString;

class SettingsFactory
{
public:
    static QSharedPointer<QSettings> Create(const QString& widget = QString());

private:
    static QMap<QString, QSharedPointer<QSettings> > g_ObjectCache;
};

#endif // SETTINGSFACTORY_H
