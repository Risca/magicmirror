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

namespace weather {
class CurrentConditions;
}
class Slideshow;

class MirrorFrame : public QFrame {
    Q_OBJECT

public:
    static MirrorFrame* Create();
    virtual ~MirrorFrame();

    void resizeEvent(QResizeEvent *e);

signals:
    void minuteChanged();
    void dayChanged(const QDate&);

public slots:
    void updateClock();
    void cycleWidgets();

private:
    MirrorFrame(QSharedPointer<QNetworkAccessManager> net);
    void createRightPanel();
    void createLeftPanel();

    Ui::MirrorFrame *ui;
    weather::CurrentConditions* m_weatherWidget;
    Slideshow *m_slideshow;
    QTimer m_clockTimer, m_cycleTimer;
    QSharedPointer<QNetworkAccessManager> m_net;
};
