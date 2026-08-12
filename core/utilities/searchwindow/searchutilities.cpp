/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2007      by Aaron Seigo <aseigo at kde dot org>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchutilities.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QLinearGradient>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOption>
#include <QTimeLine>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummodel.h"
#include "ratingwidget.h"
#include "thememanager.h"
#include "itemvisibilitycontroller.h"

namespace Digikam
{

class Q_DECL_HIDDEN AnimatedClearButton::Private : public AnimatedVisibility
{
    Q_OBJECT

public:

    explicit Private(QObject* const parent)
        : AnimatedVisibility(parent)
    {
    }

public:

    bool    stayAlwaysVisible   = false;
    bool    pixmapEnabled       = true;
    QPixmap pixmap;
};

AnimatedClearButton::AnimatedClearButton(QWidget* const parent)
    : QWidget(parent),
      d      (new Private(this))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(d, SIGNAL(opacityChanged()),
            this, SLOT(update()));

    connect(d, SIGNAL(visibleChanged()),
            this, SLOT(slotVisibleChanged()));
}

QSize AnimatedClearButton::sizeHint() const
{
    QFontMetrics fm(font());

    return QSize(d->pixmap.width(), fm.lineSpacing());
}

void AnimatedClearButton::stayVisibleWhenAnimatedOut(bool stayVisible)
{
    d->stayAlwaysVisible = stayVisible;
    slotVisibleChanged();
}

void AnimatedClearButton::setShallBeShown(bool shown)
{
    d->controller()->setShallBeShownDirectly(shown);
    slotVisibleChanged();
}

void AnimatedClearButton::animateVisible(bool visible)
{
    // skip animation if parent widget is not visible

    if (!parentWidget() || !parentWidget()->isVisible())
    {
        d->controller()->setDirectlyVisible(visible);
    }

    d->controller()->setAnimationDuration(visible ? 150 : 250);
    d->controller()->setVisible(visible);
}

void AnimatedClearButton::setDirectlyVisible(bool visible)
{
    d->controller()->setDirectlyVisible(visible);
}

void AnimatedClearButton::setPixmap(const QPixmap& p)
{
    d->pixmap = p;
}

QPixmap AnimatedClearButton::pixmap() const
{
    return d->pixmap;
}

void AnimatedClearButton::slotPixmapEnabled(bool b)
{
    d->pixmapEnabled = b;
}

void AnimatedClearButton::slotUpdateAnimationSettings()
{
}

void AnimatedClearButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter p(this);

    if (d->pixmapEnabled)
    {
        p.setOpacity(1); // make sure
        qreal dpr = devicePixelRatio();
        p.drawPixmap((width()  * dpr - d->pixmap.width())  / 2,
                     (height() * dpr - d->pixmap.height()) / 2,
                     d->pixmap);
    }
    else
    {
        p.fillRect(0, 0,
                   d->pixmap.width(),
                   d->pixmap.height(),
                   QBrush(QColor(Qt::transparent)));
    }
}

void AnimatedClearButton::slotVisibleChanged()
{
    Q_EMIT visibleChanged(d->isVisible());

    if      (d->isVisible())
    {
        show();
    }
    else if (!d->controller()->shallBeShown() || !d->stayAlwaysVisible)
    {
        hide();
    }
}

void AnimatedClearButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT clicked();
    }
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN CustomStepsDoubleSpinBox::Private
{
public:

    Private() = default;

public:

    bool          beforeInitialValue    = true;
    QList<double> values;
    double        initialValue          = 0.0;
    double        smallerStep           = 0.0;
    double        largerStep            = 0.0;
    bool          invertStepping        = false;
};

CustomStepsDoubleSpinBox::CustomStepsDoubleSpinBox(QWidget* const parent)
    : QDoubleSpinBox(parent),
      d             (new Private)
{
}

CustomStepsDoubleSpinBox::~CustomStepsDoubleSpinBox()
{
    delete d;
}

