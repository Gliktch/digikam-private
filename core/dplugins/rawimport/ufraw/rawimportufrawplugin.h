/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : UFRaw raw import plugin.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QProcess>

// Local includes

#include "dpluginrawimport.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.rawimport.UFRaw"

using namespace Digikam;

namespace DigikamRawImportUFRawPlugin
{

// cppcheck-suppress noConstructor
class UFRawRawImportPlugin : public DPluginRawImport
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginRawImport)

public:

    explicit UFRawRawImportPlugin(QObject* const parent = nullptr);
    ~UFRawRawImportPlugin()                                    override;

    QString name()                                       const override;
    QString iid()                                        const override;
    QIcon   icon()                                       const override;
    QString details()                                    const override;
    QString description()                                const override;
    QList<DPluginAuthor> authors()                       const override;
    QString handbookSection()                            const override;
    QString handbookChapter()                            const override;
    QString handbookReference()                          const override;
    QString getRawProgram()                              const override;

    void setup(QObject* const)                                 override;

    bool run(const QString& filePath, const DRawDecoding& def) override;

private Q_SLOTS:

    void slotErrorOccurred(QProcess::ProcessError);
    void slotProcessFinished(int, QProcess::ExitStatus);
    void slotProcessReadyRead();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamRawImportUFRawPlugin
