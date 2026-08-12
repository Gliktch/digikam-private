/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizard page.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dwizardpage.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QWizard>

// Local includes

#include "digikam_globals.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN DWizardPage::Private
{
public:

    Private() = default;

public:

    bool         isComplete      = true;
    int          id              = -1;

    QWidget*     leftView        = nullptr;
    QLabel*      logo            = nullptr;
    QLabel*      leftBottomPix   = nullptr;
    DLineWidget* vline           = nullptr;

    QHBoxLayout* hlay            = nullptr;
    QVBoxLayout* vlay            = nullptr;

    QWizard*     dlg             = nullptr;
    QScrollArea* sv              = nullptr;

    const int spacing            = layoutSpacing();
};

DWizardPage::DWizardPage(QWizard* const dlg, const QString& title)
    : QWizardPage(dlg),
      d          (new Private)
{
    setTitle(title);

    d->sv      = new QScrollArea(this);
    QWidget* const panel       = new QWidget(d->sv->viewport());
    d->sv->setWidget(panel);
    d->sv->setWidgetResizable(true);

    d->hlay                    = new QHBoxLayout(panel);
    d->leftView                = new QWidget(panel);
    QVBoxLayout* const vboxLay = new QVBoxLayout(d->leftView);
    d->logo                    = new QLabel(d->leftView);
    d->logo->setAlignment(Qt::AlignTop);
    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(128, 128)));

    QWidget* const space       = new QLabel(d->leftView);
    d->leftBottomPix           = new QLabel(d->leftView);
    d->leftBottomPix->setAlignment(Qt::AlignBottom);

    vboxLay->addWidget(d->logo);
    vboxLay->addWidget(space);
    vboxLay->addWidget(d->leftBottomPix);
    vboxLay->setStretchFactor(space, 10);
    vboxLay->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    vboxLay->setSpacing(d->spacing);

    d->vline                   = new DLineWidget(Qt::Vertical, panel);

    d->hlay->addWidget(d->leftView);
    d->hlay->addWidget(d->vline);
    d->hlay->setContentsMargins(QMargins(d->spacing, d->spacing, d->spacing, d->spacing));
    d->hlay->setSpacing(d->spacing);

    d->vlay = new QVBoxLayout;
    d->vlay->addWidget(d->sv);
    setLayout(d->vlay);

    d->dlg = dlg;
    d->id  = d->dlg->addPage(this);
}

DWizardPage::~DWizardPage()
{
    delete d;
}

void DWizardPage::setComplete(bool b)
{
    d->isComplete = b;

    Q_EMIT completeChanged();
}

bool DWizardPage::isComplete() const
{
    return d->isComplete;
}

int DWizardPage::id() const
{
    return d->id;
}

void DWizardPage::setShowLeftView(bool v)
{
    d->leftView->setVisible(v);
    d->vline->setVisible(v);

    if (v)
    {
        d->hlay->setContentsMargins(QMargins(d->spacing, d->spacing, d->spacing, d->spacing));
        d->hlay->setSpacing(d->spacing);
        d->sv->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
        d->vlay->setContentsMargins(QMargins(d->spacing, d->spacing, d->spacing, d->spacing));
        d->vlay->setSpacing(d->spacing);
    }
    else
    {
        d->hlay->setContentsMargins(QMargins(0, 0, 0, 0));
        d->hlay->setSpacing(0);
        d->sv->setContentsMargins(0, 0, 0, 0);
        d->vlay->setContentsMargins(QMargins(0, 0, 0, 0));
        d->vlay->setSpacing(0);
    }
}

void DWizardPage::setPageWidget(QWidget* const w)
{
    d->hlay->addWidget(w);
    d->hlay->setStretchFactor(w, 10);
}

void DWizardPage::removePageWidget(QWidget* const w)
{
    d->hlay->removeWidget(w);
}

void DWizardPage::setLeftBottomPix(const QPixmap& pix)
{
    d->leftBottomPix->setPixmap(pix);
}

void DWizardPage::setLeftBottomPix(const QIcon& icon)
{
    d->leftBottomPix->setPixmap(icon.pixmap(128));
}

QWizard* DWizardPage::assistant() const
{
    return d->dlg;
}

} // namespace Digikam

#include "moc_dwizardpage.cpp"
