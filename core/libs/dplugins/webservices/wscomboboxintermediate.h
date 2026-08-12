/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : A combobox which also has an intermediate state.
 *               This is akin to the intermediate state in a checkbox and
 *               needed when a single combobox controls more than one item,
 *               which are manually set to different states.
 *               The intermediate state is indicated by appending an extra item
 *               with a user specified text (default is "Various"). Whenever an
 *               other item is set, this special state is removed from the list
 *               so it can never be selected explicitly.
 *
 * SPDX-FileCopyrightText: 2009      by Pieter Edelman <pieter dot edelman at gmx dot net>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QComboBox>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSComboBoxIntermediate : public QComboBox
{
    Q_OBJECT

public:

    /**
     * Initialize the combobox with a parent and a string to indicate the
     * intermediate state.
     */
    explicit WSComboBoxIntermediate(QWidget* const = nullptr, const QString& = QString());
    ~WSComboBoxIntermediate() override;

    /**
     * Set the state of the combobox to intermediate. The intermediate state is
     * 'unset' when another index is selected.
     */
    void setIntermediate(bool);

private Q_SLOTS:

    void slotIndexChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
