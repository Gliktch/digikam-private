/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : A widget to host settings as expander box
 *
 * SPDX-FileCopyrightText: 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Manuel Viet <contact at 13zenrv dot fr>
 * SPDX-FileCopyrightText: 2001      by Frerich Raabe <raabe at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dexpanderbox.h"

// Qt includes

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QScreen>
#include <QWindow>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "thememanager.h"

namespace Digikam
{

DLineWidget::DLineWidget(Qt::Orientation orientation, QWidget* const parent)
    : QFrame(parent)
{
    setLineWidth(1);
    setMidLineWidth(0);
    setFrameShadow(QFrame::Sunken);

    if (orientation == Qt::Vertical)
    {
        setFrameShape(QFrame::VLine);
        setMinimumSize(2, 0);
    }
    else
    {
        setFrameShape(QFrame::HLine);
        setMinimumSize(0, 2);
    }

    updateGeometry();
}

DLineWidget::~DLineWidget()
{
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DAdjustableLabel::Private
{
public:

    Private() = default;

public:

    QString           ajdText;
    Qt::TextElideMode emode = Qt::ElideMiddle;
};

DAdjustableLabel::DAdjustableLabel(QWidget* const parent)
    : QLabel(parent),
      d     (new Private)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

DAdjustableLabel::~DAdjustableLabel()
{
    delete d;
}

void DAdjustableLabel::resizeEvent(QResizeEvent*)
{
    adjustTextToLabel();
}

QSize DAdjustableLabel::minimumSizeHint() const
{
    QSize sh = QLabel::minimumSizeHint();
    sh.setWidth(-1);

    return sh;
}

QSize DAdjustableLabel::sizeHint() const
{
    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QFontMetrics fm(fontMetrics());
    int maxW     = screen->geometry().width() * 3 / 4;
    int currentW = fm.horizontalAdvance(d->ajdText);

    return (QSize((currentW > maxW) ? maxW : currentW, QLabel::sizeHint().height()));
}

void DAdjustableLabel::setAdjustedText(const QString& text)
{
    d->ajdText = text;

    if (d->ajdText.isNull())
    {
        QLabel::clear();
    }

    adjustTextToLabel();
}

QString DAdjustableLabel::adjustedText() const
{
    return d->ajdText;
}

void DAdjustableLabel::setAlignment(Qt::Alignment alignment)
{
    QString tmp(d->ajdText);
    QLabel::setAlignment(alignment);
    d->ajdText = tmp;
}

void DAdjustableLabel::setElideMode(Qt::TextElideMode mode)
{
    d->emode = mode;
    adjustTextToLabel();
}

void DAdjustableLabel::adjustTextToLabel()
{
    QFontMetrics fm(fontMetrics());
    QStringList adjustedLines;
    int lblW         = size().width();
    bool adjusted    = false;
    const auto alist = d->ajdText.split(QLatin1Char('\n'));

    for (const QString& line : alist)
    {
        int lineW = fm.horizontalAdvance(line);

        if (lineW > lblW)
        {
            adjusted = true;
            adjustedLines << fm.elidedText(line, d->emode, lblW);
        }
        else
        {
            adjustedLines << line;
        }
    }

    if (adjusted)
    {
        QLabel::setText(adjustedLines.join(QLatin1Char('\n')));
        setToolTip(d->ajdText);
    }
    else
    {
        QLabel::setText(d->ajdText);
        setToolTip(QString());
    }
}

// ------------------------------------------------------------------------------------

DClickLabel::DClickLabel(QWidget* const parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

DClickLabel::DClickLabel(const QString& text, QWidget* const parent)
    : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

DClickLabel::~DClickLabel()
{
}

void DClickLabel::mousePressEvent(QMouseEvent* event)
{
    QLabel::mousePressEvent(event);

    /*
     * In some contexts, like QGraphicsView, there will be no
     * release event if the press event was not accepted.
     */
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
    }
}

void DClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT leftClicked();
        Q_EMIT activated();

        event->accept();
    }
}

void DClickLabel::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Down:
        case Qt::Key_Right:
        case Qt::Key_Space:
        {
            Q_EMIT activated();

            return;
        }

        default:
        {
            break;
        }
    }

    QLabel::keyPressEvent(e);
}

// ------------------------------------------------------------------------