void CustomStepsDoubleSpinBox::stepBy(int steps)
{
    if (d->invertStepping)
    {
        steps = -steps;
    }

    if (d->values.isEmpty())
    {
        QDoubleSpinBox::stepBy(steps);
        return;
    }

    if (d->beforeInitialValue && (d->initialValue > minimum()))
    {
        setValue(d->initialValue);
        return;
    }

    double v = value();

    if ((v >= d->values.first()) && (v <= d->values.last()))
    {
        int nextStep = 0;

        if (steps > 0)
        {
            // find the next value in d->values after current value

            for (nextStep = 0 ; nextStep < d->values.count() ; ++nextStep)
            {
                if (v <= d->values.at(nextStep))
                {
                    ++nextStep;
                    break;
                }
            }

            // go as many steps in d->values as we need

            int stepsToGo = steps;

            for ( ; (stepsToGo > 0) && (nextStep < d->values.count()) ; --stepsToGo)
            {
                v = d->values.at(nextStep++);
            }

            // set the new value

            setValue(v);

            // if anything is left, use Qt code

            if (stepsToGo)
            {
                QDoubleSpinBox::stepBy(stepsToGo);
            }
        }
        else
        {
            for (nextStep = d->values.count() - 1 ; nextStep >= 0 ; --nextStep)
            {
                if (v >= d->values.at(nextStep))
                {
                    --nextStep;
                    break;
                }
            }

            int stepsToGo = -steps;

            for ( ; (stepsToGo > 0) && (nextStep >= 0) ; --stepsToGo)
            {
                v = d->values.at(nextStep--);
            }

            setValue(v);

            if (stepsToGo)
            {
                QDoubleSpinBox::stepBy(-stepsToGo);
            }
        }
    }
    else
    {
        QDoubleSpinBox::stepBy(steps);
    }
}

void CustomStepsDoubleSpinBox::setSuggestedValues(const QList<double>& values)
{
    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));

    d->values = values;
    std::sort(d->values.begin(), d->values.end());
}

void CustomStepsDoubleSpinBox::setSuggestedInitialValue(double initialValue)
{
    d->initialValue = initialValue;
}

void CustomStepsDoubleSpinBox::setSingleSteps(double smaller, double larger)
{
    d->smallerStep = smaller;
    d->largerStep  = larger;
}

void CustomStepsDoubleSpinBox::setInvertStepping(bool invert)
{
    d->invertStepping = invert;
}

void CustomStepsDoubleSpinBox::reset()
{
    setValue(minimum());
    d->beforeInitialValue = true;
}

void CustomStepsDoubleSpinBox::slotValueChanged(double val)
{
    if (val != minimum())
    {
        d->beforeInitialValue = false;
    }

    if (!d->values.isEmpty())
    {
        if      (d->largerStep && val >= d->values.last())
        {
            setSingleStep(d->largerStep);
        }
        else if (d->smallerStep)
        {
            setSingleStep(d->smallerStep);
        }
    }
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN CustomStepsIntSpinBox::Private
{
public:

    Private() = default;

public:

    bool       beforeInitialValue   = true;
    QList<int> values;
    int        initialValue         = 0;
    int        smallerStep          = 0;
    int        largerStep           = 0;
    bool       invertStepping       = false;
    QString    fractionPrefix;
    QString    fractionSpecialValueText;
};

CustomStepsIntSpinBox::CustomStepsIntSpinBox(QWidget* const parent)
    : QSpinBox(parent),
      d       (new Private)
{
}

CustomStepsIntSpinBox::~CustomStepsIntSpinBox()
{
    delete d;
}

void CustomStepsIntSpinBox::stepBy(int steps)
{
    if (d->invertStepping)
    {
        steps = -steps;
    }

    if (d->values.isEmpty())
    {
        QSpinBox::stepBy(steps);
        return;
    }

    if (d->beforeInitialValue && d->initialValue > minimum())
    {
        setValue(d->initialValue);
        return;
    }

    int v = value();

    if ((v >= d->values.first()) && (v <= d->values.last()))
    {
        int nextStep = 0;

        if (steps > 0)
        {
            // find the next value in d->values after current value

            for (nextStep = 0 ; nextStep < d->values.count() ; ++nextStep)
            {
                if (v <= d->values.at(nextStep))
                {
                    ++nextStep;
                    break;
                }
            }

            // go as many steps in d->values as we need

            int stepsToGo = steps;

            for ( ; (stepsToGo > 0) && (nextStep < d->values.count()) ; --stepsToGo)
            {
                v = d->values.at(nextStep++);
            }

            // set the new value

            setValue(v);

            // if anything is left, use Qt code

            if (stepsToGo)
            {
                QSpinBox::stepBy(stepsToGo);
            }
        }
        else
        {
            for ((nextStep = d->values.count() - 1) ; (nextStep >= 0) ; --nextStep)
            {
                if (v >= d->values.at(nextStep))
                {
                    --nextStep;
                    break;
                }
            }

            int stepsToGo = -steps;

            for ( ; (stepsToGo > 0) && (nextStep >= 0) ; --stepsToGo)
            {
                v = d->values.at(nextStep--);
            }

            setValue(v);

            if (stepsToGo)
            {
                QSpinBox::stepBy(-stepsToGo);
            }
        }
    }
    else
    {
        QSpinBox::stepBy(steps);
    }
}

void CustomStepsIntSpinBox::setSuggestedValues(const QList<int>& values)
{
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChanged(int)));

    d->values = values;
    std::sort(d->values.begin(), d->values.end());
}

