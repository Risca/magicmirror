/*
This file is part of MythClock.
MythClock is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
MythClock is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with MythClock.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include "MirrorFrame.h"
#include "settingsfactory.h"

#include <time.h>

#ifdef __PRINT_FONTS__
#include <QFontDatabase>
#include <QString>
#include <QStringList>
static void printFonts()
{
    QFontDatabase database;

    foreach (const QString &family, database.families()) {
        qDebug() << "Family:" << family;
        foreach (const QString &style, database.styles(family)) {
            qDebug() << " Style:" << style;
        }
    }
}
#else
#define printFonts()
#endif

int main(int argc, char **argv)
{
    QApplication app (argc, argv);
    MirrorFrame *frame = NULL;

    srand(time(NULL));

    QSharedPointer<QSettings> settings = SettingsFactory::Create();
    qDebug() << "Using settings file:" << settings->fileName();

    if (settings->contains("font")) {
        QFont font(settings->value("font").toString());
        QApplication::setFont(font);
    }
    printFonts();
    qDebug() << "Using font:" << QApplication::font().toString();

    QLocale::setDefault(settings->value("locale", "en_US").toString());

    // set stylesheet
    QFile f(":/dark.qss");
    if (!f.exists()) {
        qWarning() << "Unable to set stylesheet, file not found";
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    QCursor cursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(cursor);
    frame = MirrorFrame::Create();
    frame->showFullScreen();

    return app.exec();
}
