#include "fadingqlabel.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

FadingQLabel::FadingQLabel(int msec_in, int msec_out, int period, QWidget *parent) :
    QLabel(parent)
{
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setInterval(period);

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    effect->setOpacity(0.0f);
    this->setGraphicsEffect(effect);

    m_fadeOut = new QPropertyAnimation(effect, "opacity");
    m_fadeOut->setDuration(msec_in);
    m_fadeOut->setStartValue(1.0f);
    m_fadeOut->setEndValue(0.0f);

    m_fadeIn = new QPropertyAnimation(effect, "opacity");
    m_fadeIn->setDuration(msec_out);
    m_fadeIn->setStartValue(0.0f);
    m_fadeIn->setEndValue(1.0f);

    connect(&m_timer, &QTimer::timeout, this, &FadingQLabel::fadeOut);
    connect(m_fadeOut, &QAbstractAnimation::finished, this, &FadingQLabel::fadedOut);
    connect(m_fadeOut, &QAbstractAnimation::finished, this, &FadingQLabel::fadeIn);
    connect(m_fadeIn, &QAbstractAnimation::finished, this, &FadingQLabel::fadedIn);
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
    connect(this, &FadingQLabel::fadedIn,
            &m_timer, static_cast<void(QTimer::*)()>(&QTimer::start));
#else
    connect(this, &FadingQLabel::fadedIn, &m_timer, QOverload<>::of(&QTimer::start));
#endif
}

FadingQLabel::~FadingQLabel()
{
    delete m_fadeIn;
    delete m_fadeOut;
}

void FadingQLabel::fadeOut()
{
    m_timer.stop();
    m_fadeIn->stop();
    m_fadeOut->start();
}

void FadingQLabel::fadeIn()
{
    m_fadeOut->stop();
    m_fadeIn->start();
}
