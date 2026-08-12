/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2014 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"
#include "digikam_debug.h"
#include "histogrambox.h"
#include "digikam_globals.h"

class QPushButton;
class QPixmap;
class QString;
class QIcon;

namespace Digikam
{

class HistogramBox;
class EditorTool;

class DIGIKAM_EXPORT EditorToolSettings : public QScrollArea
{
    Q_OBJECT

public:

    enum ButtonCode
    {
        Default = 0x00000001,
        Try     = 0x00000002,
        Ok      = 0x00000004,
        Cancel  = 0x00000008,
        SaveAs  = 0x00000010,
        Load    = 0x00000020
    };
    Q_DECLARE_FLAGS(Buttons, ButtonCode)

    enum ToolCode
    {
        NoTool     = 0x00000000,
        ColorGuide = 0x00000001,
        Histogram  = 0x00000002
    };
    Q_DECLARE_FLAGS(Tools, ToolCode)

public:

    explicit EditorToolSettings(QWidget* const parent);
    ~EditorToolSettings()                     override;

    void setButtons(Buttons buttonMask);
    void setTools(Tools toolMask);
    void setHistogramType(HistogramBoxType type);
    void setTool(EditorTool* const tool);

    int marginHint();
    int spacingHint();

    QWidget*      plainPage()           const;
    HistogramBox* histogramBox()        const;

    QColor guideColor()                 const;
    void setGuideColor(const QColor& color);

    int guideSize()                     const;
    void setGuideSize(int size);

    QPushButton* button(int buttonCode) const;
    void enableButton(int buttonCode, bool state);

    QSize minimumSizeHint()             const override;
    virtual void setBusy(bool)   {};
    virtual void writeSettings() {};
    virtual void readSettings()  {};
    virtual void resetSettings() {};

Q_SIGNALS:

    void signalOkClicked();
    void signalCancelClicked();
    void signalTryClicked();
    void signalDefaultClicked();
    void signalSaveAsClicked();
    void signalLoadClicked();
    void signalColorGuideChanged();
    void signalChannelChanged();
    void signalScaleChanged();

private Q_SLOTS:

    void slotAboutPlugin();
    void slotHelpPlugin();

private:

    /// @note disabled
    EditorToolSettings() = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::EditorToolSettings::Buttons)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::EditorToolSettings::Tools)
