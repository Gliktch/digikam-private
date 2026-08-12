/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with Google web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.GoogleServices"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

class GSWindow;

class GSPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit GSPlugin(QObject* const parent = nullptr);
    ~GSPlugin()                          override = default;

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;

    void setup(QObject* const)           override;
    void cleanUp()                       override;

private Q_SLOTS:

    void slotImportGphoto();
    void slotExportGphoto();
    void slotExportGdrive();

private:

    QPointer<GSWindow> m_toolDlgImportGphoto;
    QPointer<GSWindow> m_toolDlgExportGphoto;
    QPointer<GSWindow> m_toolDlgExportGdrive;
};

} // namespace DigikamGenericGoogleServicesPlugin
