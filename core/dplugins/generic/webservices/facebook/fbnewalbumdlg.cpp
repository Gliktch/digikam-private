/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fbnewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QComboBox>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "fbitem.h"

namespace DigikamGenericFaceBookPlugin
{

FbNewAlbumDlg::FbNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();

    m_privacyCoB        = new QComboBox;
    m_privacyCoB->setEditable(false);
    m_privacyCoB->setWhatsThis(i18n("Privacy setting of the album that will be created (required)."));
    m_privacyCoB->addItem(QIcon::fromTheme(QLatin1String("secure-card")),
                          i18n("Only Me"),                 FB_ME);
    m_privacyCoB->addItem(QIcon::fromTheme(QLatin1String("user-identity")),
                          i18n("Only Friends"),            FB_FRIENDS);
    m_privacyCoB->addItem(QIcon::fromTheme(QLatin1String("system-users")),
                          i18n("Friends of Friends"),      FB_FRIENDS_OF_FRIENDS);
/*
    m_privacyCoB->addItem(QIcon::fromTheme(QLatin1String("network-workgroup")),
                          i18n("My Networks and Friends"), FB_NETWORKS);
*/
    m_privacyCoB->addItem(QIcon::fromTheme(QLatin1String("folder-html")),
                          i18n("Everyone"),                FB_EVERYONE);
    m_privacyCoB->setCurrentIndex(1);

    addToMainLayout(m_privacyCoB);
}

FbNewAlbumDlg::~FbNewAlbumDlg()
{
}

void FbNewAlbumDlg::getAlbumProperties(FbAlbum& album)
{
    WSAlbum baseAlbum;
    getBaseAlbumProperties(baseAlbum);

    album.setBaseAlbum(baseAlbum);
    album.privacy = static_cast<DigikamGenericFaceBookPlugin::FbPrivacy>(m_privacyCoB->itemData(m_privacyCoB->currentIndex()).toInt());
}

} // namespace DigikamGenericFaceBookPlugin

#include "moc_fbnewalbumdlg.cpp"
