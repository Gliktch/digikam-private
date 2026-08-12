/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-28
 * Description : a dialog for showing the advancedrename tooltip
 *
 * SPDX-FileCopyrightText: 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tooltipdialog.h"

// Qt includes

#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "tooltipcreator.h"

namespace Digikam
{

class Q_DECL_HIDDEN TooltipDialog::Private
{
public:

    Private() = default;

    QDialogButtonBox* buttons     = nullptr;
    QTextBrowser*     textBrowser = nullptr;
};

TooltipDialog::TooltipDialog(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setWindowTitle(i18nc("@title:window", "Information"));

    d->buttons             = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Help, this);
    d->buttons->button(QDialogButtonBox::Close)->setDefault(true);

    d->textBrowser         = new QTextBrowser(this);
    d->textBrowser->setFrameStyle(QFrame::NoFrame);
    d->textBrowser->setOpenLinks(true);
    d->textBrowser->setOpenExternalLinks(true);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->textBrowser);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

TooltipDialog::~TooltipDialog()
{
    delete d;
}

void TooltipDialog::setTooltip(const QString& tooltip)
{
    clearTooltip();

    d->textBrowser->setHtml(tooltip);
}

void TooltipDialog::clearTooltip()
{
    d->textBrowser->clear();
}

void TooltipDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("main_window"),
                            QLatin1String("image_view"),
                            QLatin1String("renaming-photograph"));
}

} // namespace Digikam

#include "moc_tooltipdialog.cpp"
