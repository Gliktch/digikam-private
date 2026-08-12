/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a tool to insert a text over an image.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

class QFont;

using namespace Digikam;

namespace DigikamEditorInsertTextToolPlugin
{

class InsertTextTool : public EditorTool
{
    Q_OBJECT

public:

    explicit InsertTextTool(QObject* const parent);
    ~InsertTextTool()                           override;

Q_SIGNALS:

    void signalUpdatePreview();

private Q_SLOTS:

    void slotResetSettings()                    override;
    void slotFontPropertiesChanged(const QFont& font);
    void slotUpdatePreview();
    void slotAlignModeChanged(int mode);

private:

    void readSettings()                         override;
    void writeSettings()                        override;
    void finalRendering()                       override;
    void setBackgroundColor(const QColor& bg)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorInsertTextToolPlugin
