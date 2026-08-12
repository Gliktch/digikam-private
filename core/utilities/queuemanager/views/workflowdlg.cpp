/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-19
 * Description : Workflow properties dialog.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "workflowdlg.h"

// Qt includes

#include <QPointer>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dxmlguiwindow.h"
#include "dexpanderbox.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN WorkflowDlg::Private
{

public:

    Private() = default;

    QDialogButtonBox* buttons   = nullptr;
    DTextEdit*        titleEdit = nullptr;
    DTextEdit*        descEdit  = nullptr;
};

WorkflowDlg::WorkflowDlg(const Workflow& wf, bool create)
    : QDialog(nullptr),
      d      (new Private)
{
    setModal(true);
    setWindowTitle(create ? i18nc("@title:window", "New Workflow") : i18nc("@title:window", "Edit Workflow"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QWidget* const page    = new QWidget(this);
    QLabel* const logo     = new QLabel(page);
    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    QLabel* const topLabel = new QLabel(page);

    if (create)
    {
        topLabel->setText(i18n("<qt><b>Create new Workflow</b></qt>"));
    }
    else
    {
        topLabel->setText(i18n("<qt><b>Workflow Properties</b></qt>"));
    }

    topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topLabel->setWordWrap(false);

    DLineWidget* const topLine = new DLineWidget(Qt::Horizontal);

    // --------------------------------------------------------

    QLabel* const titleLabel    = new QLabel(page);
    titleLabel->setText(i18nc("@title: batch worklow name", "&Title:"));

    d->titleEdit                = new DTextEdit(page);
    d->titleEdit->setLinesVisible(1);
    d->titleEdit->setIgnoredCharacters(QLatin1String("/"));
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();
    titleLabel->setBuddy(d->titleEdit);

    // --------------------------------------------------------

    QLabel* const descLabel = new QLabel(page);
    descLabel->setText(i18n("Description:"));

    d->descEdit             = new DTextEdit(page);
    d->descEdit->setLinesVisible(1);
    d->descEdit->setIgnoredCharacters(QLatin1String("/"));
    descLabel->setBuddy(d->descEdit);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout();
    grid->addWidget(logo,         0, 0, 1, 1);
    grid->addWidget(topLabel,     0, 1, 1, 1);
    grid->addWidget(topLine,      1, 0, 1, 2);
    grid->addWidget(titleLabel,   2, 0, 1, 1);
    grid->addWidget(d->titleEdit, 2, 1, 1, 1);
    grid->addWidget(descLabel,    3, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    grid->addWidget(d->descEdit,  3, 1, 1, 1);
    grid->setSpacing(layoutSpacing());

    grid->setContentsMargins(QMargins());
    page->setLayout(grid);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    if (create)
    {
        d->titleEdit->setText(i18n("New Workflow"));
    }
    else
    {
        d->titleEdit->setText(wf.title);
        d->descEdit->setText(wf.desc);
    }

    // -- slots connections -------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged()),
            this, SLOT(slotTitleChanged()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

WorkflowDlg::~WorkflowDlg()
{
    delete d;
}

QString WorkflowDlg::title() const
{
    return d->titleEdit->text();
}

QString WorkflowDlg::description() const
{
    return d->descEdit->text();
}

bool WorkflowDlg::editProps(Workflow& wf)
{
    QPointer<WorkflowDlg> dlg = new WorkflowDlg(wf);
    bool ok                   = (dlg->exec() == QDialog::Accepted);

    if (ok)
    {
        wf.title = dlg->title();
        wf.desc  = dlg->description();
    }

    delete dlg;

    return ok;
}

bool WorkflowDlg::createNew(Workflow& wf)
{
    QPointer<WorkflowDlg> dlg = new WorkflowDlg(wf, true);
    bool ok                   = (dlg->exec() == QDialog::Accepted);

    if (ok)
    {
        wf.title = dlg->title();
        wf.desc  = dlg->description();
    }

    delete dlg;

    return ok;
}

void WorkflowDlg::slotTitleChanged()
{
    QString text = d->titleEdit->text();
    Workflow wf  = WorkflowManager::instance()->findByTitle(text);
    bool enable  = (wf.title.isEmpty() && !text.isEmpty());
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void WorkflowDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("batch_queue"),
                            QLatin1String("bqm_workflow"));
}

} // namespace Digikam

#include "moc_workflowdlg.cpp"
