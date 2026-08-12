/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "NavigationFloatItem.h"

// Qt includes

#include <qmath.h>
#include <QContextMenuEvent>
#include <QRect>
#include <QSlider>
#include <QWidget>
#include <QPainter>
#include <QPixmapCache>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ui_navigation.h"
#include "ViewportParams.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "WidgetGraphicsItem.h"
#include "MarbleGraphicsGridLayout.h"

namespace Marble
{

NavigationFloatItem::NavigationFloatItem(const MarbleModel* marbleModel)
    : AbstractFloatItem(marbleModel, QPointF(-10, -30))
{
    // Plugin is visible by default on desktop systems
    const bool smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
    setEnabled(!smallScreen);
    setVisible(true);

    setCacheMode(NoCache);
    setBackground(QBrush(QColor(Qt::transparent)));
    setFrame(NoFrame);
}

NavigationFloatItem::~NavigationFloatItem()
{
    delete m_navigationWidget;
}

QStringList NavigationFloatItem::backendTypes() const
{
    return QStringList(QStringLiteral("navigation"));
}

QString NavigationFloatItem::name() const
{
    return i18n("Navigation");
}

QString NavigationFloatItem::guiString() const
{
    return i18n("&Navigation");
}

QString NavigationFloatItem::nameId() const
{
    return QStringLiteral("navigation");
}

QString NavigationFloatItem::version() const
{
    return QStringLiteral("1.0");
}

QString NavigationFloatItem::description() const
{
    return i18n("A plugin to add mouse control to zoom and move the map");
}

QString NavigationFloatItem::copyrightYears() const
{
    return QStringLiteral("2008, 2010, 2013");
}

QVector<PluginAuthor> NavigationFloatItem::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Dennis Nienhüser"), QStringLiteral("nienhueser@kde.org"))
           << PluginAuthor(QStringLiteral("Bastian Holst"), QStringLiteral("bastianholst@gmx.de"))
           << PluginAuthor(QStringLiteral("Mohammed Nafees"), QStringLiteral("nafees.technocool@gmail.com"));
}

QIcon NavigationFloatItem::icon() const
{
    return QIcon::fromTheme(QStringLiteral("circular-arrow-shape"));
}

void NavigationFloatItem::initialize()
{
    QWidget* navigationParent = new QWidget(nullptr);
    navigationParent->setAttribute(Qt::WA_NoSystemBackground, true);

    m_navigationWidget = new Ui::Navigation;
    m_navigationWidget->setupUi(navigationParent);

    m_widgetItem = new WidgetGraphicsItem(this);
    m_widgetItem->setWidget(navigationParent);

    MarbleGraphicsGridLayout* layout = new MarbleGraphicsGridLayout(1, 1);
    layout->addItem(m_widgetItem, 0, 0);

    setLayout(layout);

    if (m_showHomeButton)
    {
        activateHomeButton();
    }

    else
    {
        activateCurrentPositionButton();
    }
}

bool NavigationFloatItem::isInitialized() const
{
    return m_widgetItem;
}

void NavigationFloatItem::setProjection(const ViewportParams* viewport)
{
    if (viewport->radius() != m_oldViewportRadius)
    {
        m_oldViewportRadius = viewport->radius();
        // The slider depends on the map state (zoom factor)
        update();
    }

    AbstractFloatItem::setProjection(viewport);
}

