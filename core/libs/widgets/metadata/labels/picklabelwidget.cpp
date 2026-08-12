/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "picklabelwidget_p.h"

namespace Digikam
{

PickLabelWidget::PickLabelWidget(QWidget* const parent)
    : DVBox(parent),
      d    (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    DHBox* const hbox = new DHBox(this);
    hbox->setContentsMargins(QMargins());
    hbox->setSpacing(0);

    d->btnNone = new QToolButton(hbox);
    d->btnNone->setCheckable(true);
    d->btnNone->setFocusPolicy(Qt::NoFocus);
    d->btnNone->setIcon(buildIcon(NoPickLabel));
    d->btnNone->installEventFilter(this);

    d->btnRej = new QToolButton(hbox);
    d->btnRej->setCheckable(true);
    d->btnRej->setFocusPolicy(Qt::NoFocus);
    d->btnRej->setIcon(buildIcon(RejectedLabel));
    d->btnRej->installEventFilter(this);

    d->btnPndg = new QToolButton(hbox);
    d->btnPndg->setCheckable(true);
    d->btnPndg->setFocusPolicy(Qt::NoFocus);
    d->btnPndg->setIcon(buildIcon(PendingLabel));
    d->btnPndg->installEventFilter(this);

    d->btnAccpt = new QToolButton(hbox);
    d->btnAccpt->setCheckable(true);
    d->btnAccpt->setFocusPolicy(Qt::NoFocus);
    d->btnAccpt->setIcon(buildIcon(AcceptedLabel));
    d->btnAccpt->installEventFilter(this);

    d->pickBtns = new QButtonGroup(hbox);
    d->pickBtns->addButton(d->btnNone,  NoPickLabel);
    d->pickBtns->addButton(d->btnRej,   RejectedLabel);
    d->pickBtns->addButton(d->btnPndg,  PendingLabel);
    d->pickBtns->addButton(d->btnAccpt, AcceptedLabel);

    d->descBox  = new DHBox(this);
    d->descBox->setContentsMargins(QMargins());
    d->descBox->setSpacing(0);
    d->desc     = new QLabel(d->descBox);
    d->shortcut = new DAdjustableLabel(d->descBox);
    QFont fnt = d->shortcut->font();
    fnt.setItalic(true);
    d->shortcut->setFont(fnt);
    d->shortcut->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->shortcut->setWordWrap(false);

    setSpacing(0);
    setContentsMargins(QMargins());
    setPickLabels(QList<PickLabel>() << NoPickLabel);
    setDescriptionBoxVisible(true);
    setButtonsExclusive(true);

    // -------------------------------------------------------------

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->pickBtns, SIGNAL(idReleased(int)),
            this, SIGNAL(signalPickLabelChanged(int)));

#else

    connect(d->pickBtns, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPickLabelChanged(int)));

#endif

}

PickLabelWidget::~PickLabelWidget()
{
    delete d;
}

void PickLabelWidget::setDescriptionBoxVisible(bool b)
{
    d->descBox->setVisible(b);

    if (!b)
    {
        const auto btns = d->pickBtns->buttons();

        for (QAbstractButton* const btn : btns)
        {
            PickLabel id = (PickLabel)(d->pickBtns->id(btn));
            btn->setToolTip(labelPickName(id));
        }
    }
}

void PickLabelWidget::setButtonsExclusive(bool b)
{
    d->pickBtns->setExclusive(b);
}

void PickLabelWidget::updateDescription(PickLabel label)
{
    d->desc->setText(labelPickName(label));

    DXmlGuiWindow* const app = dynamic_cast<DXmlGuiWindow*>(qApp->activeWindow());

    if (app)
    {
        QAction* const ac = app->actionCollection()->action(QString::fromLatin1("pickshortcut-%1").arg(label));

        if (ac)
        {
            d->shortcut->setAdjustedText(ac->shortcut().toString());
        }
    }
}

bool PickLabelWidget::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == d->btnNone)
    {
        if (ev->type() == QEvent::Enter)
        {
            updateDescription(NoPickLabel);

            return false;
        }
    }

    if (obj == d->btnRej)
    {
        if (ev->type() == QEvent::Enter)
        {
            updateDescription(RejectedLabel);

            return false;
        }
    }

    if (obj == d->btnPndg)
    {
        if (ev->type() == QEvent::Enter)
        {
            updateDescription(PendingLabel);

            return false;
        }
    }

    if (obj == d->btnAccpt)
    {
        if (ev->type() == QEvent::Enter)
        {
            updateDescription(AcceptedLabel);

            return false;
        }
    }

    // pass the event on to the parent class

    return DVBox::eventFilter(obj, ev);
}

void PickLabelWidget::setPickLabels(const QList<PickLabel>& list)
{
    const auto btns = d->pickBtns->buttons();

    for (QAbstractButton* const btn : btns)
    {
        PickLabel id = (PickLabel)(d->pickBtns->id(btn));
        btn->setChecked(list.contains(id));
        updateDescription(id);
    }
}

QList<PickLabel> PickLabelWidget::pickLabels() const
{
    QList<PickLabel> list;

    const auto btns = d->pickBtns->buttons();

    for (QAbstractButton* const btn : btns)
    {
        if (btn && btn->isChecked())
        {
            list.append((PickLabel)(d->pickBtns->id(btn)));
        }
    }

    return list;
}

QIcon PickLabelWidget::buildIcon(PickLabel label)
{
    switch (label)
    {
        case RejectedLabel:
        {
            return QIcon::fromTheme(QLatin1String("flag-red"));
        }

        case PendingLabel:
        {
            return QIcon::fromTheme(QLatin1String("flag-yellow"));
        }

        case AcceptedLabel:
        {
            return QIcon::fromTheme(QLatin1String("flag-green"));
        }

        default:
        {
            break;
        }
    }

    // default : NoPickLabel

    return QIcon::fromTheme(QLatin1String("flag-black"));
}

QString PickLabelWidget::labelPickName(PickLabel label)
{
    QString name;

    switch (label)
    {
        case RejectedLabel:
        {
            name = i18nc("@info: pick label name", "Rejected");
            break;
        }

        case PendingLabel:
        {
            name = i18nc("@info: pick label name", "Pending");
            break;
        }

        case AcceptedLabel:
        {
            name = i18nc("@info: pick label name", "Accepted");
            break;
        }

        default:   // NoPickLabel
        {
            name = i18nc("@info: pick label name", "No Pick");
            break;
        }
    }

    return name;
}

} // namespace Digikam

#include "moc_picklabelwidget.cpp"
