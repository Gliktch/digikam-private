/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-18
 * Description : XMP content settings page.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "xmpcontent.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "altlangstringedit.h"
#include "dexpanderbox.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN XMPContent::Private
{
public:

    Private() = default;

    QCheckBox*          headlineCheck           = nullptr;
    QCheckBox*          syncJFIFCommentCheck    = nullptr;
    QCheckBox*          syncEXIFCommentCheck    = nullptr;
    QCheckBox*          syncEXIFCopyrightCheck  = nullptr;
    QCheckBox*          writerCheck             = nullptr;

    DTextEdit*          headlineEdit            = nullptr;
    DTextEdit*          writerEdit              = nullptr;

    AltLangStringsEdit* captionEdit             = nullptr;
    AltLangStringsEdit* copyrightEdit           = nullptr;
    AltLangStringsEdit* usageTermsEdit          = nullptr;
};

XMPContent::XMPContent(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    int spacing = layoutSpacing();

    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->headlineCheck = new QCheckBox(i18n("Headline:"), this);
    d->headlineEdit  = new DTextEdit(this);
    d->headlineEdit->setPlaceholderText(i18n("Set here the content synopsis."));

    // --------------------------------------------------------

    d->captionEdit          = new AltLangStringsEdit(this, i18nc("content description", "Captions:"),
                                                     i18n("Set here the content descriptions."));

    QGroupBox* const syncOptions  = new QGroupBox(i18n("Default Language Caption Options"), this);
    QVBoxLayout* const vlay       = new QVBoxLayout(syncOptions);

    d->syncJFIFCommentCheck = new QCheckBox(i18n("Sync JFIF Comment section"), syncOptions);
    d->syncEXIFCommentCheck = new QCheckBox(i18n("Sync Exif Comment"), syncOptions);

    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);
    vlay->addWidget(d->syncJFIFCommentCheck);
    vlay->addWidget(d->syncEXIFCommentCheck);

    // --------------------------------------------------------

    d->writerCheck = new QCheckBox(i18n("Caption Writer:"), this);
    d->writerEdit  = new DTextEdit(this);
    d->writerEdit->setPlaceholderText(i18n("Set here the name of the caption author."));

    // --------------------------------------------------------

    d->copyrightEdit          = new AltLangStringsEdit(this, i18n("Copyright:"),
                                                       i18n("Set here the necessary copyright notice."));

    d->syncEXIFCopyrightCheck = new QCheckBox(i18n("Sync Exif Copyright"), this);

    d->usageTermsEdit         = new AltLangStringsEdit(this, i18n("Right Usage Terms:"),
                                                       i18n("Set here the instructions on how a "
                                                            "resource can be legally used here."));

    // --------------------------------------------------------

    grid->addWidget(d->headlineCheck,                      0, 0, 1, 3);
    grid->addWidget(d->headlineEdit,                       1, 0, 1, 3);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this), 2, 0, 1, 3);
    grid->addWidget(d->captionEdit,                        3, 0, 1, 3);
    grid->addWidget(syncOptions,                           4, 0, 1, 3);
    grid->addWidget(d->writerCheck,                        5, 0, 1, 3);
    grid->addWidget(d->writerEdit,                         6, 1, 1, 3);
    grid->addWidget(d->copyrightEdit,                      7, 0, 1, 3);
    grid->addWidget(d->syncEXIFCopyrightCheck,             8, 0, 1, 3);
    grid->addWidget(d->usageTermsEdit,                     9, 0, 1, 3);
    grid->setRowStretch(10, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(signalDefaultLanguageEnabled(bool)),
            this, SLOT(slotSyncCaptionOptionsEnabled(bool)));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            d->headlineEdit, SLOT(setEnabled(bool)));

    connect(d->writerCheck, SIGNAL(toggled(bool)),
            d->writerEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->copyrightEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->usageTermsEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->copyrightEdit, SIGNAL(signalDefaultLanguageEnabled(bool)),
            this, SLOT(slotSyncCopyrightOptionsEnabled(bool)));

    connect(d->writerCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->copyrightEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->usageTermsEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->headlineEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->writerEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

XMPContent::~XMPContent()
{
    delete d;
}

bool XMPContent::syncJFIFCommentIsChecked() const
{
    return d->syncJFIFCommentCheck->isChecked();
}

bool XMPContent::syncEXIFCommentIsChecked() const
{
    return d->syncEXIFCommentCheck->isChecked();
}

bool XMPContent::syncEXIFCopyrightIsChecked() const
{
    return d->syncEXIFCopyrightCheck->isChecked();
}

QString XMPContent::getXMPCaption() const
{
    return d->captionEdit->defaultAltLang();
}

QString XMPContent::getXMPCopyright() const
{
    return d->copyrightEdit->defaultAltLang();
}

void XMPContent::setCheckedSyncJFIFComment(bool c)
{
    d->syncJFIFCommentCheck->setChecked(c);
}

void XMPContent::setCheckedSyncEXIFComment(bool c)
{
    d->syncEXIFCommentCheck->setChecked(c);
}

void XMPContent::setCheckedSyncEXIFCopyright(bool c)
{
    d->syncEXIFCopyrightCheck->setChecked(c);
}

void XMPContent::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    DMetadata::AltLangMap map;
    QString data;

    d->headlineEdit->clear();
    d->headlineCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Headline", false);

    if (!data.isNull())
    {
        d->headlineEdit->setText(data);
        d->headlineCheck->setChecked(true);
    }

    d->headlineEdit->setEnabled(d->headlineCheck->isChecked());

    d->captionEdit->setValues(map);
    d->captionEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.dc.description", false);

    if (!map.isEmpty())
    {
        d->captionEdit->setValues(map);
        d->captionEdit->setValid(true);
    }

    d->writerEdit->clear();
    d->writerCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.CaptionWriter", false);

    if (!data.isNull())
    {
        d->writerEdit->setText(data);
        d->writerCheck->setChecked(true);
    }

    d->writerEdit->setEnabled(d->writerCheck->isChecked());

    map.clear();
    d->copyrightEdit->setValues(map);
    d->copyrightEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.dc.rights", false);

    if (!map.isEmpty())
    {
        d->copyrightEdit->setValues(map);
        d->copyrightEdit->setValid(true);
    }

    map.clear();
    d->usageTermsEdit->setValues(map);
    d->usageTermsEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", false);

    if (!map.isEmpty())
    {
        d->usageTermsEdit->setValues(map);
        d->usageTermsEdit->setValid(true);
    }

    blockSignals(false);
}