bool NavigationFloatItem::eventFilter(QObject* object, QEvent* e)
{
    if (!enabled() || !visible())
    {
        return false;
    }

    MarbleWidget* widget = dynamic_cast<MarbleWidget*>(object);

    if (!widget)
    {
        return AbstractFloatItem::eventFilter(object, e);
    }

    if (m_marbleWidget != widget)
    {
        // Delayed initialization
        m_marbleWidget = widget;

        m_maxZoom = m_marbleWidget->maximumZoom();
        m_minZoom = m_marbleWidget->minimumZoom();

        m_navigationWidget->arrowDisc->setMarbleWidget(m_marbleWidget);
        connect(m_navigationWidget->arrowDisc, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()));

        connect(m_navigationWidget->homeButton, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()));

        if (m_showHomeButton)
        {
            activateHomeButton();
        }

        else
        {
            activateCurrentPositionButton();
        }

        connect(m_navigationWidget->zoomInButton, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()));
        connect(m_navigationWidget->zoomInButton, SIGNAL(clicked()),
                m_marbleWidget, SLOT(zoomIn()));

        m_navigationWidget->zoomSlider->setMaximum(m_maxZoom);
        m_navigationWidget->zoomSlider->setMinimum(m_minZoom);
        connect(m_navigationWidget->zoomSlider, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()));
        connect(m_navigationWidget->zoomSlider, SIGNAL(valueChanged(int)),
                m_marbleWidget, SLOT(setZoom(int)));

        connect(m_navigationWidget->zoomOutButton, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()));
        connect(m_navigationWidget->zoomOutButton, SIGNAL(clicked()),
                m_marbleWidget, SLOT(zoomOut()));

        connect(m_marbleWidget, SIGNAL(zoomChanged(int)), SLOT(updateButtons(int)));
        updateButtons(m_marbleWidget->zoom());
        connect(m_marbleWidget, SIGNAL(themeChanged(QString)), this, SLOT(selectTheme(QString)));
    }

    return AbstractFloatItem::eventFilter(object, e);
}

void NavigationFloatItem::selectTheme(const QString&)
{
    if (m_marbleWidget)
    {
        m_maxZoom = m_marbleWidget->maximumZoom();
        m_minZoom = m_marbleWidget->minimumZoom();
        m_navigationWidget->zoomSlider->setMaximum(m_maxZoom);
        m_navigationWidget->zoomSlider->setMinimum(m_minZoom);
        updateButtons(m_marbleWidget->zoom());
    }
}

void NavigationFloatItem::updateButtons(int zoomValue)
{
    bool const zoomInEnabled = m_navigationWidget->zoomInButton->isEnabled();
    bool const zoomOutEnabled = m_navigationWidget->zoomOutButton->isEnabled();
    int const oldZoomValue = m_navigationWidget->zoomSlider->value();
    m_navigationWidget->zoomInButton->setEnabled(zoomValue < m_maxZoom);
    m_navigationWidget->zoomOutButton->setEnabled(zoomValue > m_minZoom);
    m_navigationWidget->zoomSlider->setValue(zoomValue);

    if (zoomInEnabled != m_navigationWidget->zoomInButton->isEnabled() ||
        zoomOutEnabled != m_navigationWidget->zoomOutButton->isEnabled() ||
        oldZoomValue != zoomValue)
    {
        update();
    }
}

QPixmap NavigationFloatItem::pixmap(const QString& id)
{
    QPixmap result;

    if (!QPixmapCache::find(id, &result))
    {
        result = QPixmap(QLatin1String(":/") + id + QLatin1String(".png"));
        QPixmapCache::insert(id, result);
    }

    return result;
}

void NavigationFloatItem::paintContent(QPainter* painter)
{
    painter->drawPixmap(0, 0, pixmap(QLatin1String("marble/navigation/navigational_backdrop_top")));
    painter->drawPixmap(0, 70, pixmap(QLatin1String("marble/navigation/navigational_backdrop_center")));
    painter->drawPixmap(0, 311, pixmap(QLatin1String("marble/navigation/navigational_backdrop_bottom")));
}

