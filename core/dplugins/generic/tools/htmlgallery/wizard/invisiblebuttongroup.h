/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

class QAbstractButton;

namespace DigikamGenericHtmlGalleryPlugin
{

class InvisibleButtonGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ selected WRITE setSelected)

public:

    explicit InvisibleButtonGroup(QWidget* const parent);
    ~InvisibleButtonGroup() override;

    int selected() const;

    void addButton(QAbstractButton* button, int id);

public Q_SLOTS:

    void setSelected(int id);

Q_SIGNALS:

    void selectionChanged(int id);

private:

    /// @note disabled
    InvisibleButtonGroup() = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
