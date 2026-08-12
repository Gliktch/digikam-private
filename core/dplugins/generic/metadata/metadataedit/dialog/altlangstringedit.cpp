/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-18
 * Description : a widget to edit multiple alternative
 *               language string entries and an activation checkbox.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Victor Dodon <dodonvictor at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "altlangstringedit.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>

// Local includes

#include "squeezedcombobox.h"
#include "metadatacheckbox.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN AltLangStringsEdit::Private
{
public:

    Private() = default;

    DMetadata::AltLangMap oldValues;
    MetadataCheckBox*     valueCheck = nullptr;
};

AltLangStringsEdit::AltLangStringsEdit(QWidget* const parent, const QString& title, const QString& desc)
    : AltLangStrEdit(parent),
      d             (new Private)
{
    d->valueCheck = new MetadataCheckBox(title, parent);
    setLinesVisible(4);
    setPlaceholderText(desc);
    setTitleWidget(d->valueCheck);

    // --------------------------------------------------------

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalToggled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotEnabledInternalWidgets(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    slotEnabledInternalWidgets(d->valueCheck->isChecked());

    // --------------------------------------------------------

    connect(this, SIGNAL(signalChanged(QString,QString)),
            this, SIGNAL(signalModified()));

    connect(this, SIGNAL(signalValueAdded(QString,QString)),
            this, SIGNAL(signalModified()));

    connect(this, SIGNAL(signalValueDeleted(QString)),
            this, SIGNAL(signalModified()));

    connect(this, SIGNAL(signalSelectionChanged(QString)),
            this, SLOT(slotSelectionChanged(QString)));
}

AltLangStringsEdit::~AltLangStringsEdit()
{
    delete d;
}

void AltLangStringsEdit::setValid(bool v)
{
    d->valueCheck->setValid(v);
    d->valueCheck->setChecked(v);
}

bool AltLangStringsEdit::isValid() const
{
    return d->valueCheck->isValid();
}

void AltLangStringsEdit::setValues(const DMetadata::AltLangMap& values)
{
    d->oldValues = values;
    AltLangStrEdit::setValues(values);
}

bool AltLangStringsEdit::getValues(DMetadata::AltLangMap& oldValues,
                                   DMetadata::AltLangMap& newValues) const
{
    oldValues = d->oldValues;
    newValues = values();

    return d->valueCheck->isChecked();
}

void AltLangStringsEdit::slotSelectionChanged(const QString& lang)
{
    Q_EMIT signalDefaultLanguageEnabled(lang == QLatin1String("x-default"));
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_altlangstringedit.cpp"