void NavigationFloatItem::contextMenuEvent(QWidget* w, QContextMenuEvent* e)
{
    if (!m_contextMenu)
    {
        m_contextMenu = contextMenu();

        m_activateCurrentPositionButtonAction = new QAction(QIcon(),
                                                            i18n("Current Location Button"),
                                                            m_contextMenu);
        m_activateHomeButtonAction = new QAction(QIcon::fromTheme(QStringLiteral("go-home")),
                                                 i18n("Home Button"),
                                                 m_contextMenu);
        m_activateHomeButtonAction->setVisible(!m_showHomeButton);
        m_activateCurrentPositionButtonAction->setVisible(m_showHomeButton);
        m_contextMenu->addSeparator();
        m_contextMenu->addAction(m_activateCurrentPositionButtonAction);
        m_contextMenu->addAction(m_activateHomeButtonAction);

        connect(m_activateCurrentPositionButtonAction, SIGNAL(triggered()), SLOT(activateCurrentPositionButton()));
        connect(m_activateHomeButtonAction, SIGNAL(triggered()), SLOT(activateHomeButton()));
    }

    Q_ASSERT(m_contextMenu);
    m_contextMenu->exec(w->mapToGlobal(e->pos()));
}

void NavigationFloatItem::activateCurrentPositionButton()
{
    if (!isInitialized())
    {
        return;
    }

    QIcon icon;
    icon.addPixmap(pixmap(QLatin1String("marble/navigation/navigational_currentlocation")), QIcon::Normal);
    icon.addPixmap(pixmap(QLatin1String("marble/navigation/navigational_currentlocation_hover")), QIcon::Active);
    icon.addPixmap(pixmap(QLatin1String("marble/navigation/navigational_currentlocation_pressed")), QIcon::Selected);
    m_navigationWidget->homeButton->setProperty("icon", QVariant(icon));

    if (m_contextMenu)
    {
        m_activateCurrentPositionButtonAction->setVisible(false);
        m_activateHomeButtonAction->setVisible(true);
    }

    if (m_marbleWidget)
    {
        disconnect(m_navigationWidget->homeButton, SIGNAL(clicked()), m_marbleWidget, SLOT(goHome()));
    }

    connect(m_navigationWidget->homeButton, SIGNAL(clicked()), SLOT(centerOnCurrentLocation()));
    Q_EMIT repaintNeeded();
    m_showHomeButton = false;
    Q_EMIT settingsChanged(nameId());
}

void NavigationFloatItem::activateHomeButton()
{
    if (!isInitialized())
    {
        return;
    }

    QIcon icon;
    icon.addPixmap(pixmap(QLatin1String("marble/navigation/navigational_homebutton")), QIcon::Normal);
    icon.addPixmap(pixmap(QLatin1String("marble/navigation/navigational_homebutton_hover")), QIcon::Active);
    icon.addPixmap(pixmap(QLatin1String("marble/navigation/navigational_homebutton_press")), QIcon::Selected);
    m_navigationWidget->homeButton->setProperty("icon", QVariant(icon));

    if (m_contextMenu)
    {
        m_activateCurrentPositionButtonAction->setVisible(true);
        m_activateHomeButtonAction->setVisible(false);
    }

    disconnect(m_navigationWidget->homeButton, SIGNAL(clicked()), this, SLOT(centerOnCurrentLocation()));

    if (m_marbleWidget)
    {
        connect(m_navigationWidget->homeButton, SIGNAL(clicked()), m_marbleWidget, SLOT(goHome()));
    }

    Q_EMIT repaintNeeded();
    m_showHomeButton = true;
    Q_EMIT settingsChanged(nameId());
}

void NavigationFloatItem::centerOnCurrentLocation()
{
}

QHash<QString, QVariant> NavigationFloatItem::settings() const
{
    QHash<QString, QVariant> settings = AbstractFloatItem::settings();
    settings.insert(QStringLiteral("showHomeButton"), m_showHomeButton);
    return settings;
}

void NavigationFloatItem::setSettings(const QHash<QString, QVariant>& settings)
{
    AbstractFloatItem::setSettings(settings);
    m_showHomeButton = settings.value(QStringLiteral("showHomeButton"), true).toBool();

    if (m_showHomeButton)
    {
        activateHomeButton();
    }

    else
    {
        activateCurrentPositionButton();
    }
}

} // namespace Marble

#include "moc_NavigationFloatItem.cpp"
