/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : IPTC status settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iptcstatus.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QToolTip>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dtextedit.h"
#include "dlayoutbox.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN IPTCStatus::Private
{
public:

    Private() = default;

public:

    QCheckBox*       statusCheck                = nullptr;
    QCheckBox*       jobIDCheck                 = nullptr;
    QCheckBox*       specialInstructionCheck    = nullptr;
    QCheckBox*       objectNameCheck            = nullptr;

    DPlainTextEdit* objectNameEdit              = nullptr;
    DPlainTextEdit* statusEdit                  = nullptr;
    DPlainTextEdit* jobIDEdit                   = nullptr;
    DPlainTextEdit* specialInstructionEdit      = nullptr;
};

IPTCStatus::IPTCStatus(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->objectNameCheck = new QCheckBox(i18nc("image title", "Title:"), this);
    d->objectNameEdit  = new DPlainTextEdit(this);
    d->objectNameEdit->setMaxLength(64);
    d->objectNameEdit->setPlaceholderText(i18n("Set here the content title"));
    d->objectNameEdit->setWhatsThis(i18n("Set here the shorthand reference of content. "
                                         "This field is limited to 64 characters."));

    // --------------------------------------------------------

    d->statusCheck = new QCheckBox(i18n("Edit Status:"), this);
    d->statusEdit  = new DPlainTextEdit(this);
    d->statusEdit->setMaxLength(64);
    d->statusEdit->setPlaceholderText(i18n("Set here the content status"));
    d->statusEdit->setWhatsThis(i18n("Set here the title of content status. This field is limited "
                                     "to 64 characters."));

    // --------------------------------------------------------

    d->jobIDCheck = new QCheckBox(i18n("Job Identifier:"), this);
    d->jobIDEdit  = new DPlainTextEdit(this);
    d->jobIDEdit->setMaxLength(32);
    d->jobIDEdit->setPlaceholderText(i18n("Set here the job ID"));
    d->jobIDEdit->setWhatsThis(i18n("Set here the string that identifies content that recurs. "
                                    "This field is limited to 32 characters."));

    // --------------------------------------------------------

    d->specialInstructionCheck = new QCheckBox(i18n("Special Instructions:"), this);
    d->specialInstructionEdit  = new DPlainTextEdit(this);
    d->specialInstructionEdit->setMaxLength(256);
    d->specialInstructionEdit->setPlaceholderText(i18n("Set here the content instructions"));
    d->specialInstructionEdit->setWhatsThis(i18n("Enter the editorial usage instructions. "
                                                 "This field is limited to 256 characters."));

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: "
                 "<a href='https://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a> "
                 "text tags are limited string sizes. Use contextual help for details. "
                 "Consider to use <a href='https://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> instead.</b>"),
                 this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->addWidget(d->objectNameCheck,         0, 0, 1, 3);
    grid->addWidget(d->objectNameEdit,          1, 0, 1, 3);
    grid->addWidget(d->statusCheck,             2, 0, 1, 3);
    grid->addWidget(d->statusEdit,              3, 0, 1, 3);
    grid->addWidget(d->jobIDCheck,              4, 0, 1, 3);
    grid->addWidget(d->jobIDEdit,               5, 0, 1, 3);
    grid->addWidget(d->specialInstructionCheck, 6, 0, 1, 3);
    grid->addWidget(d->specialInstructionEdit,  7, 0, 1, 3);
    grid->addWidget(note,                       9, 0, 1, 3);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(10, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->objectNameCheck, SIGNAL(toggled(bool)),
            d->objectNameEdit, SLOT(setEnabled(bool)));

    connect(d->statusCheck, SIGNAL(toggled(bool)),
            d->statusEdit, SLOT(setEnabled(bool)));

    connect(d->jobIDCheck, SIGNAL(toggled(bool)),
            d->jobIDEdit, SLOT(setEnabled(bool)));

    connect(d->specialInstructionCheck, SIGNAL(toggled(bool)),
            d->specialInstructionEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->objectNameCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->statusCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->jobIDCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->specialInstructionCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->objectNameEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->statusEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->jobIDEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->specialInstructionEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

IPTCStatus::~IPTCStatus()
{
    delete d;
}

void IPTCStatus::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString     data;

    d->objectNameEdit->clear();
    d->objectNameCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.ObjectName", false);

    if (!data.isNull())
    {
        d->objectNameEdit->setText(data);
        d->objectNameCheck->setChecked(true);
    }

    d->objectNameEdit->setEnabled(d->objectNameCheck->isChecked());

    d->statusEdit->clear();
    d->statusCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.EditStatus", false);

    if (!data.isNull())
    {
        d->statusEdit->setText(data);
        d->statusCheck->setChecked(true);
    }

    d->statusEdit->setEnabled(d->statusCheck->isChecked());

    d->jobIDEdit->clear();
    d->jobIDCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.FixtureId", false);

    if (!data.isNull())
    {
        d->jobIDEdit->setText(data);
        d->jobIDCheck->setChecked(true);
    }

    d->jobIDEdit->setEnabled(d->jobIDCheck->isChecked());

    d->specialInstructionEdit->clear();
    d->specialInstructionCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.SpecialInstructions", false);

    if (!data.isNull())
    {
        d->specialInstructionEdit->setPlainText(data);
        d->specialInstructionCheck->setChecked(true);
    }

    d->specialInstructionEdit->setEnabled(d->specialInstructionCheck->isChecked());

    blockSignals(false);
}

void IPTCStatus::applyMetadata(const DMetadata& meta)
{
    if (d->objectNameCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.ObjectName", d->objectNameEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.ObjectName");
    }

    if (d->statusCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.EditStatus", d->statusEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.EditStatus");
    }

    if (d->jobIDCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.FixtureId", d->jobIDEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.FixtureId");
    }

    if (d->specialInstructionCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.SpecialInstructions", d->specialInstructionEdit->toPlainText());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.SpecialInstructions");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_iptcstatus.cpp"
