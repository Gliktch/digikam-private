/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : a checkbox with a boolean valid parameter.
 *               The boolean statement is used to check if
 *               a metadata value from a picture have a know
 *               value registered by EXIF/IPTC spec.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QCheckBox>

namespace DigikamGenericMetadataEditPlugin
{

class MetadataCheckBox : public QCheckBox
{
    Q_OBJECT

public:

    explicit MetadataCheckBox(const QString& text, QWidget* const parent);
    ~MetadataCheckBox() override = default;

    void setValid(bool v);
    bool isValid() const;

private Q_SLOTS:

    void slotValid();

private:

    bool m_valid = true;
};

} // namespace DigikamGenericMetadataEditPlugin
