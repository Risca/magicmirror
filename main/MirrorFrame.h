/*
    This file is part of MagicMirror.
    MagicMirror is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    MagicMirror is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with MagicMirror.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QFrame>
#include <QSharedPointer>
#include <QTimer>

class QDate;
class QNetworkAccessManager;

namespace Ui {
class MirrorFrame;
}

class MirrorFrame : public QFrame {
    Q_OBJECT

public:
    static MirrorFrame* Create();
    virtual ~MirrorFrame();

signals:
    void dayChanged(const QDate&);

public slots:
    void updateClock();

private:
    MirrorFrame(QSharedPointer<QNetworkAccessManager> net);
    void createClimateSystem();
    void createCalendarSystem();

    Ui::MirrorFrame *ui;
    QTimer m_clockTimer;
    QSharedPointer<QNetworkAccessManager> m_net;
};