void XMPContent::applyMetadata(const DMetadata& meta)
{
    if (d->headlineCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.Headline", d->headlineEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.Headline");
    }

    DMetadata::AltLangMap oldAltLangMap, newAltLangMap;

    if      (d->captionEdit->getValues(oldAltLangMap, newAltLangMap))
    {
        meta.setXmpTagStringListLangAlt("Xmp.dc.description", newAltLangMap);

        if (syncEXIFCommentIsChecked())
        {
            meta.setExifComment(getXMPCaption());
        }

        if (syncJFIFCommentIsChecked())
        {
            meta.setComments(getXMPCaption().toUtf8());
        }
    }
    else if (d->captionEdit->isValid())
    {
        meta.removeXmpTag("Xmp.dc.description");
    }

    if (d->writerCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.CaptionWriter", d->writerEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.CaptionWriter");
    }

    if      (d->copyrightEdit->getValues(oldAltLangMap, newAltLangMap))
    {
        meta.setXmpTagStringListLangAlt("Xmp.dc.rights", newAltLangMap);

        if (syncEXIFCopyrightIsChecked())
        {
            meta.removeExifTag("Exif.Image.Copyright");
            meta.setExifTagString("Exif.Image.Copyright", getXMPCopyright());
        }
    }
    else if (d->copyrightEdit->isValid())
    {
        meta.removeXmpTag("Xmp.dc.rights");
    }

    if      (d->usageTermsEdit->getValues(oldAltLangMap, newAltLangMap))
    {
        meta.setXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", newAltLangMap);
    }
    else if (d->usageTermsEdit->isValid())
    {
        meta.removeXmpTag("Xmp.xmpRights.UsageTerms");
    }
}

void XMPContent::slotSyncCaptionOptionsEnabled(bool defaultLangAlt)
{
    bool cond = defaultLangAlt & d->captionEdit->isValid();
    d->syncJFIFCommentCheck->setEnabled(cond);
    d->syncEXIFCommentCheck->setEnabled(cond);
}

void XMPContent::slotSyncCopyrightOptionsEnabled(bool defaultLangAlt)
{
    bool cond = defaultLangAlt & d->copyrightEdit->isValid();
    d->syncEXIFCopyrightCheck->setEnabled(cond);
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_xmpcontent.cpp"
