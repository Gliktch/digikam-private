/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC subjects settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace DigikamGenericMetadataEditPlugin
{

class MetadataEditPage : public QScrollArea
{
    Q_OBJECT

public:

    explicit MetadataEditPage(QWidget* const parent);
    ~MetadataEditPage() override = default;
};

} // namespace DigikamGenericMetadataEditPlugin
