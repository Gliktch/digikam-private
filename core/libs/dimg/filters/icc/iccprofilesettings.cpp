/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Icc profile settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccprofilesettings.h"

// Qt includes

#include <QCache>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "iccprofilescombobox.h"
#include "iccprofileinfodlg.h"
#include "iccsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN IccProfilesSettings::Private
{
public:

    Private()
    {
        favoriteProfiles.setMaxCost(10);
    }

    const QString configRecentlyUsedProfilesEntry = QLatin1String("Recently Used Profiles");

    QCache<QString, bool> favoriteProfiles;
    IccProfilesComboBox*  profilesBox = nullptr;
};

// --------------------------------------------------------

IccProfilesSettings::IccProfilesSettings(QWidget* const parent)
    : DVBox(parent),
      d    (new Private)
{
    QLabel* const newProfileLabel  = new QLabel(i18n("Convert to:"), this);
    d->profilesBox                 = new IccProfilesComboBox(this);
    d->profilesBox->addProfilesSqueezed(IccSettings::instance()->workspaceProfiles());
    d->profilesBox->setWhatsThis(i18n("Select the profile of the color space to convert to."));
    newProfileLabel->setBuddy(d->profilesBox);
    QPushButton* const newProfInfo = new QPushButton(i18n("Info..."), this);

    layout()->setAlignment(newProfInfo, Qt::AlignLeft);
    const int spacing = layoutSpacing();

    setSpacing(spacing);
    setContentsMargins(QMargins());

    // -------------------------------------------------------------

    connect(d->profilesBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotProfileChanged()));

    connect(newProfInfo, SIGNAL(clicked()),
            this, SLOT(slotNewProfInfo()));
}

IccProfilesSettings::~IccProfilesSettings()
{
    delete d;
}

void IccProfilesSettings::slotNewProfInfo()
{
    ICCProfileInfoDlg infoDlg(qApp->activeWindow(), QString(), d->profilesBox->currentProfile());
    infoDlg.exec();
}

void IccProfilesSettings::slotProfileChanged()
{
    d->favoriteProfiles.insert(d->profilesBox->currentProfile().filePath(), new bool(true));

    Q_EMIT signalSettingsChanged();
}

IccProfile IccProfilesSettings::currentProfile() const
{
    return d->profilesBox->currentProfile();
}

void IccProfilesSettings::setCurrentProfile(const IccProfile& prof)
{
    blockSignals(true);
    d->profilesBox->setCurrentProfile(prof);
    blockSignals(false);
}

void IccProfilesSettings::resetToDefault()
{
    blockSignals(true);
    d->profilesBox->setCurrentIndex(0);
    blockSignals(false);
}

IccProfile IccProfilesSettings::defaultProfile() const
{
    return d->profilesBox->itemData(0).value<IccProfile>();
}

void IccProfilesSettings::readSettings(KConfigGroup& group)
{
    QStringList lastProfiles = group.readPathEntry(d->configRecentlyUsedProfilesEntry, QStringList());

    for (const QString& path : std::as_const(lastProfiles))
    {
        d->favoriteProfiles.insert(path, new bool(true));
    }
}

void IccProfilesSettings::writeSettings(KConfigGroup& group)
{
    group.writePathEntry(d->configRecentlyUsedProfilesEntry, d->favoriteProfiles.keys());
}

// Static Methods.

QStringList IccProfilesSettings::favoriteProfiles(KConfigGroup& group)
{
    Private d;

    return group.readPathEntry(d.configRecentlyUsedProfilesEntry, QStringList());
}

} // namespace Digikam

#include "moc_iccprofilesettings.cpp"
