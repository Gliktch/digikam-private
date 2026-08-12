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

#include "htmlwidget_qwebengine.h"

// Qt includes

#include <QEventLoop>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QWebEngineSettings>
#include <QtWebEngineWidgetsVersion>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

HTMLWidgetPage::HTMLWidgetPage(HTMLWidget* const parent)
    : QWebEnginePage(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    m_timer->setSingleShot(true);

    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(slotSendHTMLEvents()),
            Qt::QueuedConnection);
}

void HTMLWidgetPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel /*level*/,
                                              const QString& message,
                                              int /*lineNumber*/,
                                              const QString& /*sourceID*/)
{
    if (!message.startsWith(QLatin1String("(event)")))
    {
        m_message = message;
        m_timer->start();

        return;
    }

    qCDebug(DIGIKAM_GEOIFACE_LOG) << message;

    const QString eventString = message.mid(7);

    if (eventString.isEmpty())
    {
        return;
    }

    m_events << eventString;
    m_timer->start();
}

void HTMLWidgetPage::slotSendHTMLEvents()
{
    if (!m_message.isEmpty())
    {
        Q_EMIT signalMessageEvent(m_message);

        m_message.clear();
    }

    if (!m_events.isEmpty())
    {
        Q_EMIT signalHTMLEvents(m_events);

        m_events.clear();
    }
}

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN HTMLWidget::Private
{
public:

    Private() = default;

    QWidget*        parent              = nullptr;
    QWidget*        child               = nullptr;
    HTMLWidgetPage* hpage               = nullptr;

    bool            isReady             = false;
    bool            selectionStatus     = false;

    GeoCoordinates  firstSelectionPoint;
    GeoCoordinates  intermediateSelectionPoint;

    QPoint          firstSelectionScreenPoint;
    QPoint          intermediateSelectionScreenPoint;
};

HTMLWidget::HTMLWidget(QWidget* const parent)
    : QWebEngineView(parent),
      d             (new Private)
{
    d->parent = parent;
    setAcceptDrops(false);
    setFocusPolicy(Qt::WheelFocus);

    d->hpage  = new HTMLWidgetPage(this);
    setPage(d->hpage);

    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

    d->parent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(progress(int)));

    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(slotHTMLCompleted(bool)));

    connect(d->hpage, SIGNAL(signalHTMLEvents(QStringList)),
            this, SIGNAL(signalHTMLEvents(QStringList)));

    connect(d->hpage, SIGNAL(signalMessageEvent(QString)),
            this, SIGNAL(signalMessageEvent(QString)));

    if (d->parent)
    {
        d->parent->installEventFilter(this);
    }

    installEventFilter(this);
}

HTMLWidget::~HTMLWidget()
{
    delete d;
}

void HTMLWidget::progress(int progress)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Maps Loading Progress: " << progress << "%";
}

void HTMLWidget::slotHTMLCompleted(bool ok)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Map Loading Completed: " << ok;
    d->isReady = ok;

    Q_EMIT signalJavaScriptReady();
}

/**
 * @brief Wrapper around executeScript to catch more errors
 */
QVariant HTMLWidget::runScript(const QString& scriptCode, bool async)
{
    GEOIFACE_ASSERT(d->isReady);

    if (!d->isReady)
    {
        return QVariant();
    }
/*
    qCDebug(DIGIKAM_GEOIFACE_LOG) << scriptCode;
*/
    if (async)
    {
        page()->runJavaScript(scriptCode);
    }
    else
    {
        QVariant   ret;
        QEventLoop loop;

        // Lambda C++11 function capturing value returned by java script code which is not synchro with QWebEngineView.
        // See https://wiki.qt.io/Porting_from_QtWebKit_to_QtWebEngine.

        page()->runJavaScript(scriptCode,
                              [&ret, &loop](const QVariant& result)
            {
                ret.setValue(result);
                loop.quit();
            }
        );

        loop.exec();

        return ret;
    }

    return QVariant();
}

