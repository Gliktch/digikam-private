/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-08
 * Description : a widget to edit Application2 ObjectAttribute
 *               Iptc tag.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "objectattributesedit.h"

// Qt includes

#include <QCheckBox>
#include <QToolTip>
#include <QPushButton>
#include <QGridLayout>
#include <QComboBox>
#include <QApplication>
#include <QListWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "metadatacheckbox.h"
#include "squeezedcombobox.h"
#include "dtextedit.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN ObjectAttributesEdit::Private
{
public:

    Private() = default;

public:

    QStringList       oldValues;

    QPushButton*      addValueButton    = nullptr;
    QPushButton*      delValueButton    = nullptr;
    QPushButton*      repValueButton    = nullptr;

    DPlainTextEdit*   valueEdit         = nullptr;

    QListWidget*      valueBox          = nullptr;

    MetadataCheckBox* valueCheck        = nullptr;

    SqueezedComboBox* dataList          = nullptr;
};

ObjectAttributesEdit::ObjectAttributesEdit(QWidget* const parent, int size)
    : QWidget(parent),
      d      (new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->valueCheck     = new MetadataCheckBox(i18n("Attribute:"), this);

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

    // --------------------------------------------------------

    d->dataList       = new SqueezedComboBox(this);
    d->dataList->model()->sort(0);
    d->dataList->setWhatsThis(i18n("Select here the editorial attribute of content."));
    d->dataList->addSqueezedItem(QLatin1String("001 - ") + i18nc("Content type", "Current"));
    d->dataList->addSqueezedItem(QLatin1String("002 - ") + i18nc("Content type", "Analysis"));
    d->dataList->addSqueezedItem(QLatin1String("003 - ") + i18nc("Content type", "Archive material"));
    d->dataList->addSqueezedItem(QLatin1String("004 - ") + i18nc("Content type", "Background"));
    d->dataList->addSqueezedItem(QLatin1String("005 - ") + i18nc("Content type", "Feature"));
    d->dataList->addSqueezedItem(QLatin1String("006 - ") + i18nc("Content type", "Forecast"));
    d->dataList->addSqueezedItem(QLatin1String("007 - ") + i18nc("Content type", "History"));
    d->dataList->addSqueezedItem(QLatin1String("008 - ") + i18nc("Content type", "Obituary"));
    d->dataList->addSqueezedItem(QLatin1String("009 - ") + i18nc("Content type", "Opinion"));
    d->dataList->addSqueezedItem(QLatin1String("010 - ") + i18nc("Content type", "Polls and Surveys"));
    d->dataList->addSqueezedItem(QLatin1String("011 - ") + i18nc("Content type", "Profile"));
    d->dataList->addSqueezedItem(QLatin1String("012 - ") + i18nc("Content type", "Results Listings and Table"));
    d->dataList->addSqueezedItem(QLatin1String("013 - ") + i18nc("Content type", "Side bar and Supporting information"));
    d->dataList->addSqueezedItem(QLatin1String("014 - ") + i18nc("Content type", "Summary"));
    d->dataList->addSqueezedItem(QLatin1String("015 - ") + i18nc("Content type", "Transcript & Verbatim"));
    d->dataList->addSqueezedItem(QLatin1String("016 - ") + i18nc("Content type", "Interview"));
    d->dataList->addSqueezedItem(QLatin1String("017 - ") + i18nc("Content type", "From the Scene"));
    d->dataList->addSqueezedItem(QLatin1String("018 - ") + i18nc("Content type", "Retrospective"));
    d->dataList->addSqueezedItem(QLatin1String("019 - ") + i18nc("Content type", "Statistics"));
    d->dataList->addSqueezedItem(QLatin1String("020 - ") + i18nc("Content type", "Update"));
    d->dataList->addSqueezedItem(QLatin1String("021 - ") + i18nc("Content type", "Wrap-up"));
    d->dataList->addSqueezedItem(QLatin1String("022 - ") + i18nc("Content type", "Press Release"));

    // --------------------------------------------------------

    d->valueEdit = new DPlainTextEdit(this);
    d->valueEdit->setLinesVisible(1);
    QString whatsThis = i18n("Set here the editorial attribute description of "
                             "content.");

    if (size != -1)
    {
        whatsThis.append(i18n(" This field is limited to "));
        d->valueEdit->setMaxLength(size);
        whatsThis.append(i18np("%1 character.","%1 characters.", size));

        connect(d->valueEdit, SIGNAL(textChanged()),
                this, SLOT(slotLineEditModified()));
    }

    d->valueEdit->setWhatsThis(whatsThis);
    d->valueEdit->setPlaceholderText(i18n("Set here the editorial description"));

    // --------------------------------------------------------

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->valueCheck,     0, 0, 1, 1);
    grid->addWidget(d->addValueButton, 0, 1, 1, 1);
    grid->addWidget(d->delValueButton, 0, 2, 1, 1);
    grid->addWidget(d->repValueButton, 0, 3, 1, 1);
    grid->addWidget(d->valueBox,       0, 4, 4, 1);
    grid->addWidget(d->dataList,       1, 0, 1, 4);
    grid->addWidget(d->valueEdit,      2, 0, 1, 4);
    grid->setRowStretch(3, 10);
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
            d->dataList, SLOT(setEnabled(bool)));

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

ObjectAttributesEdit::~ObjectAttributesEdit()
{
    delete d;
}

void ObjectAttributesEdit::slotDeleteValue()
{
    QListWidgetItem* const item = d->valueBox->currentItem();

    if (!item)
    {
        return;
    }

    d->valueBox->takeItem(d->valueBox->row(item));
    delete item;
}

void ObjectAttributesEdit::slotReplaceValue()
{
    QString newValue = d->dataList->itemHighlighted().left(3);
    newValue.append(QString::fromUtf8(":%1").arg(d->valueEdit->text()));

    if (!d->valueBox->selectedItems().isEmpty())
    {
        d->valueBox->selectedItems()[0]->setText(newValue);
    }
}

void ObjectAttributesEdit::slotSelectionChanged()
{
    if (!d->valueBox->selectedItems().isEmpty())
    {
        bool ok   = false;
        int index = d->valueBox->selectedItems()[0]->text().section(QLatin1Char(':'), 0, 0).toInt(&ok);

        if (ok)
        {
            d->dataList->setCurrentIndex(index-1);
            d->valueEdit->setText(d->valueBox->selectedItems()[0]->text().section(QLatin1Char(':'), -1));
            d->delValueButton->setEnabled(true);
            d->repValueButton->setEnabled(true);
            return;
        }
    }

    d->delValueButton->setEnabled(false);
    d->repValueButton->setEnabled(false);
}

void ObjectAttributesEdit::slotAddValue()
{
    QString newValue = d->dataList->itemHighlighted().left(3);
    newValue.append(QLatin1Char(':'));

    if (!d->valueEdit->text().isEmpty())
    {
        newValue.append(d->valueEdit->text());
    }
    else
    {
        newValue.append(d->dataList->itemHighlighted()
                        .section(QLatin1String(" - "), -1));
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
    }
}

void ObjectAttributesEdit::setValues(const QStringList& values)
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

    d->dataList->setEnabled(d->valueCheck->isChecked());
    d->valueBox->setEnabled(d->valueCheck->isChecked());
    d->addValueButton->setEnabled(d->valueCheck->isChecked());
    d->delValueButton->setEnabled(d->valueCheck->isChecked());

    blockSignals(false);
}

bool ObjectAttributesEdit::getValues(QStringList& oldValues, QStringList& newValues)
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

void ObjectAttributesEdit::setValid(bool v)
{
    d->valueCheck->setValid(v);
}

bool ObjectAttributesEdit::isValid() const
{
    return d->valueCheck->isValid();
}

void ObjectAttributesEdit::slotLineEditModified()
{
    QToolTip::showText(d->valueEdit->mapToGlobal(QPoint(0, (-1)*(d->valueEdit->height() + 16))),
                       i18np("%1 character left", "%1 characters left",
                       d->valueEdit->maxLength() - d->valueEdit->text().size()),
                       d->valueEdit);
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_objectattributesedit.cpp"
