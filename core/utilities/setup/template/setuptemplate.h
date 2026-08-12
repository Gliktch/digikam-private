/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : metadata template setup page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "template.h"

namespace Digikam
{

class SetupTemplate : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupTemplate(QWidget* const parent = nullptr);
    ~SetupTemplate() override;

    void applySettings();
    void setTemplate(const Template& t);

private:

    void readSettings();
    void populateTemplate(const Template& t);

private Q_SLOTS:

    void slotSelectionChanged();
    void slotAddTemplate();
    void slotDelTemplate();
    void slotRepTemplate();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
