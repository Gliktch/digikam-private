/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-16
 * Description : Spell-check Config widget.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SpellCheckConfig : public QWidget
{
    Q_OBJECT

public:

    explicit SpellCheckConfig(QWidget* const parent = nullptr);
    ~SpellCheckConfig() override;

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotSpellcheckActivated(bool);
    void slotIgnoreWordSelectionChanged();
    void slotAddWord();
    void slotDelWord();
    void slotRepWord();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