/**
 * @brief Execute a script which returns coordinates and parse these
 */
bool HTMLWidget::runScript2Coordinates(const QString& scriptCode, GeoCoordinates* const coordinates)
{
    const QVariant scriptResult = runScript(scriptCode, false);

    return GeoIfaceHelperParseLatLonString(scriptResult.toString(), coordinates);
}

bool HTMLWidget::eventFilter(QObject* object, QEvent* event)
{
    if      (object == this)
    {
        if (event->type() == QEvent::ChildAdded)
        {
            d->child = findChild<QWidget*>();

            if (d->child)
            {
                d->child->installEventFilter(this);
            }
        }

        return QWebEngineView::eventFilter(object, event);
    }
    else if (d->parent && (object == d->parent))
    {
        if (event->type() == QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);

            if (resizeEvent)
            {
                resize(resizeEvent->size());
            }
        }
    }
    else if (d->child && (object == d->child))
    {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* const e = dynamic_cast<QMouseEvent*>(event);

            if (e && (s->currentMouseMode == MouseModeRegionSelection))
            {
                if (!d->firstSelectionPoint.hasCoordinates())
                {
                    runScript2Coordinates(QString::fromLatin1("kgeomapPixelToLatLng(%1, %2);")

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                          .arg(e->position().x())
                                          .arg(e->position().y()),

#else

                                          .arg(e->x())
                                          .arg(e->y()),

#endif

                                          &d->firstSelectionPoint);

                    d->firstSelectionScreenPoint = QPoint(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                        e->position().toPoint()

#else

                        e->x(), e->y()

#endif

                    );
                }
                else
                {
                    runScript2Coordinates(QString::fromLatin1("kgeomapPixelToLatLng(%1, %2);")

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                          .arg(e->position().toPoint().x())
                                          .arg(e->position().toPoint().y()),

#else

                                          .arg(e->x())
                                          .arg(e->y()),

#endif

                                          &d->intermediateSelectionPoint);

                    d->intermediateSelectionScreenPoint = QPoint(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                        e->position().toPoint()

#else

                        e->x(), e->y()

#endif

                    );

                    qreal lonWest = 0.0, latNorth = 0.0, lonEast = 0.0, latSouth = 0.0;

                    if (d->firstSelectionScreenPoint.x() < d->intermediateSelectionScreenPoint.x())
                    {
                        lonWest  = d->firstSelectionPoint.lon();
                        lonEast  = d->intermediateSelectionPoint.lon();
                    }
                    else
                    {
                        lonEast  = d->firstSelectionPoint.lon();
                        lonWest  = d->intermediateSelectionPoint.lon();
                    }

                    if (d->firstSelectionScreenPoint.y() < d->intermediateSelectionScreenPoint.y())
                    {
                        latNorth = d->firstSelectionPoint.lat();
                        latSouth = d->intermediateSelectionPoint.lat();
                    }
                    else
                    {
                        latNorth = d->intermediateSelectionPoint.lat();
                        latSouth = d->firstSelectionPoint.lat();
                    }

                    runScript(QLatin1String("kgeomapRemoveTemporarySelectionRectangle();"));
                    runScript(QString::fromLatin1("kgeomapSetSelectionRectangle(%1, %2, %3, %4);")
                              .arg(lonWest)
                              .arg(latNorth)
                              .arg(lonEast)
                              .arg(latSouth));

                    const GeoCoordinates::Pair selectionCoordinates(
                            GeoCoordinates(latNorth, lonWest),
                            GeoCoordinates(latSouth, lonEast));

                    d->firstSelectionPoint.clear();
                    d->intermediateSelectionPoint.clear();

                    Q_EMIT selectionHasBeenMade(selectionCoordinates);
                }
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* const e = dynamic_cast<QMouseEvent*>(event);

            if (e                                                 &&
                (s->currentMouseMode == MouseModeRegionSelection) &&
                d->firstSelectionPoint.hasCoordinates())
            {
                runScript2Coordinates(QString::fromLatin1("kgeomapPixelToLatLng(%1, %2);")

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                      .arg(e->position().toPoint().x())
                                      .arg(e->position().toPoint().y()),

#else

                                      .arg(e->x())
                                      .arg(e->y()),

#endif

                                      &d->intermediateSelectionPoint);

                d->intermediateSelectionScreenPoint = QPoint(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                        e->position().toPoint()

#else

                        e->x(), e->y()

#endif

                );

                qCDebug(DIGIKAM_GEOIFACE_LOG) << d->firstSelectionScreenPoint << QLatin1Char(' ')
                                              << d->intermediateSelectionScreenPoint;

                qreal lonWest = 0.0, latNorth = 0.0, lonEast = 0.0, latSouth = 0.0;

                if (d->firstSelectionScreenPoint.x() < d->intermediateSelectionScreenPoint.x())
                {
                    lonWest  = d->firstSelectionPoint.lon();
                    lonEast  = d->intermediateSelectionPoint.lon();
                }
                else
                {
                    lonEast  = d->firstSelectionPoint.lon();
                    lonWest  = d->intermediateSelectionPoint.lon();
                }

                if (d->firstSelectionScreenPoint.y() < d->intermediateSelectionScreenPoint.y())
                {
                    latNorth = d->firstSelectionPoint.lat();
                    latSouth = d->intermediateSelectionPoint.lat();
                }
                else
                {
                    latNorth = d->intermediateSelectionPoint.lat();
                    latSouth = d->firstSelectionPoint.lat();
                }

                runScript(QString::fromLatin1("kgeomapSetTemporarySelectionRectangle(%1, %2, %3, %4);")
                          .arg(lonWest)
                          .arg(latNorth)
                          .arg(lonEast)
                          .arg(latSouth));
            }
        }
    }

    return false;
}

