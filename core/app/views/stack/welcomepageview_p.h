/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "welcomepageview.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QPushButton>
#include <QButtonGroup>
#include <QApplication>
#include <QStandardPaths>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStackedWidget>
#include <QStylePainter>
#include <QStyleOptionTab>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "daboutdata.h"
#include "thememanager.h"
#include "webbrowserdlg.h"

namespace Digikam
{

class Q_DECL_HIDDEN TitleEffect : public QGraphicsDropShadowEffect
{
    Q_OBJECT

public:

    explicit TitleEffect(QWidget* const parent = nullptr);
};

// ---

class Q_DECL_HIDDEN GradientWidget : public QWidget
{
    Q_OBJECT

public:

    explicit GradientWidget(QWidget* const parent = nullptr);

protected:

    void paintEvent(QPaintEvent*) override;
};

// ---

class Q_DECL_HIDDEN InvertedGradientWidget : public QWidget
{
    Q_OBJECT

public:

    explicit InvertedGradientWidget(QWidget* const parent = nullptr);

protected:

    void paintEvent(QPaintEvent*) override;
};

// ---

class Q_DECL_HIDDEN ResizableBackgroundWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ResizableBackgroundWidget(QWidget* const parent = nullptr);

protected:

    void resizeEvent(QResizeEvent* event) override;

    void paintEvent(QPaintEvent*)         override;

private:

    QPixmap m_backgroundPixmap = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                        QLatin1String("digikam/data/body-background.webp"));
};

} // namespace Digikam
