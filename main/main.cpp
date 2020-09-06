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

#include "MirrorFrame.h"
#include "settingsfactory.h"

MirrorFrame *frame = NULL;

namespace {

#ifdef __PRINT_FONTS__
void printFonts()
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

void printSettingsFile()
{
    qDebug() << __PRETTY_FUNCTION__ << "Using" << SettingsFactory::Create()->fileName();
}

} // anonymous

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    printSettingsFile();
    printFonts();

    QLocale::setDefault(SettingsFactory::Create()->value("locale", "en_US").toString());

    QCursor cursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(cursor);
    frame = MirrorFrame::Create();
    frame->showFullScreen();

    return app.exec();
}
