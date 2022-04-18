#include "slideshow.h"

#include "utils/settingsfactory.h"

#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QPixmap>
#include <QResizeEvent>
#include <QSizePolicy>

#define SLIDESHOW_DEFAULT_INTERVAL (10)

bool Slideshow::Create(Slideshow *&slideshow, QWidget *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Slideshow");
    if (settings->contains("directory")) {
        const QDir dir(settings->value("directory").toString());
        QStringList filters, files;
        filters << "*.jpg" << "*.jpeg" << "*.png";
        files = dir.entryList(filters, QDir::Files | QDir::Readable);
        if (!files.empty()) {
            QStringList paths;
            foreach (const QString& file, files) {
                paths << dir.absoluteFilePath(file);
            }
            int interval = settings->value("interval", SLIDESHOW_DEFAULT_INTERVAL).toInt();
            qDebug() << __PRETTY_FUNCTION__ << "Found" << paths.size() << "images to show, switching every" << interval << "second";
            slideshow = new Slideshow(paths, interval * 1000, parent);
            return true;
        }
    }
    return false;
}

QSize Slideshow::minimumSizeHint() const
{
    return QSize(20, 20);
}

QSize Slideshow::sizeHint() const
{
    int w = this->width();
    return QSize(w, heightForWidth(w));
}

void Slideshow::resizeEvent(QResizeEvent *e)
{
    if (!m_lastFile.isEmpty()) {
        QImageReader img;
        img.setAutoTransform(true);
        img.setFileName(m_lastFile);
        if (img.canRead()) {
            QSize const imgSize = img.size();
            QSize scaledSize;

            if (img.transformation() & QImageIOHandler::TransformationRotate90) {
                QSize const realImgSize = imgSize.transposed();
                scaledSize = realImgSize.scaled(QSize(e->size().width(), realImgSize.height()), Qt::KeepAspectRatio).transposed();
            }
            else {
                scaledSize = imgSize.scaled(QSize(e->size().width(), imgSize.height()), Qt::KeepAspectRatio);
            }

            img.setScaledSize(scaledSize);
            setPixmap(QPixmap::fromImageReader(&img));
        }
    }
}

Slideshow::Slideshow(const QStringList &files, int interval, QWidget *parent) :
    FadingQLabel(1000, 1000, interval, parent),
    m_files(files)
{
    connect(this, &FadingQLabel::fadedOut, this, &Slideshow::changePicture);
    this->fadeOut();
}

int Slideshow::heightForWidth(int width) const
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    const QPixmap *pix = this->pixmap();
    return !pix ? this->height() : ((qreal)pix->height()*width)/pix->width();
#else
    const QPixmap& pix = this->pixmap(Qt::ReturnByValue);
    return pix.isNull() ? this->height() : ((qreal)pix.height()*width)/pix.width();
#endif
}

void Slideshow::changePicture()
{
    const int numFiles = m_files.size();
    int retries = 10;
    QImageReader img;

    img.setAutoTransform(true);

    do {
        const QString& filename = m_files[rand() % numFiles];
        if (filename == m_lastFile)
            continue;
        m_lastFile = filename;
        img.setFileName(filename);
    } while (retries-- && !img.canRead());

    if (img.canRead()) {
        // Don't actually read the image file here, just resize the widget and read the file in resizeEvent()
        QSize const imgSize = img.transformation() & QImageIOHandler::TransformationRotate90 ?
                    img.size().transposed() : img.size();
        QSize const scaledSize = imgSize.scaled(QSize(width(), imgSize.height()), Qt::KeepAspectRatio);

        if (scaledSize == this->size()) {
            // If the size doesn't change, no event will get posted, unless we do it.
            QResizeEvent *e = new QResizeEvent(scaledSize, this->size());
            QCoreApplication::postEvent(this, e);
        }
        else {
            this->resize(scaledSize);
        }
    }
}
