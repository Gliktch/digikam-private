/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-08
 * Description : a widget to edit a tag with multiple string entries.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "multistringsedit.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QListWidget>
#include <QToolTip>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dtextedit.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN MultiStringsEdit::Private
{
public:

    Private() = default;

    QStringList      oldValues;

    QPushButton*     addValueButton = nullptr;
    QPushButton*     delValueButton = nullptr;
    QPushButton*     repValueButton = nullptr;

    QCheckBox*       valueCheck     = nullptr;

    DPlainTextEdit*  valueEdit      = nullptr;

    QListWidget*     valueBox       = nullptr;
};

MultiStringsEdit::MultiStringsEdit(QWidget* const parent,
                                   const QString& title,
                                   const QString& desc,
                                   int size)
    : QWidget(parent),
      d      (new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->valueCheck     = new QCheckBox(title, this);

    d->addValueButton = new QPushButton(this);
    d->delValueButton = new QPushButton(this);
    d->repValueButton = new QPushButton(this);
    d->addValueButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delValueButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->repValueButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->addValueButton->setWhatsThis(i18n("Add a new value to the list"));
    d->delValueButton->setWhatsThis(i18n("Remove the current selected value from the list"));
    d->repValueButton->setWhatsThis(i18n("Replace the current selected value from the list"));
    d->delValueButton->setEnabled(false);
    d->repValueButton->setEnabled(false);

    d->valueBox       = new QListWidget(this);
    d->valueBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    d->valueBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    d->valueEdit      = new DPlainTextEdit(this);
    d->valueEdit->setLinesVisible(1);
    QString whatsThis = desc;

    if (size != -1)
    {
        whatsThis.append(i18n(" This field is limited to:"));
        d->valueEdit->setMaxLength(size);
        whatsThis.append(i18np("<p>1 character.</p>","<p>%1 characters.</p>", size));

        connect(d->valueEdit, SIGNAL(textChanged()),
                this, SLOT(slotLineEditModified()));
    }

    d->valueEdit->setPlaceholderText(desc);
    d->valueEdit->setWhatsThis(whatsThis);

    // --------------------------------------------------------

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->valueCheck,     0, 0, 1, 1);
    grid->addWidget(d->addValueButton, 0, 1, 1, 1);
    grid->addWidget(d->delValueButton, 0, 2, 1, 1);
    grid->addWidget(d->repValueButton, 0, 3, 1, 1);
    grid->addWidget(d->valueBox,       0, 4, 3, 1);
    grid->addWidget(d->valueEdit,      2, 0, 1, 4);
    grid->setRowStretch(1, 10);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(4, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(layoutSpacing());

    // --------------------------------------------------------

    connect(d->valueBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addValueButton, SIGNAL(clicked()),
            this, SLOT(slotAddValue()));

    connect(d->delValueButton, SIGNAL(clicked()),
            this, SLOT(slotDeleteValue()));

    connect(d->repValueButton, SIGNAL(clicked()),
            this, SLOT(slotReplaceValue()));

    // --------------------------------------------------------

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->valueEdit, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->addValueButton, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->delValueButton, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->repValueButton, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->valueBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->addValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->repValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));
}

MultiStringsEdit::~MultiStringsEdit()
{
    delete d;
}

void MultiStringsEdit::slotDeleteValue()
{
    QListWidgetItem* const item = d->valueBox->currentItem();

    if (!item)
    {
        return;
    }

    d->valueBox->takeItem(d->valueBox->row(item));
    delete item;
}

void MultiStringsEdit::slotReplaceValue()
{
    QString newValue = d->valueEdit->text();

    if (newValue.isEmpty())
    {
        return;
    }

    if (!d->valueBox->selectedItems().isEmpty())
    {
        d->valueBox->selectedItems()[0]->setText(newValue);
        d->valueEdit->clear();
    }
}

void MultiStringsEdit::slotSelectionChanged()
{
    if (!d->valueBox->selectedItems().isEmpty())
    {
        d->valueEdit->setText(d->valueBox->selectedItems()[0]->text());
        d->delValueButton->setEnabled(true);
        d->repValueButton->setEnabled(true);
    }
    else
    {
        d->delValueButton->setEnabled(false);
        d->repValueButton->setEnabled(false);
    }
}

void MultiStringsEdit::slotAddValue()
{
    QString newValue = d->valueEdit->text();

    if (newValue.isEmpty())
    {
        return;
    }

    bool found = false;

    for (int i = 0 ; i < d->valueBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->valueBox->item(i);

        if (newValue == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->valueBox->insertItem(d->valueBox->count(), newValue);
        d->valueEdit->clear();
    }
}

void MultiStringsEdit::setValues(const QStringList& values)
{
    blockSignals(true);
    d->oldValues = values;

    d->valueBox->clear();
    d->valueCheck->setChecked(false);

    if (!d->oldValues.isEmpty())
    {
        d->valueBox->insertItems(0, d->oldValues);
        d->valueCheck->setChecked(true);
    }

    d->valueEdit->setEnabled(d->valueCheck->isChecked());
    d->valueBox->setEnabled(d->valueCheck->isChecked());
    d->addValueButton->setEnabled(d->valueCheck->isChecked());
    d->delValueButton->setEnabled(d->valueCheck->isChecked());

    blockSignals(false);
}

bool MultiStringsEdit::getValues(QStringList& oldValues, QStringList& newValues)
{
    oldValues = d->oldValues;
    newValues.clear();

    for (int i = 0 ; i < d->valueBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->valueBox->item(i);
        newValues.append(item->text());
    }

    return d->valueCheck->isChecked();
}

void MultiStringsEdit::slotLineEditModified()
{
    QToolTip::showText(d->valueEdit->mapToGlobal(QPoint(0, (-1)*(d->valueEdit->height() + 16))),
                       i18np("%1 character left", "%1 characters left",
                       d->valueEdit->maxLength() - d->valueEdit->text().size()),
                       d->valueEdit);
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_multistringsedit.cpp"
