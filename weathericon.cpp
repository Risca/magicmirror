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

#include "settingsfactory.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QUrl>

namespace {

QUrl GetUrlForIcon(const QString &icon)
{
    return QUrl("http://openweathermap.org/img/w/" + icon + ".png");
}

} // anonymous namespace

WeatherIcon::WeatherIcon(QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    QObject(parent),
    m_net(net),
    m_iconReply(0)
{
    m_path = SettingsFactory::Create("Weather")->value("cachedir", QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString();

    QDir cache(m_path);
    if (!cache.exists() && !cache.mkpath(m_path)) {
        qDebug() << __PRETTY_FUNCTION__ << ": Failed to create cache dir" << m_path;
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ": Using cache dir" << m_path;
    }
}

WeatherIcon::~WeatherIcon()
{
}

bool WeatherIcon::exists(const QString &name) const
{
    QString fullPath = m_path + "/" + name + ".png";
    QDir cache(m_path);

    return cache.exists(fullPath);
}

void WeatherIcon::download(const QString name)
{
    m_iconsToFetch.push_back(name);
    if (!m_iconReply) {
        // No active request, get icon immediately
        m_iconReply = FetchNextIcon();
    }
}

bool WeatherIcon::get(const QString &name, QImage &icon) const
{
    const QString fullPath = GetFullPath(name);
    QImageReader image(fullPath);

    if (image.canRead()) {
        icon = image.read();
        return true;
    }
    qDebug() << __PRETTY_FUNCTION__ << ": Unable to read cached icon at" << fullPath;
    return false;
}

bool WeatherIcon::Store(const QString &name, const QByteArray &data)
{
    const QString fullPath = GetFullPath(name);
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

void WeatherIcon::iconReplyFinished()
{
    if (m_iconReply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_iconReply->errorString();
    }
    else {
        QNetworkRequest r = m_iconReply->request();
        QString icon = QFileInfo(r.url().fileName()).baseName();
        if (!exists(icon) && icon.length() > 0) {
            Store(icon, m_iconReply->readAll());
        }
        emit iconDownloaded(icon);
    }
    m_iconReply->deleteLater();
    m_iconReply = FetchNextIcon();
}

QNetworkReply *WeatherIcon::FetchNextIcon()
{
    QString icon;
    do {
        if (m_iconsToFetch.isEmpty()) {
            return 0;
        }
        icon = m_iconsToFetch.takeFirst();
    } while(exists(icon));
    QNetworkReply* reply = m_net->get(QNetworkRequest(GetUrlForIcon(icon)));
    connect(reply, SIGNAL(finished()), this, SLOT(iconReplyFinished()));
    return reply;
}

QString WeatherIcon::GetFullPath(const QString &name) const
{
    QString fullPath = m_path + "/" + name;

    if (!name.endsWith("png"))
        fullPath += ".png";

    return fullPath;
}
