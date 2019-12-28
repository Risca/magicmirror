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

#ifdef __USE_RPI__
  #include <wiringPi.h>
#endif

#include "MirrorFrame.h"

MirrorFrame *frame = NULL;

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

void touchEvent(void) {
    qDebug() << "Touch event registered";
    if (frame)
        frame->registerTouchEvent();
}

void setupTouchEvents()
{
#ifdef __USE_RPI__
	qDebug() << __PRETTY_FUNCTION__;
    wiringPiSetupGpio();
    pinMode(12, INPUT);
    wiringPiISR(12, INT_EDGE_FALLING, &touchEvent);
#endif
}

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

#ifdef __PRINT_FONTS__
    printFonts();
#endif

    setupTouchEvents();
    QCursor cursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(cursor);
    frame = new MirrorFrame();
    frame->setGeometry(0, 0, 1680, 1050);
    frame->getEvents();
    frame->getCurrentWeather();
    frame->getForecast();
    frame->show();

    return app.exec();
}

