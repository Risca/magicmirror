#pragma once

#include "fadingqlabel.h"

#include <QStringList>

class Slideshow : public FadingQLabel
{
    Q_OBJECT
public:
    static bool Create(Slideshow*& slideshow, QWidget *parent = 0);

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

    virtual void resizeEvent(QResizeEvent* e);

public slots:
    void changePicture();

private:
    Slideshow(const QStringList &files, int interval, QWidget *parent = 0);

    int heightForWidth(int width) const;

    const QStringList m_files;
    QString m_lastFile;
};
