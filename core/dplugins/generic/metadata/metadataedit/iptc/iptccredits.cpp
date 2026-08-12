/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : IPTC credits settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iptccredits.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QToolTip>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "multistringsedit.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN IPTCCredits::Private
{
public:

    Private() = default;

    QCheckBox*        copyrightCheck    = nullptr;
    QCheckBox*        creditCheck       = nullptr;
    QCheckBox*        sourceCheck       = nullptr;

    DPlainTextEdit*  copyrightEdit      = nullptr;
    DPlainTextEdit*  creditEdit         = nullptr;
    DPlainTextEdit*  sourceEdit         = nullptr;

    MultiStringsEdit* bylineEdit        = nullptr;
    MultiStringsEdit* bylineTitleEdit   = nullptr;
    MultiStringsEdit* contactEdit       = nullptr;
};

IPTCCredits::IPTCCredits(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->copyrightCheck = new QCheckBox(i18n("Copyright:"), this);
    d->copyrightEdit  = new DPlainTextEdit(this);
    d->copyrightEdit->setMaxLength(128);
    d->copyrightEdit->setPlaceholderText(i18n("Set here the copyright notice."));
    d->copyrightEdit->setWhatsThis(i18n("Set here the necessary copyright notice. This field is limited "
                                        "to 128 characters."));

    // --------------------------------------------------------

    d->bylineEdit  = new MultiStringsEdit(this, i18n("Byline:"),
                                          i18n("Set here the name of content creator."),
                                          32);

    // --------------------------------------------------------

    d->bylineTitleEdit  = new MultiStringsEdit(this, i18n("Byline Title:"),
                                               i18n("Set here the title of content creator."),
                                               32);

    // --------------------------------------------------------

    d->creditCheck = new QCheckBox(i18n("Credit:"), this);
    d->creditEdit  = new DPlainTextEdit(this);
    d->creditEdit->setMaxLength(32);
    d->creditEdit->setPlaceholderText(i18n("Set here the content provider."));
    d->creditEdit->setWhatsThis(i18n("Set here the content provider. "
                                     "This field is limited to 32 characters."));

    // --------------------------------------------------------

    d->sourceCheck = new QCheckBox(i18nc("original owner of content", "Source:"), this);
    d->sourceEdit  = new DPlainTextEdit(this);
    d->sourceEdit->setMaxLength(32);
    d->sourceEdit->setPlaceholderText(i18n("Set here the content owner."));
    d->sourceEdit->setWhatsThis(i18n("Set here the original owner of content. "
                                     "This field is limited to 32 characters."));

    // --------------------------------------------------------

    d->contactEdit  = new MultiStringsEdit(this, i18n("Contact:"),
                                           i18n("Set here the person or organization to contact."),
                                           128);

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

    grid->addWidget(d->bylineEdit,      0, 0, 1, 3);
    grid->addWidget(d->bylineTitleEdit, 1, 0, 1, 3);
    grid->addWidget(d->contactEdit,     2, 0, 1, 3);
    grid->addWidget(d->creditCheck,     3, 0, 1, 3);
    grid->addWidget(d->creditEdit,      4, 0, 1, 3);
    grid->addWidget(d->sourceCheck,     5, 0, 1, 3);
    grid->addWidget(d->sourceEdit,      6, 0, 1, 3);
    grid->addWidget(d->copyrightCheck,  7, 0, 1, 3);
    grid->addWidget(d->copyrightEdit,   8, 0, 1, 3);
    grid->addWidget(note,               9, 0, 1, 3);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(10, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->copyrightCheck, SIGNAL(toggled(bool)),
            d->copyrightEdit, SLOT(setEnabled(bool)));

    connect(d->creditCheck, SIGNAL(toggled(bool)),
            d->creditEdit, SLOT(setEnabled(bool)));

    connect(d->sourceCheck, SIGNAL(toggled(bool)),
            d->sourceEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->copyrightCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->bylineEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->bylineTitleEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->creditCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sourceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->contactEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->copyrightEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->creditEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->sourceEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

IPTCCredits::~IPTCCredits()
{
    delete d;
}

void IPTCCredits::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString     data;
    QStringList list;

    d->copyrightEdit->clear();
    d->copyrightCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Copyright", false);

    if (!data.isNull())
    {
        d->copyrightEdit->setText(data);
        d->copyrightCheck->setChecked(true);
    }

    d->copyrightEdit->setEnabled(d->copyrightCheck->isChecked());

    list = meta.getIptcTagsStringList("Iptc.Application2.Byline", false);
    d->bylineEdit->setValues(list);

    list = meta.getIptcTagsStringList("Iptc.Application2.BylineTitle", false);
    d->bylineTitleEdit->setValues(list);

    d->creditEdit->clear();
    d->creditCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Credit", false);

    if (!data.isNull())
    {
        d->creditEdit->setText(data);
        d->creditCheck->setChecked(true);
    }

    d->creditEdit->setEnabled(d->creditCheck->isChecked());

    d->sourceEdit->clear();
    d->sourceCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Source", false);

    if (!data.isNull())
    {
        d->sourceEdit->setText(data);
        d->sourceCheck->setChecked(true);
    }

    d->sourceEdit->setEnabled(d->sourceCheck->isChecked());

    list = meta.getIptcTagsStringList("Iptc.Application2.Contact", false);
    d->contactEdit->setValues(list);

    blockSignals(false);
}

void IPTCCredits::applyMetadata(const DMetadata& meta)
{
    QStringList oldList, newList;

    if (d->copyrightCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Copyright", d->copyrightEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Copyright");
    }

    if (d->bylineEdit->getValues(oldList, newList))
    {
        meta.setIptcTagsStringList("Iptc.Application2.Byline", 32, oldList, newList);
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Byline");
    }

    if (d->bylineTitleEdit->getValues(oldList, newList))
    {
        meta.setIptcTagsStringList("Iptc.Application2.BylineTitle", 32, oldList, newList);
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.BylineTitle");
    }

    if (d->creditCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Credit", d->creditEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Credit");
    }

    if (d->sourceCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Source", d->sourceEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Source");
    }

    if (d->contactEdit->getValues(oldList, newList))
    {
        meta.setIptcTagsStringList("Iptc.Application2.Contact", 128, oldList, newList);
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Contact");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_iptccredits.cpp"