void CustomStepsIntSpinBox::setSuggestedInitialValue(int initialValue)
{
    d->initialValue = initialValue;
}

void CustomStepsIntSpinBox::setSingleSteps(int smaller, int larger)
{
    d->smallerStep = smaller;
    d->largerStep  = larger;
}

void CustomStepsIntSpinBox::setInvertStepping(bool invert)
{
    d->invertStepping = invert;
}

void CustomStepsIntSpinBox::enableFractionMagic(const QString& prefix)
{
    d->fractionPrefix = prefix;
    std::sort(d->values.begin(), d->values.end(), std::greater<int>());
}

void CustomStepsIntSpinBox::reset()
{
    setValue(minimum());
    d->beforeInitialValue = true;
}

QString CustomStepsIntSpinBox::textFromValue(int value) const
{
    // reimplemented for fraction magic handling

    if (d->fractionPrefix.isNull())
    {
        return QSpinBox::textFromValue(value);
    }

    if (value < 0)
    {
        return d->fractionPrefix + QSpinBox::textFromValue(- value);
    }
    else
    {
        return QSpinBox::textFromValue(value);
    }
}

int CustomStepsIntSpinBox::valueFromText(const QString& text) const
{
    // reimplemented for fraction magic handling

    if (d->fractionPrefix.isNull())
    {
        return QSpinBox::valueFromText(text);
    }

    if (text.startsWith(d->fractionPrefix))
    {
        return (- QSpinBox::valueFromText(text.mid(d->fractionPrefix.length())));
    }
    else
    {
        return QSpinBox::valueFromText(text);
    }
}

QAbstractSpinBox::StepEnabled CustomStepsIntSpinBox::stepEnabled() const
{
    if (d->fractionPrefix.isNull())
    {
        return QSpinBox::stepEnabled();
    }

    QAbstractSpinBox::StepEnabled s;

    if ((value() > minimum()) || (value() == minimum()))
    {
        s |= QAbstractSpinBox::StepUpEnabled;
    }

    if (value() < maximum())
    {
        s |= QAbstractSpinBox::StepDownEnabled;
    }

    return s;
}

double CustomStepsIntSpinBox::fractionMagicValue() const
{
    if (d->fractionPrefix.isNull())
    {
        return value();
    }

    int v = QSpinBox::value();

    if (v < 0)
    {
        return (- 1.0 / v);
    }
    else
    {
        return v;
    }
}

void CustomStepsIntSpinBox::setFractionMagicValue(double value)
{
    if (d->fractionPrefix.isNull())
    {
        setValue((int)value);
        return;
    }

    if (value < 1.0)
    {
        setValue(- lround(1.0 / value));
    }
    else
    {
        setValue((int)value);
    }
}

void CustomStepsIntSpinBox::slotValueChanged(int val)
{
    if (val != minimum())
    {
        d->beforeInitialValue = false;
    }

    if (!d->values.isEmpty())
    {
        if      (d->largerStep && (val >= d->values.last()))
        {
            setSingleStep(d->largerStep);
        }
        else if (d->smallerStep)
        {
            setSingleStep(d->smallerStep);
        }
    }
}

// ------------------------------------------------------------------------

StyleSheetDebugger::StyleSheetDebugger(QWidget* const object)
    : QWidget(nullptr),
      m_widget(object)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* const vbox = new QVBoxLayout;
    m_edit                  = new QTextEdit;

    m_okButton = new QPushButton(i18n("OK"));
    m_okButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));

    vbox->addWidget(m_edit,     1);
    vbox->addWidget(m_okButton, 0, Qt::AlignRight);
    setLayout(vbox);

    connect(m_okButton, SIGNAL(clicked()),
            this, SLOT(buttonClicked()));

    m_edit->setPlainText(m_widget->styleSheet());

    resize(400, 300);
    show();
}

void StyleSheetDebugger::buttonClicked()
{
    m_widget->setStyleSheet(m_edit->toPlainText());
}

} // namespace Digikam

#include "searchutilities.moc"

#include "moc_searchutilities.cpp"