DSqueezedClickLabel::DSqueezedClickLabel(QWidget* const parent)
    : DAdjustableLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

DSqueezedClickLabel::DSqueezedClickLabel(const QString& text, QWidget* const parent)
    : DAdjustableLabel(parent)
{
    setAdjustedText(text);
    setCursor(Qt::PointingHandCursor);
}

DSqueezedClickLabel::~DSqueezedClickLabel()
{
}

void DSqueezedClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT leftClicked();
        Q_EMIT activated();

        event->accept();
    }
}

void DSqueezedClickLabel::mousePressEvent(QMouseEvent* event)
{
    QLabel::mousePressEvent(event);

    /*
     * In some contexts, like QGraphicsView, there will be no
     * release event if the press event was not accepted.
     */
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
    }
}

void DSqueezedClickLabel::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Down:
        case Qt::Key_Right:
        case Qt::Key_Space:
        {
            Q_EMIT activated();

            return;
        }

        default:
        {
            break;
        }
    }

    QLabel::keyPressEvent(e);
}

// ------------------------------------------------------------------------

DArrowClickLabel::DArrowClickLabel(QWidget* const parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void DArrowClickLabel::setArrowType(Qt::ArrowType type)
{
    m_arrowType = type;
    update();
}

Qt::ArrowType DArrowClickLabel::arrowType() const
{
    return m_arrowType;
}

void DArrowClickLabel::mousePressEvent(QMouseEvent* event)
{
    /*
     * In some contexts, like QGraphicsView, there will be no
     * release event if the press event was not accepted.
     */
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
    }
}

void DArrowClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT leftClicked();
    }
}

void DArrowClickLabel::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.lineWidth    = 2;
    opt.midLineWidth = 0;

/*
    p.fillRect( rect(), palette().brush( QPalette::Window ) );
    style()->drawPrimitive( QStyle::PE_Frame, &opt, &p, this);
*/

    if (m_arrowType == Qt::NoArrow)
    {
        return;
    }

    if (
        (width()  < (m_size + m_margin)) ||
        (height() < (m_size + m_margin))
       )
    {
        return; // don't draw arrows if we are too small
    }

    unsigned int x = 0;
    unsigned int y = 0;

    if      (m_arrowType == Qt::DownArrow)
    {
        x = (width() - m_size) / 2;
        y = height() - (m_size + m_margin);
    }
    else if (m_arrowType == Qt::UpArrow)
    {
        x = (width() - m_size) / 2;
        y = m_margin;
    }
    else if (m_arrowType == Qt::RightArrow)
    {
        x = width() - (m_size + m_margin);
        y = (height() - m_size) / 2;
    }
    else // arrowType == LeftArrow
    {
        x = m_margin;
        y = (height() - m_size) / 2;
    }

/*
    if (isDown())
    {
        ++x;
        ++y;
    }
*/

    QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowLeft;

    switch (m_arrowType)
    {
        case Qt::LeftArrow:
        {
            e = QStyle::PE_IndicatorArrowLeft;
            break;
        }

        case Qt::RightArrow:
        {
            e = QStyle::PE_IndicatorArrowRight;
            break;
        }

        case Qt::UpArrow:
        {
            e = QStyle::PE_IndicatorArrowUp;
            break;
        }

        case Qt::DownArrow:
        {
            e = QStyle::PE_IndicatorArrowDown;
            break;
        }

        case Qt::NoArrow:
        {
            break;
        }
    }

    opt.state |= QStyle::State_Enabled;
    opt.rect   = QRect( x, y, m_size, m_size);

    style()->drawPrimitive( e, &opt, &p, this );
}

