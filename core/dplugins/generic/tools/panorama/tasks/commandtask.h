/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-07
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QProcess>

// Local includes

#include "panotask.h"

namespace DigikamGenericPanoramaPlugin
{

class CommandTask : public PanoTask
{
public:

    explicit CommandTask(PanoAction action, const QString& workDirPath, const QString& commandPath);
    ~CommandTask()          override = default;

    void    requestAbort()  override;

protected:

    QString output;

protected:

    void    runProcess(QStringList& args);
    QString getProgram();
    QString getCommandLine();
    QString getProcessError();
    void    printDebug(const QString& binaryName);

private:

    QSharedPointer<QProcess> process;
    QString                  commandPath;

private:

    /// @note disabled
    CommandTask() = delete;

    Q_DISABLE_COPY(CommandTask)
};

} // namespace DigikamGenericPanoramaPlugin
