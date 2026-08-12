/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupMime : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupMime(QWidget* const parent = nullptr);
    ~SetupMime() override;

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotShowCurrentImageSettings();
    void slotShowCurrentMovieSettings();
    void slotShowCurrentAudioSettings();

private:

    void setupMime();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