QSize DArrowClickLabel::sizeHint() const
{
    return QSize(m_size + 2*m_margin, m_size + 2*m_margin);
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN DLabelExpander::Private
{
public:

    Private() = default;

public:

    bool              expandByDefault   = true;

    QCheckBox*        checkBox          = nullptr;
    QLabel*           pixmapLabel       = nullptr;
    QToolButton*      btn               = nullptr;
    QWidget*          containerWidget   = nullptr;
    QGridLayout*      grid              = nullptr;

    DLineWidget*      line              = nullptr;
    QWidget*          hbox              = nullptr;

    QIcon             icon;

    DArrowClickLabel* arrow             = nullptr;
    DClickLabel*      clickLabel        = nullptr;
};

DLabelExpander::DLabelExpander(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = layoutSpacing();


    d->grid                 = new QGridLayout(this);
    d->line                 = new DLineWidget(Qt::Horizontal, this);
    d->hbox                 = new QWidget(this);
    d->arrow                = new DArrowClickLabel(d->hbox);
    d->checkBox             = new QCheckBox(d->hbox);
    d->pixmapLabel          = new QLabel(d->hbox);
    d->clickLabel           = new DClickLabel(d->hbox);
    d->btn                  = new QToolButton(d->hbox);

    QHBoxLayout* const hlay = new QHBoxLayout(d->hbox);
    hlay->addWidget(d->arrow);
    hlay->addWidget(d->checkBox);
    hlay->addWidget(d->pixmapLabel);
    hlay->addWidget(d->clickLabel, 10);
    hlay->addWidget(d->btn);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(spacing);

    d->pixmapLabel->installEventFilter(this);
    d->pixmapLabel->setCursor(Qt::PointingHandCursor);

    d->hbox->setCursor(Qt::PointingHandCursor);
    setCheckBoxVisible(false);
    setButtonVisible(false);

    d->grid->addWidget(d->line, 0, 0, 1, 3);
    d->grid->addWidget(d->hbox, 1, 0, 1, 3);
    d->grid->setColumnStretch(2, 10);
    d->grid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->grid->setSpacing(spacing);

    connect(d->arrow, &DArrowClickLabel::leftClicked,
            this, &DLabelExpander::slotToggleContainer);

    connect(d->clickLabel, &DClickLabel::activated,
            this, &DLabelExpander::slotToggleContainer);

    connect(d->checkBox, &QCheckBox::toggled,
            this, &DLabelExpander::signalToggled);

    connect(d->btn, &QToolButton::pressed,
            this, &DLabelExpander::signalButtonPressed);

    connect(ThemeManager::instance(), &ThemeManager::signalThemeChanged,
            this, &DLabelExpander::slotUpdateIcon);
}

DLabelExpander::~DLabelExpander()
{
    delete d;
}

void DLabelExpander::setCheckBoxVisible(bool b)
{
    d->checkBox->setVisible(b);
}

bool DLabelExpander::checkBoxIsVisible() const
{
    return d->checkBox->isVisible();
}

void DLabelExpander::setChecked(bool b)
{
    d->checkBox->setChecked(b);
}

bool DLabelExpander::isChecked() const
{
    return d->checkBox->isChecked();
}

void DLabelExpander::setLineVisible(bool b)
{
    d->line->setVisible(b);
}

bool DLabelExpander::lineIsVisible() const
{
    return d->line->isVisible();
}

void DLabelExpander::setText(const QString& txt)
{
    d->clickLabel->setText(QString::fromUtf8("<qt><b>%1</b></qt>").arg(txt));
}

QString DLabelExpander::text() const
{
    return d->clickLabel->text();
}

void DLabelExpander::setIcon(const QIcon& icon)
{
    d->icon = icon;
    slotUpdateIcon();
}

QIcon DLabelExpander::icon() const
{
    return d->icon;
}

void DLabelExpander::setButtonVisible(bool b)
{
    d->btn->setVisible(b);
}

bool DLabelExpander::buttonIsVisible() const
{
    return d->btn->isVisible();
}

void DLabelExpander::setButtonIcon(const QIcon& icon)
{
    d->btn->setIcon(icon);
    d->btn->setIconSize(QSize(style()->pixelMetric(QStyle::PM_SmallIconSize),
                              style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

void DLabelExpander::setButtonToolTip(const QString& tip)
{
    d->btn->setToolTip(tip);
}

void DLabelExpander::setWidget(QWidget* const widget)
{
    if (widget)
    {
        d->containerWidget = widget;
        d->containerWidget->setParent(this);
        d->grid->addWidget(d->containerWidget, 2, 0, 1, 3);
    }
}

QWidget* DLabelExpander::widget() const
{
    return d->containerWidget;
}

void DLabelExpander::setExpandByDefault(bool b)
{
    d->expandByDefault = b;
}

bool DLabelExpander::isExpandByDefault() const
{
    return d->expandByDefault;
}

void DLabelExpander::setExpanded(bool b)
{
    if (d->containerWidget)
    {
        d->containerWidget->setVisible(b);

        if (b)
        {
           d->arrow->setArrowType(Qt::DownArrow);
        }
        else
        {
           d->arrow->setArrowType(Qt::RightArrow);
        }
    }

    Q_EMIT signalExpanded(b);
}

bool DLabelExpander::isExpanded() const
{
    return (d->arrow->arrowType() == Qt::DownArrow);
}

void DLabelExpander::slotToggleContainer()
{
    if (d->containerWidget)
    {
        setExpanded(!d->containerWidget->isVisible());
    }
}

bool DLabelExpander::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == d->pixmapLabel)
    {
        if (ev->type() == QEvent::MouseButtonRelease)
        {
            slotToggleContainer();
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // pass the event on to the parent class

        return QWidget::eventFilter(obj, ev);
    }
}

void DLabelExpander::slotUpdateIcon()
{
    d->pixmapLabel->setPixmap(d->icon.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN DExpanderBox::Private
{
public:

    explicit Private(DExpanderBox* const box)
      : parent(box)
    {
    }

    void createItem(int index, QWidget* const w, const QIcon& icon, const QString& txt,
                    const QString& objName, bool expandBydefault)
    {
        DLabelExpander* const exp = new DLabelExpander(parent->viewport());
        exp->setText(txt);
        exp->setIcon(icon);
        exp->setWidget(w);
        exp->setLineVisible(!wList.isEmpty());
        exp->setObjectName(objName);
        exp->setExpandByDefault(expandBydefault);

        if (index >= 0)
        {
            vbox->insertWidget(index, exp);
            wList.insert(index, exp);
        }
        else
        {
            vbox->addWidget(exp);
            wList.append(exp);
        }

        parent->connect(exp, SIGNAL(signalExpanded(bool)),
                        parent, SLOT(slotItemExpanded(bool)));

        parent->connect(exp, SIGNAL(signalToggled(bool)),
                        parent, SLOT(slotItemToggled(bool)));

        parent->connect(exp, SIGNAL(signalButtonPressed()),
                        parent, SLOT(slotItemButtonPressed()));
    }

public:

    QList<DLabelExpander*> wList;

    QVBoxLayout*           vbox     = nullptr;

    DExpanderBox*          parent   = nullptr;
};

DExpanderBox::DExpanderBox(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private(this))
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    QWidget* const main = new QWidget(viewport());
    d->vbox             = new QVBoxLayout(main);
    d->vbox->setSpacing(layoutSpacing());

    d->vbox->setContentsMargins(QMargins());
    setWidget(main);

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    main->setAutoFillBackground(false);
}

DExpanderBox::~DExpanderBox()
{
    d->wList.clear();
    delete d;
}

void DExpanderBox::setCheckBoxVisible(int index, bool b)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setCheckBoxVisible(b);
}

bool DExpanderBox::checkBoxIsVisible(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return false;
    }

    return d->wList[index]->checkBoxIsVisible();
}

void DExpanderBox::setChecked(int index, bool b)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setChecked(b);
}

bool DExpanderBox::isChecked(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return false;
    }

    return d->wList[index]->isChecked();
}

void DExpanderBox::addItem(QWidget* const w, const QIcon& icon, const QString& txt,
                           const QString& objName, bool expandBydefault)
{
    d->createItem(-1, w, icon, txt, objName, expandBydefault);
}

void DExpanderBox::addItem(QWidget* const w, const QString& txt,
                           const QString& objName, bool expandBydefault)
{
    addItem(w, QIcon(), txt, objName, expandBydefault);
}

void DExpanderBox::addStretch()
{
    d->vbox->addStretch(10);
}

void DExpanderBox::insertItem(int index, QWidget* const w, const QIcon& icon, const QString& txt,
                              const QString& objName, bool expandBydefault)
{
    d->createItem(index, w, icon, txt, objName, expandBydefault);
}

void DExpanderBox::slotItemExpanded(bool b)
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (exp)
    {
        int index = indexOf(exp);

        Q_EMIT signalItemExpanded(index, b);
    }
}

void DExpanderBox::slotItemButtonPressed()
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (exp)
    {
        int index = indexOf(exp);

        Q_EMIT signalItemButtonPressed(index);
    }
}

void DExpanderBox::slotItemToggled(bool b)
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (exp)
    {
        int index = indexOf(exp);

        Q_EMIT signalItemToggled(index, b);
    }
}

void DExpanderBox::insertItem(int index, QWidget* const w, const QString& txt,
                              const QString& objName, bool expandBydefault)
{
    insertItem(index, w, QIcon(), txt, objName, expandBydefault);
}

