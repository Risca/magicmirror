#pragma once

#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QUrl>
#include <QVector>

class QByteArray;
class QIcon;
class QImage;
class QNetworkAccessManager;
class QNetworkReply;

class IconCache : public QObject
{
    Q_OBJECT

public:
    explicit IconCache(QSharedPointer<QNetworkAccessManager> net, QObject *parent = 0);
    ~IconCache();

    bool exists(const QString &name) const;
    bool get(const QString &name, QIcon &icon) const;
    bool get(const QString &name, QImage &icon) const;
    bool get(const QString &name, QPixmap &icon) const;
    void download(const QString name, const QUrl& url);

signals:
    void iconDownloaded(const QString& icon);

private:
    QString m_path;
    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_iconReply;
    QVector<QString> m_iconsToFetch;
    QMap<QString, QUrl> m_urls;

    QString GetFullPath(const QString &name) const;
    QNetworkReply* FetchNextIcon();
    bool Store(const QString &name, const QByteArray &data);

private slots:
    void iconReplyFinished();
};
