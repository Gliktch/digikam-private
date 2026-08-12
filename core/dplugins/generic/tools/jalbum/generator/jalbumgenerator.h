/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "dprogresswdg.h"
#include "dhistoryview.h"

using namespace Digikam;

namespace DigikamGenericJAlbumPlugin
{

class JAlbumSettings;
class JAlbumElementFunctor;

/**
 * This class is responsible for generating the HTML and scaling the images
 * according to the settings specified by the user.
 */
class JAlbumGenerator : public QObject
{
    Q_OBJECT

public:

    explicit JAlbumGenerator(JAlbumSettings* const);
    ~JAlbumGenerator() override;

    void setProgressWidgets(DHistoryView* const, DProgressWdg* const);

    bool run();
    bool warnings() const;

    /**
     * Produce a web-friendly file name
     */
    static QString webifyFileName(const QString&);

Q_SIGNALS:

    /**
     * This signal is emitted from JAlbumElementFunctor. It uses a
     * QueuedConnection to switch between the JAlbumElementFunctor thread and
     * the gui thread.
     */
    void logWarningRequested(const QString&);

private Q_SLOTS:

    void logWarning(const QString&);
    void slotCancel();

private:

    /// @note disabled
    explicit JAlbumGenerator(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;

    friend class Private;
    friend class JAlbumElementFunctor;
};

} // namespace DigikamGenericJAlbumPlugin
