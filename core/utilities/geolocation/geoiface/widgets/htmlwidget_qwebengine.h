/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Widget for displaying HTML in the backends - QtWebEngine version
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTimer>
#include <QWebEngineView>
#include <QWebEnginePage>

// Local includes

#include "geoifacecommon.h"
#include "geoifacetypes.h"
#include "geocoordinates.h"

namespace Digikam
{

class HTMLWidget;

class HTMLWidgetPage : public QWebEnginePage
{
    Q_OBJECT

public:

    explicit HTMLWidgetPage(HTMLWidget* const parent = nullptr);
    ~HTMLWidgetPage() override = default;

Q_SIGNALS:

    void signalHTMLEvents(const QStringList& events);
    void signalMessageEvent(const QString& message);

private Q_SLOTS:

    void slotSendHTMLEvents();

protected:

    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel, const QString&, int, const QString&) override;

private:

    QStringList m_events;
    QString     m_message;
    QTimer*     m_timer = nullptr;
};

// -------------------------------------------------------------------

class HTMLWidget : public QWebEngineView
{
    Q_OBJECT

public:

    explicit HTMLWidget(QWidget* const parent = nullptr);
    ~HTMLWidget() override;

    void loadInitialHTML(const QString& initialHTML);
    QVariant runScript(const QString& scriptCode, bool async = true);
    bool runScript2Coordinates(const QString& scriptCode, GeoCoordinates* const coordinates);
    void mouseModeChanged(const GeoMouseModes mouseMode);
    void setSelectionRectangle(const GeoCoordinates::Pair& searchCoordinates);
    void removeSelectionRectangle();
    void centerOn(const qreal west, const qreal north, const qreal east, const qreal south,
                  const bool useSaneZoomLevel = true);
    void setSharedGeoIfaceObject(GeoIfaceSharedData* const sharedData);

Q_SIGNALS:

    void signalHTMLEvents(const QStringList& events);
    void signalMessageEvent(const QString& message);
    void signalJavaScriptReady();
    void selectionHasBeenMade(const Digikam::GeoCoordinates::Pair& coordinatesRect);

protected:

    bool eventFilter(QObject*, QEvent*) override;

protected Q_SLOTS:

    void slotHTMLCompleted(bool ok);
    void progress(int progress);

private:

    class Private;
    Private* const      d = nullptr;

    GeoIfaceSharedData* s = nullptr;
};

} // namespace Digikam
