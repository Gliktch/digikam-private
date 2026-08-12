/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1999-2001 by Mirko Boehm <mirko at kde dot org>
 * SPDX-FileCopyrightText: 1999-2001 by Espen Sand <espen at kde dot org>
 * SPDX-FileCopyrightText: 1999-2001 by Holger Freyther <freyther at kde dot org>
 * SPDX-FileCopyrightText: 2005-2006 by Olivier Goffart <ogoffart at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dconfigdlg.h"

// Qt includes

#include <QTimer>
#include <QLayout>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN DConfigDlgPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlg)

protected:

    explicit DConfigDlgPrivate(DConfigDlg* const parent)
        : q_ptr(parent)
    {
    }

    virtual ~DConfigDlgPrivate() = default;

    void init()
    {
        Q_Q(DConfigDlg);

        delete q->layout();

        QVBoxLayout* const layout = new QVBoxLayout;
        q->setLayout(layout);

        if (mPageWidget)
        {
            q->connect(mPageWidget, SIGNAL(currentPageChanged(DConfigDlgWdgItem*,DConfigDlgWdgItem*)),
                       q, SIGNAL(currentPageChanged(DConfigDlgWdgItem*,DConfigDlgWdgItem*)));

            q->connect(mPageWidget, &DConfigDlgWdg::pageRemoved,
                       q, &DConfigDlg::pageRemoved);

            layout->addWidget(mPageWidget);
        }
        else
        {
            layout->addStretch();
        }

        if (mButtonBox)
        {
            q->connect(mButtonBox, &QDialogButtonBox::accepted,
                       q, &QDialog::accept);

            q->connect(mButtonBox, &QDialogButtonBox::rejected,
                       q, &QDialog::reject);

            layout->addWidget(mButtonBox);
        }
    }

protected:

    DConfigDlg* const q_ptr         = nullptr;
    DConfigDlgWdg*    mPageWidget   = nullptr;
    QDialogButtonBox* mButtonBox    = nullptr;

    QString configGroupName;

private:

    Q_DISABLE_COPY(DConfigDlgPrivate)
};

// ------------------------------------------------------------------------

DConfigDlg::DConfigDlg(QWidget* const parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      d_ptr  (new DConfigDlgPrivate(this))
{
    Q_D(DConfigDlg);

    d->mPageWidget = new DConfigDlgWdg(this);
    d->mPageWidget->layout()->setContentsMargins(QMargins());

    d->mButtonBox  = new QDialogButtonBox(this);
    d->mButtonBox->setObjectName(QLatin1String("buttonbox"));
    d->mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    d->init();
}

DConfigDlg::DConfigDlg(DConfigDlgWdg* const widget, QWidget* const parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      d_ptr  (new DConfigDlgPrivate(this))
{
    Q_D(DConfigDlg);
    Q_ASSERT(widget);

    widget->setParent(this);
    d->mPageWidget = widget;
    d->mButtonBox  = new QDialogButtonBox(this);
    d->mButtonBox->setObjectName(QLatin1String("buttonbox"));
    d->mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    d->init();
}

DConfigDlg::DConfigDlg(DConfigDlgPrivate& dd, DConfigDlgWdg* const widget, QWidget* const parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      d_ptr  (&dd)
{
    Q_D(DConfigDlg);

    if (widget)
    {
        widget->setParent(this);
        d->mPageWidget = widget;
    }
    else
    {
        d->mPageWidget = new DConfigDlgWdg(this);
    }

    d->mButtonBox = new QDialogButtonBox(this);
    d->mButtonBox->setObjectName(QLatin1String("buttonbox"));
    d->mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    d->init();
}

DConfigDlg::~DConfigDlg()
{
    if (!d_func()->configGroupName.isNull())
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(d_func()->configGroupName);

        DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    }

    delete d_ptr;
}

void DConfigDlg::setFaceType(FaceType faceType)
{
    d_func()->mPageWidget->setFaceType(static_cast<DConfigDlgWdg::FaceType>(faceType));
}

void DConfigDlg::setConfigGroup(const QString& group)
{
    d_func()->configGroupName = group;
}

DConfigDlgWdgItem* DConfigDlg::addPage(QWidget* const widget, const QString& name)
{
    return d_func()->mPageWidget->addPage(widget, name);
}

void DConfigDlg::addPage(DConfigDlgWdgItem* const item)
{
    d_func()->mPageWidget->addPage(item);
}

DConfigDlgWdgItem* DConfigDlg::insertPage(DConfigDlgWdgItem* const before, QWidget* const widget, const QString& name)
{
    return d_func()->mPageWidget->insertPage(before, widget, name);
}

void DConfigDlg::insertPage(DConfigDlgWdgItem* const before, DConfigDlgWdgItem* const item)
{
    d_func()->mPageWidget->insertPage(before, item);
}

DConfigDlgWdgItem* DConfigDlg::addSubPage(DConfigDlgWdgItem* const parent, QWidget* const widget, const QString& name)
{
    return d_func()->mPageWidget->addSubPage(parent, widget, name);
}

void DConfigDlg::addSubPage(DConfigDlgWdgItem* const parent, DConfigDlgWdgItem* const item)
{
    d_func()->mPageWidget->addSubPage(parent, item);
}

void DConfigDlg::removePage(DConfigDlgWdgItem* const item)
{
    d_func()->mPageWidget->removePage(item);
}

void DConfigDlg::setCurrentPage(DConfigDlgWdgItem* const item)
{
    d_func()->mPageWidget->setCurrentPage(item);
}

DConfigDlgWdgItem* DConfigDlg::currentPage() const
{
    return d_func()->mPageWidget->currentPage();
}

void DConfigDlg::setStandardButtons(QDialogButtonBox::StandardButtons buttons)
{
    d_func()->mButtonBox->setStandardButtons(buttons);
}

QPushButton* DConfigDlg::button(QDialogButtonBox::StandardButton which) const
{
    return d_func()->mButtonBox->button(which);
}

void DConfigDlg::addActionButton(QAbstractButton* const button)
{
    d_func()->mButtonBox->addButton(button, QDialogButtonBox::ActionRole);
}

DConfigDlgWdg* DConfigDlg::pageWidget()
{
    return d_func()->mPageWidget;
}

const DConfigDlgWdg* DConfigDlg::pageWidget() const
{
    return d_func()->mPageWidget;
}

void DConfigDlg::setPageWidget(DConfigDlgWdg* const widget)
{
    delete d_func()->mPageWidget;
    d_func()->mPageWidget = widget;
    d_func()->init();
}

QDialogButtonBox* DConfigDlg::buttonBox()
{
    return d_func()->mButtonBox;
}

const QDialogButtonBox* DConfigDlg::buttonBox() const
{
    return d_func()->mButtonBox;
}

void DConfigDlg::setButtonBox(QDialogButtonBox* const box)
{
    delete d_func()->mButtonBox;
    d_func()->mButtonBox = box;
    d_func()->init();
}

void DConfigDlg::showEvent(QShowEvent* e)
{
    if (!d_func()->configGroupName.isNull())
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(d_func()->configGroupName);

        DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
        DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
        resize(windowHandle()->size());
    }

    QDialog::showEvent(e);
}

} // namespace Digikam

#include "moc_dconfigdlg.cpp"
