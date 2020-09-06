#include "calendar.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // set stylesheet
    QFile f(":/qdarkstyle/style.qss");
    if (!f.exists()) {
        qWarning() << "Unable to set stylesheet, file not found";
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    calendar::Calendar w;
    w.show();

    return a.exec();
}
