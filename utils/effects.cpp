#include "effects.h"

#include <QEvent>
#include <QGraphicsEffect>
#include <QLinearGradient>
#include <QHash>
#include <QObject>
#include <QWidget>

namespace utils {

namespace {

void SetGradient(QWidget* w)
{
    QLinearGradient alphaGradient(w->rect().topLeft(), w->rect().bottomLeft());
    alphaGradient.setColorAt(0.5, Qt::black);
    alphaGradient.setColorAt(1.0, Qt::transparent);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    effect->setOpacity(1);
    effect->setOpacityMask(alphaGradient);
    w->setGraphicsEffect(effect);
}

class FadeEventFilter : public QObject
{
public:
    explicit FadeEventFilter(QWidget* widget) :
        w(widget)
    {
        SetGradient(w);
        w->installEventFilter(this);
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        if (object == w && event->type() == QEvent::Resize) {
            SetGradient(w);
        }
        return false;
    }

private:
    QWidget* w;
};

} // anonymous namespace

void ApplyFade(QWidget *w)
{
    static QHash<QWidget*, FadeEventFilter*> installedFilters;

    FadeEventFilter* filter = installedFilters[w];
    if (!filter) {
        installedFilters[w] = new FadeEventFilter(w);
    }
}

} // namespace utils