void HTMLWidget::setSelectionRectangle(const GeoCoordinates::Pair& searchCoordinates)
{
    if (!searchCoordinates.first.hasCoordinates())
    {
        runScript(QLatin1String("kgeomapRemoveSelectionRectangle();"));

        return;
    }

    qreal West  = searchCoordinates.first.lon();
    qreal North = searchCoordinates.first.lat();
    qreal East  = searchCoordinates.second.lon();
    qreal South = searchCoordinates.second.lat();

    runScript(QString::fromLatin1("kgeomapSetSelectionRectangle(%1, %2, %3, %4);")
              .arg(West).arg(North).arg(East).arg(South));
}

void HTMLWidget::removeSelectionRectangle()
{
    runScript(QLatin1String("kgeomapRemoveSelectionRectangle();"));
}

void HTMLWidget::mouseModeChanged(const GeoMouseModes mouseMode)
{
    const bool inSelectionMode = (mouseMode == MouseModeRegionSelection);

    if (inSelectionMode)
    {
        d->firstSelectionPoint.clear();
        d->intermediateSelectionPoint.clear();
        runScript(QString::fromLatin1("kgeomapSelectionModeStatus(%1);").arg(inSelectionMode));
    }
    else
    {
        runScript(QString::fromLatin1("kgeomapSelectionModeStatus(%1);").arg(inSelectionMode));
    }
}

void HTMLWidget::centerOn(const qreal west, const qreal north,
                          const qreal east, const qreal south,
                          const bool useSaneZoomLevel)
{
/*
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "West:" << west
                                  << " North:" << north
                                  << " East:" << east
                                  << " South:" << south;
*/
    runScript(QString::fromLatin1("kgeomapSetMapBoundaries(%1, %2, %3, %4, %5);")
              .arg(west)
              .arg(north)
              .arg(east)
              .arg(south)
              .arg(useSaneZoomLevel ? 1 : 0));
}

void HTMLWidget::setSharedGeoIfaceObject(GeoIfaceSharedData* const sharedData)
{
    s = sharedData;
}

} // namespace Digikam

#include "moc_htmlwidget_qwebengine.cpp"
