/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-23
 * Description : a widget to select between system font or a custom font.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFont>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFontSelect : public DHBox
{
    Q_OBJECT

public:

    enum FontMode
    {
        SystemFont = 0,
        CustomFont
    };

public:

    explicit DFontSelect(const QString& text, QWidget* const parent = nullptr);
    ~DFontSelect() override;

    void setMode(FontMode mode);
    FontMode mode() const;

    QFont font()    const;
    void setFont(const QFont& font);

Q_SIGNALS:

    void signalFontChanged();

protected:

    bool event(QEvent* e) override;

private Q_SLOTS:

    void slotOpenFontDialog();
    void slotChangeMode(int index);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
