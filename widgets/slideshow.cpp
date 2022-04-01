#include "slideshow.h"

#include "utils/settingsfactory.h"

#include <QDir>
#include <QPixmap>

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
            int interval = settings->value("interval", SLIDESHOW_DEFAULT_INTERVAL).toInt() * 1000;
            slideshow = new Slideshow(paths, interval, parent);
            return true;
        }
    }
    return false;
}

Slideshow::Slideshow(const QStringList &files, int interval, QWidget *parent) :
    FadingQLabel(1000, 1000, interval, parent),
    m_files(files)
{
    connect(this, &FadingQLabel::fadedOut, this, &Slideshow::changePicture);
    this->fadeOut();
}

void Slideshow::changePicture()
{
    int idx, retries = 10;
    QPixmap pic;

    do {
        idx = rand() % m_files.size();
    } while (retries-- && !pic.load(m_files[idx]));

    if (!pic.isNull()) {
        setPixmap(pic.scaledToWidth(width()));
    }
}
