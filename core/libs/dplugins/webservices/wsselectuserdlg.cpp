/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-16-05
 * Description : a dialog to select user for Web Service tools
 *
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wsselectuserdlg.h"

// Qt includes

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QIcon>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

class Q_DECL_HIDDEN WSSelectUserDlg::Private
{
public:

    Private() = default;

    QComboBox*   userComboBox   = nullptr;
    QLabel*      label          = nullptr;
    QPushButton* okButton       = nullptr;
    QString      userName;
    QString      serviceName;
};

WSSelectUserDlg::WSSelectUserDlg(QWidget* const parent, const QString& serviceName)
    : QDialog(parent),
      d      (new Private)
{
    d->serviceName = serviceName;

    setWindowTitle(i18nc("@title:window", "Account Selector"));
    setModal(true);

    QDialogButtonBox* const buttonBox   = new QDialogButtonBox();
    QPushButton* const buttonNewAccount = new QPushButton(buttonBox);
    buttonNewAccount->setText(i18n("Add another account"));
    buttonNewAccount->setIcon(QIcon::fromTheme(QLatin1String("network-workgroup")));

    buttonBox->addButton(buttonNewAccount, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Close);

    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    d->okButton = buttonBox->button(QDialogButtonBox::Ok);

    if (d->serviceName == QLatin1String("23"))
    {
        setWindowIcon(QIcon::fromTheme(QLatin1String("hq")));
    }
    else
    {
        setWindowIcon(QIcon::fromTheme(QLatin1String("dk-flickr")));
    }

    d->userName     = QString();
    d->label        = new QLabel(this);
    d->label->setText(i18n("Choose the %1 account to use for exporting images:", d->serviceName));
    d->userComboBox = new QComboBox(this);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(d->label);
    mainLayout->addWidget(d->userComboBox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));

    connect(buttonNewAccount, SIGNAL(clicked()),
            this, SLOT(slotNewAccountClicked()));
}

WSSelectUserDlg::~WSSelectUserDlg()
{
    delete d->userComboBox;
    delete d->label;
    delete d;
}

void WSSelectUserDlg::reactivate()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();

    d->userComboBox->clear();
    const auto glist = config->groupList();

    for (const QString& group : glist)
    {
        if (!(group.contains(d->serviceName)))
        {
            continue;
        }

        KConfigGroup grp = config->group(group);

        if (QString::compare(grp.readEntry(QLatin1String("username")), QString(), Qt::CaseInsensitive) == 0)
        {
            continue;
        }

        d->userComboBox->addItem(grp.readEntry(QLatin1String("username")));
    }

    d->okButton->setEnabled(d->userComboBox->count() > 0);

    exec();
}

void WSSelectUserDlg::slotOkClicked()
{
    d->userName = d->userComboBox->currentText();
}

void WSSelectUserDlg::slotNewAccountClicked()
{
    d->userName = QString();
}

QString WSSelectUserDlg::getUserName() const
{
    return d->userName;
}

} // namespace Digikam

#include "moc_wsselectuserdlg.cpp"
