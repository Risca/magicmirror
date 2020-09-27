#include "iconcache.h"

#include "settingsfactory.h"

#include <QBitmap>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegion>
#include <QStandardPaths>

namespace {

void ResizeToVisible(QImage& im)
{
    const QRect visibleRect = QRegion(QBitmap::fromImage(im.createMaskFromColor(0x00000000))).boundingRect();
    if (im.rect() != visibleRect) {
        im = im.copy(visibleRect).scaled(im.size(), Qt::KeepAspectRatio);
    }
}

void LoadImage(QImageReader& r, QImage& image)
{
    r.read(&image);
    ResizeToVisible(image);
}

void LoadImage(QImageReader& reader, QPixmap& pixmap)
{
    QImage im;
    LoadImage(reader, im);
    pixmap = QPixmap::fromImage(im);
}

void LoadImage(QImageReader& reader, QIcon& icon)
{
    QPixmap pixmap;
    LoadImage(reader, pixmap);
    icon = QIcon(pixmap);
}

template<class T>
bool LoadImageFromPath(const QString& fullPath, T& icon)
{
    QImageReader reader(fullPath);

    if (!reader.canRead()) {
        qDebug() << __PRETTY_FUNCTION__ << ": Unable to read cached icon at" << fullPath;
        return false;
    }

    LoadImage(reader, icon);
    return true;
}

} // anonymous namespace

IconCache::IconCache(QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    QObject(parent),
    m_net(net),
    m_iconReply(0)
{
    m_path = SettingsFactory::Create()->value("cachedir", QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString();

    QDir cache(m_path);
    if (!cache.exists() && !cache.mkpath(m_path)) {
        qDebug() << __PRETTY_FUNCTION__ << ": Failed to create cache dir" << m_path;
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << ": Using cache dir" << m_path;
    }
}

IconCache::~IconCache()
{
    if (m_iconReply) {
        m_iconReply->deleteLater();
    }
}

bool IconCache::exists(const QString &name) const
{
    return QFileInfo::exists(GetFullPath(name));
}

bool IconCache::get(const QString &name, QIcon &icon) const
{
    const QString fullPath = GetFullPath(name);
    return LoadImageFromPath(fullPath, icon);
}

bool IconCache::get(const QString &name, QImage &icon) const
{
    const QString fullPath = GetFullPath(name);
    return LoadImageFromPath(fullPath, icon);
}

bool IconCache::get(const QString &name, QPixmap &icon) const
{
    const QString fullPath = GetFullPath(name);
    return LoadImageFromPath(fullPath, icon);
}

void IconCache::download(const QString name, const QUrl &url)
{
    if (exists(name)) {
        emit iconDownloaded(name);
    }
    else {
        m_iconsToFetch.push_back(name);
        m_urls[name] = url;
        if (!m_iconReply) {
            // No active request, get icon immediately
            m_iconReply = FetchNextIcon();
        }
    }
}

QString IconCache::GetFullPath(const QString &name) const
{
    QString fullPath = m_path + "/" + name;

    if (!name.endsWith("png"))
        fullPath += ".png";

    return fullPath;
}

QNetworkReply *IconCache::FetchNextIcon()
{
    QString icon;
    do {
        if (m_iconsToFetch.isEmpty()) {
            return 0;
        }
        icon = m_iconsToFetch.takeFirst();
    } while(exists(icon));

    QNetworkReply* reply = m_net->get(QNetworkRequest(m_urls[icon]));
    connect(reply, SIGNAL(finished()), this, SLOT(iconReplyFinished()));
    return reply;
}

void IconCache::iconReplyFinished()
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

bool IconCache::Store(const QString &name, const QByteArray &data)
{
    const QString fullPath = GetFullPath(name);
    QImageWriter image(fullPath);
    QImage icon;

    if (icon.loadFromData(data)) {
        ResizeToVisible(icon);
        if (image.canWrite()) {
            image.write(icon);
            qDebug() << __PRETTY_FUNCTION__ << ": Wrote icon" << name;
            return true;
        }
    }
    qDebug() << __PRETTY_FUNCTION__ << ": error storing" << fullPath;
    return false;
}
