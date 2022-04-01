#include "compliment.h"

#include "utils/settingsfactory.h"

#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTextStream>

#define COMPLIMENT_UPDATE_INTERVAL (60 * 1000)

namespace {

void LoadComplimentsFromFile(const QString& path, QStringList& compliments)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(5,5,0)
    while (!in.atEnd()) {
        QString line = in.readLine();
        compliments.push_back(line);
    }
#else
    QString line;
    while (in.readLineInto(&line)) {
        compliments.push_back(line);
    }
#endif
}

} // anonymous namespace

Compliment::Compliment(QWidget *parent) :
    FadingQLabel(1000, 1000, COMPLIMENT_UPDATE_INTERVAL, parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Compliments");
    if (settings->contains("anytime")) {
        QVariant compliments = settings->value("anytime");
        if (compliments.type() == QVariant::StringList) {
            m_compliments = compliments.toStringList();
        }
        // assume it's a file path
        else {
            LoadComplimentsFromFile(compliments.toString(), m_compliments);
        }
    }
    if (m_compliments.empty()) {
        m_compliments << "You're beautiful";
        m_compliments << "Party on!";
    }

    qDebug() << __PRETTY_FUNCTION__ << "Loaded" << m_compliments.size() << "compliments";

    connect(this, &FadingQLabel::fadedOut, this, &Compliment::changeCompliment);
}

void Compliment::setText(const QString &text)
{
    m_newText = text;
    fadeOut();
}

void Compliment::changeCompliment()
{
    if (m_newText.isEmpty()) {
        std::random_shuffle(m_compliments.begin(), m_compliments.end());

        if (m_compliments.first() == this->text()) {
            QLabel::setText(m_compliments.last());
        }
        else {
            QLabel::setText(m_compliments.first());
        }
    }
    else {
        QLabel::setText(m_newText);
        m_newText.clear();
    }
}
