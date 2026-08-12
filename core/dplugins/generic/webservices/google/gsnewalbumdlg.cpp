/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-01
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2010 by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gsnewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QIcon>
#include <QApplication>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "gsitem.h"

namespace DigikamGenericGoogleServicesPlugin
{

class Q_DECL_HIDDEN GSNewAlbumDlg::Private
{
public:

    Private() = default;

    QString        serviceName;
    QRadioButton*  publicRBtn       = nullptr;
    QRadioButton*  unlistedRBtn     = nullptr;
    QRadioButton*  protectedRBtn    = nullptr;
};

GSNewAlbumDlg::GSNewAlbumDlg(QWidget* const parent,
                             const QString& serviceName,
                             const QString& toolName)
    : WSNewAlbumDialog(parent, toolName),
      d               (new Private)
{
    d->serviceName           = serviceName;
    const int spacing        = layoutSpacing();

    QGroupBox* const privBox = new QGroupBox(i18n("Access Level"), getMainWidget());
    privBox->setWhatsThis(i18n("These are security and privacy settings for the new Google Photos album."));

    d->publicRBtn        = new QRadioButton(i18nc("google photos album privacy", "Public"));
    d->publicRBtn->setChecked(true);
    d->publicRBtn->setWhatsThis(i18n("Public album is listed on your public Google Photos page."));
    d->unlistedRBtn      = new QRadioButton(i18nc("google photos album privacy", "Unlisted / Private"));
    d->unlistedRBtn->setWhatsThis(i18n("Unlisted album is only accessible via URL."));
    d->protectedRBtn     = new QRadioButton(i18nc("google photos album privacy", "Sign-In Required to View"));
    d->protectedRBtn->setWhatsThis(i18n("Unlisted album require Sign-In to View"));

    QVBoxLayout* const radioLayout = new QVBoxLayout;
    radioLayout->addWidget(d->publicRBtn);
    radioLayout->addWidget(d->unlistedRBtn);
    radioLayout->addWidget(d->protectedRBtn);

    QFormLayout* const privBoxLayout = new QFormLayout;
    privBoxLayout->addRow(i18n("Privacy:"), radioLayout);
    privBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    privBoxLayout->setSpacing(spacing);
    privBox->setLayout(privBoxLayout);

/*  Album on google-photos now only needs title to create, so gdrive and gphoto now share the same newalbumdlg

    if (!(QString::compare(d->serviceName,
                           QLatin1String("googledriveexport"),
                           Qt::CaseInsensitive) == 0))
    {
        addToMainLayout(privBox);
    }
    else
    {
        privBox->hide();
        hideDateTime();
        hideDesc();
        hideLocation();
        getMainWidget()->setMinimumSize(300,0);
    }
*/

    privBox->hide();
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300,0);

}

GSNewAlbumDlg::~GSNewAlbumDlg()
{
    delete d;
}

void GSNewAlbumDlg::getAlbumProperties(GSFolder& album)
{
/*  Album on google-photos now only needs title to create, so gdrive and gphoto now share the same newalbumdlg

    if (QString::compare(d->serviceName,
                         QLatin1String("googledriveexport"),
                         Qt::CaseInsensitive) == 0)
    {
        album.title = getTitleEdit()->text();
        return;
    }

    album.title       = getTitleEdit()->text();
    album.description = getDescEdit()->toPlainText();
    album.location    = getLocEdit()->text();

    if (d->publicRBtn->isChecked())
    {
        album.access = QLatin1String("public");
    }
    else if (d->unlistedRBtn->isChecked())
    {
        album.access = QLatin1String("private");
    }
    else
    {
        album.access = QLatin1String("protected");
    }

    long long timestamp = getDateTimeEdit()->dateTime().toTime_t();
    album.timestamp     = QString::number(timestamp * 1000);
*/

    album.title = getTitleEdit()->text();
    return;
}

} // namespace DigikamGenericGoogleServicesPlugin

#include "moc_gsnewalbumdlg.cpp"
