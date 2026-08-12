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

#include "AbstractFloatItem.h"

// Qt includes

#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QDialog>
#include <QHelpEvent>
#include <QPen>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "DialogConfigurationInterface.h"
#include "GeoPainter.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN AbstractFloatItemPrivate
{
public:

    AbstractFloatItemPrivate() = default;

    ~AbstractFloatItemPrivate()
    {
        delete m_contextMenu;
    }

public:

    static QPen  s_pen;
    static QFont s_font;

    QMenu*       m_contextMenu = nullptr;

private:

    AbstractFloatItemPrivate(const AbstractFloatItemPrivate&)            = delete;
    AbstractFloatItemPrivate& operator=(const AbstractFloatItemPrivate&) = delete;
};

QPen  AbstractFloatItemPrivate::s_pen  = QPen(Qt::black);

#ifdef Q_OS_MACOS

QFont AbstractFloatItemPrivate::s_font = QFont(QStringLiteral("Sans Serif"), 10);

#else

QFont AbstractFloatItemPrivate::s_font = QFont(QStringLiteral("Sans Serif"), 8);

#endif

AbstractFloatItem::AbstractFloatItem(const MarbleModel* marbleModel, const QPointF& point, const QSizeF& size)
    : RenderPlugin     (marbleModel),
      FrameGraphicsItem(),
      d                (new AbstractFloatItemPrivate())
{
    setCacheMode    (ItemCoordinateCache);
    setFrame        (RectFrame);
    setPadding      (4.0);
    setContentSize  (size);
    setPosition     (point);
}

AbstractFloatItem::~AbstractFloatItem()
{
    delete d;
}

QHash<QString, QVariant> AbstractFloatItem::settings() const
{
    QHash<QString, QVariant> updated = RenderPlugin::settings();

#ifdef Q_OS_MACOS

    updated.insert(QStringLiteral("position"), position().toPoint());

#else

    updated.insert(QStringLiteral("position"), position());

#endif

    return updated;
}

void AbstractFloatItem::setSettings(const QHash<QString, QVariant>& settings)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (settings.value(QStringLiteral("position")).metaType().id() == QMetaType::QString)

#else

    if (settings.value(QStringLiteral("position")).type() == QVariant::String)

#endif

    {

#ifdef Q_OS_MACOS

        setPosition(settings.value(QStringLiteral("position"), position()).toPointF());

#else

        // work around KConfig turning QPointFs into QStrings

        const QStringList coordinates = settings.value(QStringLiteral("position"))
                                                       .toString().split(QLatin1Char(','));
        setPosition(QPointF(coordinates.at(0).toFloat(), coordinates.at(1).toFloat()));

#endif

    }
    else
    {
        setPosition(settings.value(QStringLiteral("position"), position()).toPointF());
    }

    RenderPlugin::setSettings(settings);
}

RenderPlugin::RenderType AbstractFloatItem::renderType() const
{
    return RenderPlugin::PanelRenderType;
}

QPen AbstractFloatItem::pen() const
{
    return d->s_pen;
}

void AbstractFloatItem::setPen(const QPen& pen)
{
    d->s_pen = pen;
    update();
}

QFont AbstractFloatItem::font() const
{
    return d->s_font;
}

void AbstractFloatItem::setFont(const QFont& font)
{
    d->s_font = font;
    update();
}

QString AbstractFloatItem::renderPolicy() const
{
    return QStringLiteral("ALWAYS");
}

QStringList AbstractFloatItem::renderPosition() const
{
    return QStringList(QStringLiteral("FLOAT_ITEM"));
}

// cppcheck-suppress duplInheritedMember
void AbstractFloatItem::setVisible(bool visible)
{
    // Reimplemented since AbstractFloatItem does multiple inheritance
    // and the (set)Visible() methods are available in both base classes!

    RenderPlugin::setVisible(visible);
}