void DExpanderBox::insertStretch(int index)
{
    d->vbox->insertStretch(index, 10);
}

void DExpanderBox::removeItem(int index)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->hide();
    d->wList.removeAt(index);
}

void DExpanderBox::setItemText(int index, const QString& txt)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setText(txt);
}

QString DExpanderBox::itemText(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return QString();
    }

    return d->wList[index]->text();
}

void DExpanderBox::setItemIcon(int index, const QIcon& icon)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setIcon(icon.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

QIcon DExpanderBox::itemIcon(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return QIcon();
    }

    return d->wList[index]->icon();
}

void DExpanderBox::setButtonVisible(int index, bool b)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setButtonVisible(b);
}

bool DExpanderBox::buttonIsVisible(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return false;
    }

    return d->wList[index]->buttonIsVisible();
}

void DExpanderBox::setButtonIcon(int index, const QIcon& icon)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setButtonIcon(icon);
}

void DExpanderBox::setButtonToolTip(int index, const QString& tip)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setToolTip(tip);
}

int DExpanderBox::count() const
{
    return d->wList.count();
}

void DExpanderBox::setItemToolTip(int index, const QString& tip)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setToolTip(tip);
}

QString DExpanderBox::itemToolTip(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return QString();
    }

    return d->wList[index]->toolTip();
}

void DExpanderBox::setItemEnabled(int index, bool enabled)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    d->wList[index]->setEnabled(enabled);
}

bool DExpanderBox::isItemEnabled(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return false;
    }

    return d->wList[index]->isEnabled();
}

DLabelExpander* DExpanderBox::widget(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return nullptr;
    }

    return d->wList[index];
}

int DExpanderBox::indexOf(DLabelExpander* const widget) const
{
    for (int i = 0 ; i < count() ; ++i)
    {
        const DLabelExpander* const exp = d->wList[i];

        if (widget == exp)
        {
            return i;
        }
    }

    return -1;
}

void DExpanderBox::setItemExpanded(int index, bool b)
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return;
    }

    DLabelExpander* const exp = d->wList[index];

    if (!exp) return;

    exp->setExpanded(b);
}

bool DExpanderBox::isItemExpanded(int index) const
{
    if ((index > d->wList.count()) || (index < 0))
    {
        return false;
    }

    const DLabelExpander* const exp = d->wList[index];

    if (!exp)
    {
        return false;
    }

    return (exp->isExpanded());
}

void DExpanderBox::readSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < count() ; ++i)
    {
        DLabelExpander* const exp = d->wList[i];

        if (exp)
        {
            exp->setExpanded(group.readEntry(QString::fromUtf8("%1 Expanded").arg(exp->objectName()),
                                             exp->isExpandByDefault()));
        }
    }
}

void DExpanderBox::writeSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < count() ; ++i)
    {
        DLabelExpander* const exp = d->wList[i];

        if (exp)
        {
            group.writeEntry(QString::fromUtf8("%1 Expanded").arg(exp->objectName()),
                             exp->isExpanded());
        }
    }
}

// ------------------------------------------------------------------------

DExpanderBoxExclusive::DExpanderBoxExclusive(QWidget* const parent)
    : DExpanderBox(parent)
{
}

void DExpanderBoxExclusive::slotItemExpanded(bool b)
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (!exp)
    {
        return;
    }

    if (isToolBox() && b)
    {
        int item = 0;

        while (item < count())
        {
            if (isItemExpanded(item) && (item != indexOf(exp)))
            {
                setItemExpanded(item, false);
            }

            item++;
        }
    }

    Q_EMIT signalItemExpanded(indexOf(exp), b);
}

void DExpanderBoxExclusive::setIsToolBox(bool b)
{
    m_toolbox = b;
}

bool DExpanderBoxExclusive::isToolBox() const
{
    return (m_toolbox);
}

} // namespace Digikam

#include "moc_dexpanderbox.cpp"
