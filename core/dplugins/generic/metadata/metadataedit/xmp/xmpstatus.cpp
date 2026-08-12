/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow status settings page.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "xmpstatus.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "altlangstringedit.h"
#include "multistringsedit.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN XMPStatus::Private
{
public:

    Private() = default;

    QCheckBox*           nicknameCheck              = nullptr;
    QCheckBox*           specialInstructionCheck    = nullptr;

    DTextEdit*           nicknameEdit               = nullptr;

    DPlainTextEdit*      specialInstructionEdit     = nullptr;

    MultiStringsEdit*    identifiersEdit            = nullptr;

    AltLangStringsEdit*  objectNameEdit             = nullptr;
};

XMPStatus::XMPStatus(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid  = new QGridLayout(widget());

    // --------------------------------------------------------

    d->objectNameEdit  = new AltLangStringsEdit(this, i18nc("short title for the content", "Title:"),
                                                i18n("Set here a shorthand reference for the content."));

    // --------------------------------------------------------

    d->nicknameCheck = new QCheckBox(i18n("Nickname:"), this);
    d->nicknameEdit  = new DTextEdit(this);
    d->nicknameEdit->setLinesVisible(1);
    d->nicknameEdit->setPlaceholderText(i18n("Set here a short informal name"));
    d->nicknameEdit->setWhatsThis(i18n("A short informal name for the resource."));

    // --------------------------------------------------------

    d->identifiersEdit = new MultiStringsEdit(this, i18n("Identifiers:"),
                             i18n("Set here the strings that identify content that recurs."));

    // --------------------------------------------------------

    d->specialInstructionCheck = new QCheckBox(i18n("Special Instructions:"), this);
    d->specialInstructionEdit  = new DPlainTextEdit(this);
    d->specialInstructionEdit->setLinesVisible(4);
    d->specialInstructionEdit->setPlaceholderText(i18n("Set here the editorial usage instructions"));
    d->specialInstructionEdit->setWhatsThis(i18n("Enter the editorial usage instructions."));

    // --------------------------------------------------------

    grid->addWidget(d->objectNameEdit,          0, 0, 1, 3);
    grid->addWidget(d->nicknameCheck,           1, 0, 1, 1);
    grid->addWidget(d->nicknameEdit,            1, 1, 1, 2);
    grid->addWidget(d->identifiersEdit,         2, 0, 1, 3);
    grid->addWidget(d->specialInstructionCheck, 3, 0, 1, 3);
    grid->addWidget(d->specialInstructionEdit,  4, 0, 1, 3);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(2, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->specialInstructionCheck, SIGNAL(toggled(bool)),
            d->specialInstructionEdit, SLOT(setEnabled(bool)));

    connect(d->nicknameCheck, SIGNAL(toggled(bool)),
            d->nicknameEdit, SLOT(setEnabled(bool)));

    connect(d->identifiersEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->objectNameEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->nicknameCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->specialInstructionCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->objectNameEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->nicknameEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->specialInstructionEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

XMPStatus::~XMPStatus()
{
    delete d;
}

void XMPStatus::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString            data;
    QStringList        list;
    DMetadata::AltLangMap map;

    d->objectNameEdit->setValues(map);
    d->objectNameEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.dc.title", false);

    if (!map.isEmpty())
    {
        d->objectNameEdit->setValues(map);
        d->objectNameEdit->setValid(true);
    }

    d->nicknameEdit->clear();
    d->nicknameCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.xmp.Nickname", false);

    if (!data.isNull())
    {
        d->nicknameEdit->setText(data);
        d->nicknameCheck->setChecked(true);
    }

    d->nicknameEdit->setEnabled(d->nicknameCheck->isChecked());

    list = meta.getXmpTagStringSeq("Xmp.xmp.Identifier", false);
    d->identifiersEdit->setValues(list);

    d->specialInstructionEdit->clear();
    d->specialInstructionCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Instructions", false);

    if (!data.isNull())
    {
        d->specialInstructionEdit->setPlainText(data);
        d->specialInstructionCheck->setChecked(true);
    }

    d->specialInstructionEdit->setEnabled(d->specialInstructionCheck->isChecked());

    blockSignals(false);
}

void XMPStatus::applyMetadata(const DMetadata& meta)
{
    QStringList oldList, newList;

    DMetadata::AltLangMap oldAltLangMap, newAltLangMap;

    if      (d->objectNameEdit->getValues(oldAltLangMap, newAltLangMap))
    {
        meta.setXmpTagStringListLangAlt("Xmp.dc.title", newAltLangMap);
    }
    else if (d->objectNameEdit->isValid())
    {
        meta.removeXmpTag("Xmp.dc.title");
    }

    if (d->nicknameCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.xmp.Nickname", d->nicknameEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.xmp.Nickname");
    }

    if (d->identifiersEdit->getValues(oldList, newList))
    {
        meta.setXmpTagStringSeq("Xmp.xmp.Identifier", newList);
    }
    else
    {
        meta.removeXmpTag("Xmp.xmp.Identifier");
    }

    if (d->specialInstructionCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.Instructions", d->specialInstructionEdit->toPlainText());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.Instructions");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_xmpstatus.cpp"
