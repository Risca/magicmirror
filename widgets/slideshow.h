#pragma once

#include "fadingqlabel.h"

#include <QStringList>

class Slideshow : public FadingQLabel
{
    Q_OBJECT
public:
    static bool Create(Slideshow*& slideshow, QWidget *parent = 0);

public slots:
    void changePicture();

private:
    Slideshow(const QStringList &files, int interval, QWidget *parent = 0);

    const QStringList m_files;
};
