/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : EXIF caption settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exifcaption.h"

// Qt includes

#include <QIcon>
#include <QStyle>
#include <QCheckBox>
#include <QGridLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN EXIFCaption::Private
{
public:

    Private() = default;

    QCheckBox*            documentNameCheck     = nullptr;
    QCheckBox*            imageDescCheck        = nullptr;
    QCheckBox*            artistCheck           = nullptr;
    QCheckBox*            copyrightCheck        = nullptr;
    QCheckBox*            userCommentCheck      = nullptr;
    QCheckBox*            syncJFIFCommentCheck  = nullptr;
    QCheckBox*            syncXMPCaptionCheck   = nullptr;
    QCheckBox*            syncIPTCCaptionCheck  = nullptr;

    DPlainTextEdit*       userCommentEdit       = nullptr;

    DTextEdit*            documentNameEdit      = nullptr;
    DTextEdit*            imageDescEdit         = nullptr;
    DTextEdit*            artistEdit            = nullptr;
    DTextEdit*            copyrightEdit         = nullptr;

    QLabel*               documentNameIcon      = nullptr;
    QLabel*               imageDescIcon         = nullptr;
    QLabel*               artistIcon            = nullptr;
    QLabel*               copyrightIcon         = nullptr;
};

EXIFCaption::EXIFCaption(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    QString asciiPrint;     // List of printable ASCII characters.

    for (int i = 32 ; i <= 127 ; ++i)
    {
        asciiPrint.append(QChar(i));
    }

    // --------------------------------------------------------

    d->documentNameCheck = new QCheckBox(i18nc("name of the document this image has been scanned from", "Name (*):"), this);
    d->documentNameEdit  = new DTextEdit(this);
    d->documentNameEdit->setAcceptedCharacters(asciiPrint);
    d->documentNameEdit->setPlaceholderText(i18n("Set here the original document name."));
    d->documentNameEdit->setWhatsThis(i18n("Enter the name of the document from which "
                                           "this image was been scanned. This field is limited "
                                           "to ASCII characters."));

    // --------------------------------------------------------

    d->imageDescCheck = new QCheckBox(i18nc("image description", "Description (*):"), this);
    d->imageDescEdit  = new DTextEdit(this);
    d->imageDescEdit->setAcceptedCharacters(asciiPrint);
    d->imageDescEdit->setPlaceholderText(i18n("Set here the image description."));
    d->imageDescEdit->setWhatsThis(i18n("Enter the image description. This field is limited "
                                        "to ASCII characters."));

    // --------------------------------------------------------

    d->artistCheck = new QCheckBox(i18n("Artist (*):"), this);
    d->artistEdit  = new DTextEdit(this);
    d->artistEdit->setAcceptedCharacters(asciiPrint);
    d->artistEdit->setPlaceholderText(i18n("Set here the author's name."));
    d->artistEdit->setWhatsThis(i18n("Enter the image author's name separated by semi-colons. "
                                     "This field is limited to ASCII characters."));

    // --------------------------------------------------------

    d->copyrightCheck = new QCheckBox(i18n("Copyright (*):"), this);
    d->copyrightEdit  = new DTextEdit(this);
    d->copyrightEdit->setAcceptedCharacters(asciiPrint);
    d->copyrightEdit->setPlaceholderText(i18n("Set here the copyright owner."));
    d->copyrightEdit->setWhatsThis(i18n("Enter the copyright owner of the image. "
                                        "This field is limited to ASCII characters."));

    // --------------------------------------------------------

    d->userCommentCheck = new QCheckBox(i18nc("image caption", "Caption:"), this);
    d->userCommentEdit  = new DPlainTextEdit(this);
    d->userCommentEdit->setLinesVisible(4);
    d->userCommentEdit->setPlaceholderText(i18n("Set here the image's caption."));
    d->userCommentEdit->setWhatsThis(i18n("Enter the image's caption. "
                                          "This field is not limited. UTF8 encoding "
                                          "will be used to save the text."));

    d->syncJFIFCommentCheck = new QCheckBox(i18n("Sync JFIF Comment section"), this);
    d->syncXMPCaptionCheck  = new QCheckBox(i18n("Sync XMP caption"), this);
    d->syncIPTCCaptionCheck = new QCheckBox(i18n("Sync IPTC caption (warning: limited to 2000 characters)"), this);

    if (!DMetadata::supportXmp())
        d->syncXMPCaptionCheck->setEnabled(false);

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note:</b> "
                 "<b><a href='https://en.wikipedia.org/wiki/EXIF'>Exif</a></b> "
                 "text tags marked by (*) only support printable "
                 "<b><a href='https://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                 "characters."), this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->addWidget(d->documentNameCheck,       0, 0, 1, 1);
    grid->addWidget(d->documentNameEdit,        1, 0, 1, 1);
    grid->addWidget(d->imageDescCheck,          2, 0, 1, 1);
    grid->addWidget(d->imageDescEdit,           3, 0, 1, 1);
    grid->addWidget(d->artistCheck,             4, 0, 1, 1);
    grid->addWidget(d->artistEdit,              5, 0, 1, 1);
    grid->addWidget(d->copyrightCheck,          6, 0, 1, 1);
    grid->addWidget(d->copyrightEdit,           7, 0, 1, 1);
    grid->addWidget(d->userCommentCheck,        8, 0, 1, 1);
    grid->addWidget(d->userCommentEdit,         9, 0, 1, 1);
    grid->addWidget(d->syncJFIFCommentCheck,   10, 0, 1, 1);
    grid->addWidget(d->syncXMPCaptionCheck,    11, 0, 1, 1);
    grid->addWidget(d->syncIPTCCaptionCheck,   12, 0, 1, 1);
    grid->addWidget(note,                      13, 0, 1, 1);
    grid->setRowStretch(14, 10);
    grid->setColumnStretch(0, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->documentNameCheck, SIGNAL(toggled(bool)),
            d->documentNameEdit, SLOT(setEnabled(bool)));

    connect(d->imageDescCheck, SIGNAL(toggled(bool)),
            d->imageDescEdit, SLOT(setEnabled(bool)));

    connect(d->artistCheck, SIGNAL(toggled(bool)),
            d->artistEdit, SLOT(setEnabled(bool)));

    connect(d->copyrightCheck, SIGNAL(toggled(bool)),
            d->copyrightEdit, SLOT(setEnabled(bool)));

    connect(d->userCommentCheck, SIGNAL(toggled(bool)),
            d->userCommentEdit, SLOT(setEnabled(bool)));

    connect(d->userCommentCheck, SIGNAL(toggled(bool)),
            d->syncJFIFCommentCheck, SLOT(setEnabled(bool)));

    connect(d->userCommentCheck, SIGNAL(toggled(bool)),
            d->syncXMPCaptionCheck, SLOT(setEnabled(bool)));

    connect(d->userCommentCheck, SIGNAL(toggled(bool)),
            d->syncIPTCCaptionCheck, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->documentNameCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->imageDescCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->artistCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->copyrightCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->userCommentCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->userCommentEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->documentNameEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->imageDescEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->artistEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->copyrightEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

EXIFCaption::~EXIFCaption()
{
    delete d;
}

bool EXIFCaption::syncJFIFCommentIsChecked() const
{
    return d->syncJFIFCommentCheck->isChecked();
}

bool EXIFCaption::syncXMPCaptionIsChecked() const
{
    return d->syncXMPCaptionCheck->isChecked();
}

bool EXIFCaption::syncIPTCCaptionIsChecked() const
{
    return d->syncIPTCCaptionCheck->isChecked();
}

QString EXIFCaption::getEXIFUserComments() const
{
    return d->userCommentEdit->toPlainText();
}

void EXIFCaption::setCheckedSyncJFIFComment(bool c)
{
    d->syncJFIFCommentCheck->setChecked(c);
}

void EXIFCaption::setCheckedSyncXMPCaption(bool c)
{
    d->syncXMPCaptionCheck->setChecked(c);
}

void EXIFCaption::setCheckedSyncIPTCCaption(bool c)
{
    d->syncIPTCCaptionCheck->setChecked(c);
}

void EXIFCaption::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString data;

    d->documentNameEdit->clear();
    d->documentNameCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Image.DocumentName", false);

    if (!data.isNull())
    {
        d->documentNameEdit->setText(data);
        d->documentNameCheck->setChecked(true);
    }

    d->documentNameEdit->setEnabled(d->documentNameCheck->isChecked());

    d->imageDescEdit->clear();
    d->imageDescCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Image.ImageDescription", false);

    if (!data.isNull())
    {
        d->imageDescEdit->setText(data);
        d->imageDescCheck->setChecked(true);
    }

    d->imageDescEdit->setEnabled(d->imageDescCheck->isChecked());

    d->artistEdit->clear();
    d->artistCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Image.Artist", false);

    if (!data.isNull())
    {
        d->artistEdit->setText(data);
        d->artistCheck->setChecked(true);
    }

    d->artistEdit->setEnabled(d->artistCheck->isChecked());

    d->copyrightEdit->clear();
    d->copyrightCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Image.Copyright", false);

    if (!data.isNull())
    {
        d->copyrightEdit->setText(data);
        d->copyrightCheck->setChecked(true);
    }

    d->copyrightEdit->setEnabled(d->copyrightCheck->isChecked());

    d->userCommentEdit->clear();
    d->userCommentCheck->setChecked(false);
    data = meta.getExifComment(false);

    if (!data.isNull())
    {
        d->userCommentEdit->setPlainText(data);
        d->userCommentCheck->setChecked(true);
    }

    d->userCommentEdit->setEnabled(d->userCommentCheck->isChecked());
    d->syncJFIFCommentCheck->setEnabled(d->userCommentCheck->isChecked());
    d->syncXMPCaptionCheck->setEnabled(d->userCommentCheck->isChecked());
    d->syncIPTCCaptionCheck->setEnabled(d->userCommentCheck->isChecked());

    blockSignals(false);
}

void EXIFCaption::applyMetadata(const DMetadata& meta)
{
    if (d->documentNameCheck->isChecked())
    {
        meta.setExifTagString("Exif.Image.DocumentName", d->documentNameEdit->text());
    }
    else
    {
        meta.removeExifTag("Exif.Image.DocumentName");
    }

    if (d->imageDescCheck->isChecked())
    {
        meta.setExifTagString("Exif.Image.ImageDescription", d->imageDescEdit->text());
    }
    else
    {
        meta.removeExifTag("Exif.Image.ImageDescription");
    }

    if (d->artistCheck->isChecked())
    {
        meta.setExifTagString("Exif.Image.Artist", d->artistEdit->text());
    }
    else
    {
        meta.removeExifTag("Exif.Image.Artist");
    }

    if (d->copyrightCheck->isChecked())
    {
        meta.setExifTagString("Exif.Image.Copyright", d->copyrightEdit->text());
    }
    else
    {
        meta.removeExifTag("Exif.Image.Copyright");
    }

    if (d->userCommentCheck->isChecked())
    {
        meta.setExifComment(d->userCommentEdit->toPlainText(), false);

        if (syncJFIFCommentIsChecked())
        {
            meta.setComments(d->userCommentEdit->toPlainText().toUtf8());
        }

        if (meta.supportXmp() && syncXMPCaptionIsChecked())
        {
            meta.setXmpTagStringLangAlt("Xmp.dc.description",
                                        d->userCommentEdit->toPlainText(),
                                        QString());

            meta.setXmpTagStringLangAlt("Xmp.exif.UserComment",
                                        d->userCommentEdit->toPlainText(),
                                        QString());
        }

        if (syncIPTCCaptionIsChecked())
        {
            meta.setIptcTagString("Iptc.Application2.Caption", d->userCommentEdit->toPlainText());
        }
    }
    else
    {
        meta.removeExifTag("Exif.Photo.UserComment");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_exifcaption.cpp"
