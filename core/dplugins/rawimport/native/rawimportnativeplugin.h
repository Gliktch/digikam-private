/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-08
 * Description : a Raw Import plugin based on LibRaw.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dpluginrawimport.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.rawimport.Native"

using namespace Digikam;

namespace DigikamRawImportNativePlugin
{

class RawImportNativePlugin : public DPluginRawImport
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginRawImport)

public:

    explicit RawImportNativePlugin(QObject* const parent = nullptr);
    ~RawImportNativePlugin()                                   override = default;

    QString name()                                       const override;
    QString iid()                                        const override;
    QIcon   icon()                                       const override;
    QString details()                                    const override;
    QString description()                                const override;
    QList<DPluginAuthor> authors()                       const override;
    QString handbookSection()                            const override;
    QString handbookChapter()                            const override;
    QString handbookReference()                          const override;

    void setup(QObject* const)                                 override;

    bool run(const QString& filePath, const DRawDecoding& def) override;

private Q_SLOTS:

    void slotLoadRawFromTool();
    void slotLoadRaw();

private:

    QString      m_filePath;
    DRawDecoding m_defaultSettings;
};

} // namespace DigikamRawImportNativePlugin
