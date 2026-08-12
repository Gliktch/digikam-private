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

#include "mediawiki_job_p.h"

// Qt includes

#include <QNetworkReply>

// Local include

#include "mediawiki_iface.h"

namespace MediaWiki
{

Job::Job(JobPrivate& dd, QObject* const parent)
    : KJob (parent),
      d_ptr(&dd)
{
    setCapabilities(Job::Killable);
}

Job::~Job()
{
    delete d_ptr;
}

bool Job::doKill()
{
    Q_D(Job);

    if (d->reply != nullptr)
    {
        d->reply->abort();
    }

    return true;
}

void Job::connectReply()
{
    Q_D(Job);

    connect(d->reply, SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(processUploadProgress(qint64,qint64)));
}

void Job::processUploadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    setTotalAmount(Job::Bytes, bytesTotal);
    setProcessedAmount(Job::Bytes, bytesReceived);
}

} // namespace MediaWiki

#include "moc_mediawiki_job.cpp"
