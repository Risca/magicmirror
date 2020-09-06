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

#ifndef WEATHERICON_H
#define WEATHERICON_H

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QVector>

class QByteArray;
class QImage;
class QNetworkAccessManager;
class QNetworkReply;

class WeatherIcon : public QObject
{
    Q_OBJECT

public:
    WeatherIcon(QSharedPointer<QNetworkAccessManager> net, QObject *parent = 0);
    ~WeatherIcon();

    bool exists(const QString &name) const;
    void download(const QString name);
    bool get(const QString &name, QImage &icon) const;

signals:
    void iconDownloaded(const QString& icon);

private:
    QString m_path;
    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_iconReply;
    QVector<QString> m_iconsToFetch;

    bool Store(const QString &name, const QByteArray &data);
    QNetworkReply* FetchNextIcon();
    QString GetFullPath(const QString &name) const;

private slots:
    void iconReplyFinished();
};

#endif // WEATHERICON_H
