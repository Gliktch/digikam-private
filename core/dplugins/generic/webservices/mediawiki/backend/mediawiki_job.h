/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// KDE includes

#include <kjob.h>

namespace MediaWiki
{

class Iface;
class JobPrivate;

/**
 * @brief The base class for all Iface jobs.
 */
class Job : public KJob
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Job)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
        NetworkError            = KJob::UserDefinedError + 1,
        XmlError,
        UserRequestDefinedError = KJob::UserDefinedError + 100,
        MissingMandatoryParameter
    };

public:

    /**
     * @brief Destructs the Job.
     */
    ~Job() override;

    /**
     * @brief Aborts this job quietly.
     */
    bool doKill() override;

protected:

    /**
     * @brief Constructs a Job by a private class.
     * @param dd a private class
     * @param parent the QObject parent
     */
    explicit Job(JobPrivate& dd, QObject* const parent = nullptr);

    /**
     * @brief Connects signals of the reply object (in the private object) to
     * slots of this base class.
     */
    void connectReply();

    /**
     * @brief The private d pointer.
     */
    JobPrivate* const d_ptr = nullptr;

private Q_SLOTS:

    void processUploadProgress(qint64 bytesReceived, qint64 bytesTotal);
};

} // namespace MediaWiki
