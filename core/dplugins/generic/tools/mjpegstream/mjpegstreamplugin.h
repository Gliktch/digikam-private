/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : a plugin to share items with MJPEG Stream server.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.MjpegStream"

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class MjpegStreamDlg;

class MjpegStreamPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit MjpegStreamPlugin(QObject* const parent = nullptr);
    ~MjpegStreamPlugin()                 override = default;

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

    void slotMjpegStream();

private:

    QPointer<MjpegStreamDlg> m_toolDlg;
};

} // namespace DigikamGenericMjpegStreamPlugin
