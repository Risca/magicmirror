/*
    This file is part of MagicMirror
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

#ifndef __CALENDARDATA_H__
#define __CALENDARDATA_H__

#include <QtCore/QtCore>

class CalendarData : public QObject {
    Q_OBJECT
public:
    CalendarData(QObject *parent = 0);
    virtual ~CalendarData();

public slots:
    void process();

signals:
    void newEvent(QString);
    void finished();
    void error(QString);
    void temperature(double);
    void humidity(double);
    void windSpeed(double);
    void skyConditions(QString);

private:
    void processResults(QByteArray&);
};

#endif /* __CALENDARDATA_H__ */
