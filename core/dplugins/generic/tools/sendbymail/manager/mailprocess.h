/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-02-25
 * Description : a tool to e-mailing images
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "mailsettings.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericSendByMailPlugin
{

class MailProcess : public QObject
{
    Q_OBJECT

public:

    explicit MailProcess(MailSettings* const settings,
                         DInfoInterface* const iface,
                         QObject* const parent);
    ~MailProcess() override;

    void firstStage();

Q_SIGNALS:

    void signalProgress(int);
    void signalMessage(QString, bool);
    void signalComplete(bool);

public Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotStartingResize(const QUrl&);
    void slotFinishedResize(const QUrl&, const QUrl&, int);
    void slotFailedResize(const QUrl&, const QString&, int);
    void slotCompleteResize();
    void slotCleanUp();

private:

    void secondStage();
    void invokeMailAgentError(const QString& prog, const QStringList& args);
    void invokeMailAgentDone(const QString& prog, const QStringList& args);

    /**
     * Creates a text file with all images Comments, Tags, and Rating.
     */
    void buildPropertiesFile();

    /**
     * Invokes mail agent. Depending on which mail agent to be used, we have different
     * proceedings. Easy for every agent except of mozilla derivates
     */
    bool invokeMailAgent();

    /**
     * Shows up an error dialog about the problematic resized images.
     */
    bool showFailedResizedImages() const;

    /**
     * Returns a list of image urls, whose sum file-size is smaller than the quota set in dialog.
     * The returned list are images than we can send immediately, and are removed from
     * d->attachementFiles list.
     * Files which still in d->attachementFiles need to be send by another pass.
     */
    QList<QUrl> divideEmails();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSendByMailPlugin
