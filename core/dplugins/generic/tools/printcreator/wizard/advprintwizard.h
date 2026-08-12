/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-11
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2008-2012 by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "advprintsettings.h"
#include "ditemslist.h"
#include "dinfointerface.h"
#include "dwizarddlg.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit AdvPrintWizard(QWidget* const, DInfoInterface* const iface = nullptr);
    ~AdvPrintWizard()                   override;

    void setItemsList(const QList<QUrl>& fileList = QList<QUrl>());
    QList<QUrl> itemsList()       const;

    DInfoInterface*   iface()     const;
    AdvPrintSettings* settings()  const;

    /**
     * Update the pages to be printed and preview first/last pages.
     */
    void previewPhotos();

    void updateCropFrame(AdvPrintPhoto* const, int);

    int  nextId()                 const override;

    static int normalizedInt(double n);

private:

    bool eventFilter(QObject*, QEvent*) override;

private Q_SLOTS:

    void slotPreview(const QImage&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPrintCreatorPlugin
