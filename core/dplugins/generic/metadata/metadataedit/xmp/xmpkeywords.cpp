/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-16
 * Description : XMP keywords settings page.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "xmpkeywords.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QListWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN XMPKeywords::Private
{
public:

    Private() = default;

    QStringList  oldKeywords;

    QPushButton* addKeywordButton   = nullptr;
    QPushButton* delKeywordButton   = nullptr;
    QPushButton* repKeywordButton   = nullptr;

    QCheckBox*   keywordsCheck      = nullptr;

    DTextEdit*   keywordEdit        = nullptr;

    QListWidget* keywordsBox        = nullptr;
};

XMPKeywords::XMPKeywords(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->keywordsCheck = new QCheckBox(i18n("Use information retrieval words:"), this);

    d->keywordEdit   = new DTextEdit(this);
    d->keywordEdit->setLinesVisible(1);
    d->keywordEdit->setPlaceholderText(i18n("Set here a new keyword"));

    d->keywordsBox   = new QListWidget(this);
    d->keywordsBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addKeywordButton = new QPushButton( i18n("&Add"), this);
    d->delKeywordButton = new QPushButton( i18n("&Delete"), this);
    d->repKeywordButton = new QPushButton( i18n("&Replace"), this);
    d->addKeywordButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delKeywordButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->repKeywordButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->delKeywordButton->setEnabled(false);
    d->repKeywordButton->setEnabled(false);

    // --------------------------------------------------------

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->keywordsCheck,       0, 0, 1, 2);
    grid->addWidget(d->keywordEdit,         1, 0, 1, 1);
    grid->addWidget(d->keywordsBox,         2, 0, 5, 1);
    grid->addWidget(d->addKeywordButton,    2, 1, 1, 1);
    grid->addWidget(d->delKeywordButton,    3, 1, 1, 1);
    grid->addWidget(d->repKeywordButton,    4, 1, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(5, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->keywordsBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotKeywordSelectionChanged()));

    connect(d->addKeywordButton, SIGNAL(clicked()),
            this, SLOT(slotAddKeyword()));

    connect(d->delKeywordButton, SIGNAL(clicked()),
            this, SLOT(slotDelKeyword()));

    connect(d->repKeywordButton, SIGNAL(clicked()),
            this, SLOT(slotRepKeyword()));

    // --------------------------------------------------------

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->keywordEdit, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->addKeywordButton, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->delKeywordButton, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->repKeywordButton, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->keywordsBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->addKeywordButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delKeywordButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->repKeywordButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));
}

XMPKeywords::~XMPKeywords()
{
    delete d;
}

void XMPKeywords::slotDelKeyword()
{
    QListWidgetItem* const item = d->keywordsBox->currentItem();

    if (!item)
    {
        return;
    }

    d->keywordsBox->takeItem(d->keywordsBox->row(item));
    delete item;
}

void XMPKeywords::slotRepKeyword()
{
    QString newKeyword = d->keywordEdit->text();

    if (newKeyword.isEmpty())
    {
        return;
    }

    if (!d->keywordsBox->selectedItems().isEmpty())
    {
        d->keywordsBox->selectedItems()[0]->setText(newKeyword);
        d->keywordEdit->clear();
    }
}

void XMPKeywords::slotKeywordSelectionChanged()
{
    if (!d->keywordsBox->selectedItems().isEmpty())
    {
        d->keywordEdit->setText(d->keywordsBox->selectedItems()[0]->text());
        d->delKeywordButton->setEnabled(true);
        d->repKeywordButton->setEnabled(true);
    }
    else
    {
        d->delKeywordButton->setEnabled(false);
        d->repKeywordButton->setEnabled(false);
    }
}

void XMPKeywords::slotAddKeyword()
{
    QString newKeyword = d->keywordEdit->text();

    if (newKeyword.isEmpty())
    {
        return;
    }

    bool found = false;

    for (int i = 0 ; i < d->keywordsBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->keywordsBox->item(i);

        if (newKeyword == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->keywordsBox->insertItem(d->keywordsBox->count(), newKeyword);
        d->keywordEdit->clear();
    }
}

void XMPKeywords::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    d->oldKeywords = meta.getXmpKeywords();

    d->keywordsBox->clear();
    d->keywordsCheck->setChecked(false);

    if (!d->oldKeywords.isEmpty())
    {
        d->keywordsBox->insertItems(0, d->oldKeywords);
        d->keywordsCheck->setChecked(true);
    }

    d->keywordEdit->setEnabled(d->keywordsCheck->isChecked());
    d->keywordsBox->setEnabled(d->keywordsCheck->isChecked());
    d->addKeywordButton->setEnabled(d->keywordsCheck->isChecked());
    d->delKeywordButton->setEnabled(d->keywordsCheck->isChecked());

    blockSignals(false);
}

void XMPKeywords::applyMetadata(const DMetadata& meta)
{
    QStringList newKeywords;

    for (int i = 0 ; i < d->keywordsBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->keywordsBox->item(i);
        newKeywords.append(item->text());
    }

    // We remove in first all existing keywords.

    meta.removeXmpTag("Xmp.dc.subject");

    // And add new list if necessary.

    if (d->keywordsCheck->isChecked())
    {
        meta.setXmpKeywords(newKeywords);
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_xmpkeywords.cpp"