// cppcheck-suppress duplInheritedMember
bool AbstractFloatItem::visible() const
{
    // Reimplemented since AbstractFloatItem does multiple inheritance
    // and the (set)Visible() methods are available in both base classes!

    return RenderPlugin::visible();
}

void AbstractFloatItem::setPositionLocked(bool lock)
{
    ScreenGraphicsItem::GraphicsItemFlags flags = this->flags();

    if (lock)
    {
        flags &= ~ScreenGraphicsItem::ItemIsMovable;
    }
    else
    {
        flags |= ScreenGraphicsItem::ItemIsMovable;
    }

    setFlags(flags);
}

bool AbstractFloatItem::positionLocked() const
{
    return ((flags() & ScreenGraphicsItem::ItemIsMovable) == 0);
}

bool AbstractFloatItem::eventFilter(QObject* object, QEvent* e)
{
    if (!enabled() || !visible())
    {
        return false;
    }

    if      (e->type() == QEvent::ContextMenu)
    {
        QWidget* const widget              = qobject_cast<QWidget*>(object);
        QContextMenuEvent* const menuEvent = dynamic_cast<QContextMenuEvent*>(e);

        if ((widget != nullptr) && (menuEvent != nullptr && contains(menuEvent->pos())))
        {
            contextMenuEvent(widget, menuEvent);

            return true;
        }

        return false;
    }
    else if (e->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = dynamic_cast<QHelpEvent*>(e);

        if ((helpEvent != nullptr) && contains(helpEvent->pos()))
        {
            toolTipEvent(helpEvent);

            return true;
        }

        return false;
    }
    else
    {
        return ScreenGraphicsItem::eventFilter(object, e);
    }
}

void AbstractFloatItem::contextMenuEvent(QWidget* w, QContextMenuEvent* e)
{
    contextMenu()->exec(w->mapToGlobal(e->pos()));
}

void AbstractFloatItem::toolTipEvent(QHelpEvent* e)
{
    Q_UNUSED(e);
}

bool AbstractFloatItem::render(GeoPainter* painter, ViewportParams* viewport,
                               const QString& renderPos, GeoSceneLayer* layer)
{
    Q_UNUSED(painter)
    Q_UNUSED(viewport)
    Q_UNUSED(renderPos)
    Q_UNUSED(layer)

    return true;
}

// cppcheck-suppress duplInheritedMember
void AbstractFloatItem::show()
{
    setVisible(true);
}

// cppcheck-suppress duplInheritedMember
void AbstractFloatItem::hide()
{
    setVisible(false);
}

QMenu* AbstractFloatItem::contextMenu()
{
    if (!d->m_contextMenu)
    {
        d->m_contextMenu    = new QMenu;
        QAction* const lockAction = d->m_contextMenu->addAction(QIcon::fromTheme(QStringLiteral("unlock")),
                                                                i18n("&Lock"));
        lockAction->setCheckable(true);
        lockAction->setChecked(positionLocked());

        connect(lockAction, SIGNAL(triggered(bool)),
                this, SLOT(setPositionLocked(bool)));

        if (!(flags() & ItemIsHideable))
        {
            QAction* const hideAction = d->m_contextMenu->addAction(i18n("&Hide"));

            connect(hideAction, SIGNAL(triggered()),
                    this, SLOT(hide()));
        }

        DialogConfigurationInterface* const configInterface = qobject_cast<DialogConfigurationInterface*>(this);
        QDialog* const dialog                               = configInterface ? configInterface->configDialog()
                                                                              : nullptr;

        if (dialog)
        {
            d->m_contextMenu->addSeparator();
            QAction* const configAction = d->m_contextMenu->addAction(QIcon::fromTheme(QLatin1String("configure")),
                                                                      i18n("&Configure..."));

            connect(configAction, SIGNAL(triggered()),
                    dialog, SLOT(exec()));
        }
    }

    Q_ASSERT(d->m_contextMenu);

    return d->m_contextMenu;
}

} // namespace Marble

#include "moc_AbstractFloatItem.cpp"
