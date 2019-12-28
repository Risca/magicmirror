/*
 * Copyright (c) 2019 <copyright holder> <email>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "weathericon.h"

WeatherIcon::WeatherIcon()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MagicMirror", "MagicMirror");
    m_path = settings.value("cachedir").toString();

    if (m_path.length() == 0)
        m_path = ICON_CACHE_PATH;

    QDir cache(m_path);

    if (!cache.exists()) {
        if (cache.mkpath(m_path)) {
            qDebug() << __PRETTY_FUNCTION__ << ": Created cache dir" << m_path;
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << ": Failed to create cache dir" << m_path;
        }
    }
}

bool WeatherIcon::exists(QString name)
{
    QString fullPath = m_path + "/" + name + ".png";
    QDir cache(m_path);

    return cache.exists(fullPath);
}

WeatherIcon::~WeatherIcon()
{
}

bool WeatherIcon::get(QString name, QImage *icon)
{
    QString fullPath = m_path + "/" + name;
    QImageReader image(fullPath);

    if (!name.contains("png"))
        fullPath += ".png";

    if (image.canRead()) {
        *icon = image.read();
        return true;
    }
    qDebug() << __PRETTY_FUNCTION__ << ": Unable to read cached icon at" << fullPath;
    return false;
}

bool WeatherIcon::store(QString name, QByteArray data)
{
    QString fullPath = m_path + "/" + name;
    QImageWriter image(fullPath);
    QImage icon;

    if (icon.loadFromData(data)) {
        if (image.canWrite()) {
            image.write(icon);
            qDebug() << __PRETTY_FUNCTION__ << ": Wrote icon" << name;
            return true;
        }
    }
    qDebug() << __PRETTY_FUNCTION__ << ": error storing" << fullPath;
    return false;
}
