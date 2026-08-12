/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "gallerytheme.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

class GalleryInfo;

/**
 * The wizard used by the user to select the various settings.
 */
class HTMLWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit HTMLWizard(QWidget* const parent, DInfoInterface* const iface = nullptr);
    ~HTMLWizard()                          override;

    GalleryInfo*      galleryInfo()  const;
    GalleryTheme::Ptr galleryTheme() const;

    bool validateCurrentPage()             override;
    int  nextId()                    const override;

    void setItemsList(const QList<QUrl>& urls);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
