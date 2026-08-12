/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : a dialog that can be used to display a configuration
 *               dialog for a rule
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ruledialog.h"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// Local includes

#include "digikam_globals.h"
#include "rule.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN RuleDialog::Private
{
public:

    Private() = default;

    QDialogButtonBox* buttons           = nullptr;
    QWidget*          container         = nullptr;
    QLabel*           dialogTitle       = nullptr;
    QLabel*           dialogDescription = nullptr;
    QLabel*           dialogIcon        = nullptr;
    QWidget*          settingsWidget    = nullptr;
};

RuleDialog::RuleDialog(Rule* const parent, QWidget* const widget)
    : QDialog(widget),
      d      (new Private)
{
    d->buttons                = new QDialogButtonBox(QDialogButtonBox::Ok     |
                                                     QDialogButtonBox::Cancel |
                                                     QDialogButtonBox::Help,
                                                     this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->dialogTitle            = new QLabel(this);
    d->dialogDescription      = new QLabel(this);
    d->dialogIcon             = new QLabel(this);

    setDialogTitle(parent->objectName());
    setDialogDescription(parent->description());
    setDialogIcon(parent->icon(Rule::Dialog));

    d->dialogTitle->setAlignment(Qt::AlignHCenter);
    d->dialogDescription->setAlignment(Qt::AlignHCenter);
    d->dialogDescription->setWordWrap(true);

    QFrame* const line        = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QWidget* const header     = new QWidget(this);
    QGridLayout* headerLayout = new QGridLayout(this);
    headerLayout->addWidget(d->dialogIcon,        0, 0, 4, 1);
    headerLayout->addWidget(d->dialogTitle,       1, 1, 1, 1);
    headerLayout->addWidget(d->dialogDescription, 2, 1, 1, 1);
    headerLayout->addWidget(line,                 4, 0, 1, -1);
    headerLayout->setColumnStretch(1, 10);
    header->setLayout(headerLayout);

    d->container                       = new QWidget(this);
    QGridLayout* const containerLayout = new QGridLayout(this);
    containerLayout->addWidget(header,            0, 0, 1, 1);
    containerLayout->addWidget(d->settingsWidget, 1, 0, 1, 1);
    containerLayout->setRowStretch(1, 10);
    d->container->setLayout(containerLayout);

    QVBoxLayout* const vbx             = new QVBoxLayout(this);
    vbx->addWidget(d->container);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    setMinimumWidth(300);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

RuleDialog::~RuleDialog()
{
    delete d;
}

void RuleDialog::setDialogTitle(const QString& title)
{
    // remove ellipsis and "&&" from the string

    QString _title = title;
    _title.remove(QRegularExpression(QLatin1String("\\.{3,}")))
                  .replace(QLatin1String("&&"),
                  QLatin1String("&"));

    d->dialogTitle->setText(QString::fromUtf8("<b>%1</b>").arg(_title));
    setWindowTitle(_title);
}

void RuleDialog::setDialogDescription(const QString& description)
{
    d->dialogDescription->setText(description);
}

void RuleDialog::setDialogIcon(const QPixmap& icon)
{
    d->dialogIcon->setPixmap(icon);
}

void RuleDialog::setSettingsWidget(QWidget* const settingsWidget)
{
    delete d->settingsWidget;

    d->settingsWidget    = new QWidget(this);
    QGridLayout* const l = new QGridLayout(this);
    l->addWidget(settingsWidget);
    l->setSpacing(0);
    l->setContentsMargins(QMargins());
    d->settingsWidget->setLayout(l);
    d->container->layout()->addWidget(d->settingsWidget);
}

void RuleDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("main_window"),
                            QLatin1String("image_view"),
                            QLatin1String("renaming-photograph"));
}

} // namespace Digikam

#include "moc_ruledialog.cpp"
