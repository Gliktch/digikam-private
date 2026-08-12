/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>

// Local includes

#include "dwizarddlg.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericJAlbumPlugin
{

class JAlbumSettings;

/**
 * The wizard used by the user to select the various settings.
 */
class JAlbumWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit JAlbumWizard(QWidget* const parent, DInfoInterface* const iface = nullptr);
    ~JAlbumWizard()                  override;

    JAlbumSettings* settings() const;

    bool validateCurrentPage()       override;
    int  nextId()              const override;

    void setItemsList(const QList<QUrl>& urls);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericJAlbumPlugin
