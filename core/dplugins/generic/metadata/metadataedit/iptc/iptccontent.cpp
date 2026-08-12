/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : IPTC content settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iptccontent.h"

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
#include "dlayoutbox.h"
#include "multistringsedit.h"
#include "dexpanderbox.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN IPTCContent::Private
{
public:

    Private() = default;

    QCheckBox*        captionCheck          = nullptr;
    QCheckBox*        headlineCheck         = nullptr;
    QCheckBox*        syncJFIFCommentCheck  = nullptr;
    QCheckBox*        syncEXIFCommentCheck  = nullptr;

    QLabel*           captionNote           = nullptr;
    DPlainTextEdit*   captionEdit           = nullptr;

    DPlainTextEdit*   headlineEdit          = nullptr;

    MultiStringsEdit* writerEdit            = nullptr;
};

IPTCContent::IPTCContent(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->headlineCheck = new QCheckBox(i18n("Headline:"), this);
    d->headlineEdit  = new DPlainTextEdit(this);
    d->headlineEdit->setMaxLength(256);
    d->headlineEdit->setPlaceholderText(i18n("Set here the content synopsis"));
    d->headlineEdit->setWhatsThis(i18n("Enter here the content synopsis. This field is limited "
                                       "to 256 characters."));

    // --------------------------------------------------------

    DHBox* const captionHeader = new DHBox(this);
    d->captionCheck            = new QCheckBox(i18nc("content description", "Caption:"), captionHeader);
    d->captionNote             = new QLabel(captionHeader);
    captionHeader->setStretchFactor(d->captionCheck, 10);

    d->captionEdit             = new DPlainTextEdit(this);
    d->captionEdit->setLinesVisible(4);
    d->syncJFIFCommentCheck    = new QCheckBox(i18n("Sync JFIF Comment section"), this);
    d->syncEXIFCommentCheck    = new QCheckBox(i18n("Sync Exif Comment"), this);
    d->captionEdit->setMaxLength(2000);
    d->captionEdit->setPlaceholderText(i18n("Set here the content description"));
    d->captionEdit->setWhatsThis(i18n("Enter the content description. This field is limited "
                                      "to 2000 characters."));

    // --------------------------------------------------------

    d->writerEdit  = new MultiStringsEdit(this, i18n("Caption Writer:"),
                                          i18n("Enter the name of the caption author."),
                                          32);

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

    grid->addWidget(d->headlineCheck,                       0, 0, 1, 3);
    grid->addWidget(d->headlineEdit,                        1, 0, 1, 3);
    grid->addWidget(captionHeader,                          2, 0, 1, 3);
    grid->addWidget(d->captionEdit,                         3, 0, 1, 3);
    grid->addWidget(d->syncJFIFCommentCheck,                4, 0, 1, 3);
    grid->addWidget(d->syncEXIFCommentCheck,                6, 0, 1, 3);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  7, 0, 1, 3);
    grid->addWidget(d->writerEdit,                          8, 0, 1, 3);
    grid->addWidget(note,                                   9, 0, 1, 3);
    grid->setRowStretch(10, 10);
    grid->setColumnStretch(2, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            d->captionEdit, SLOT(setEnabled(bool)));

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            d->syncJFIFCommentCheck, SLOT(setEnabled(bool)));

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            d->syncEXIFCommentCheck, SLOT(setEnabled(bool)));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            d->headlineEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->writerEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->headlineEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

IPTCContent::~IPTCContent()
{
    delete d;
}

bool IPTCContent::syncJFIFCommentIsChecked() const
{
    return d->syncJFIFCommentCheck->isChecked();
}

bool IPTCContent::syncEXIFCommentIsChecked() const
{
    return d->syncEXIFCommentCheck->isChecked();
}

QString IPTCContent::getIPTCCaption() const
{
    return d->captionEdit->toPlainText();
}

void IPTCContent::setCheckedSyncJFIFComment(bool c)
{
    d->syncJFIFCommentCheck->setChecked(c);
}

void IPTCContent::setCheckedSyncEXIFComment(bool c)
{
    d->syncEXIFCommentCheck->setChecked(c);
}

void IPTCContent::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString     data;
    QStringList list;

    d->captionEdit->clear();
    d->captionCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Caption", false);

    if (!data.isNull())
    {
        d->captionEdit->setPlainText(data);
        d->captionCheck->setChecked(true);
    }

    d->captionEdit->setEnabled(d->captionCheck->isChecked());
    d->syncJFIFCommentCheck->setEnabled(d->captionCheck->isChecked());
    d->syncEXIFCommentCheck->setEnabled(d->captionCheck->isChecked());

    list = meta.getIptcTagsStringList("Iptc.Application2.Writer", false);
    d->writerEdit->setValues(list);

    d->headlineEdit->clear();
    d->headlineCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Headline", false);

    if (!data.isNull())
    {
        d->headlineEdit->setPlainText(data);
        d->headlineCheck->setChecked(true);
    }

    d->headlineEdit->setEnabled(d->headlineCheck->isChecked());

    blockSignals(false);
}

void IPTCContent::applyMetadata(const DMetadata& meta)
{
    if (d->captionCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Caption", d->captionEdit->toPlainText());

        if (syncEXIFCommentIsChecked())
        {
            meta.setExifComment(getIPTCCaption());
        }

        if (syncJFIFCommentIsChecked())
        {
            meta.setComments(getIPTCCaption().toUtf8());
        }
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Caption");
    }

    QStringList oldList, newList;

    if (d->writerEdit->getValues(oldList, newList))
    {
        meta.setIptcTagsStringList("Iptc.Application2.Writer", 32, oldList, newList);
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Writer");
    }

    if (d->headlineCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Headline", d->headlineEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.Headline");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_iptccontent.cpp"
