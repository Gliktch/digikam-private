/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : colors theme manager
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thememanager_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ThemeManagerCreator
{
public:

    ThemeManager object;
};

Q_GLOBAL_STATIC(ThemeManagerCreator, creator)

// -----------------------------------------------------

ThemeManager::ThemeManager()
    : QObject(),
      d      (new Private)
{
}

ThemeManager::~ThemeManager()
{
    delete d;
}

ThemeManager* ThemeManager::instance()
{
    return &creator->object;
}

QString ThemeManager::defaultThemeName() const
{
    return d->defaultThemeName;
}

QString ThemeManager::currentThemeName() const
{
    if (!d->themeMenuAction || !d->themeMenuActionGroup)
    {
        return defaultThemeName();
    }

    const QAction* const action = d->themeMenuActionGroup->checkedAction();

    return (!action ? defaultThemeName()
                    : action->text().remove(QLatin1Char('&')));
}

void ThemeManager::setCurrentTheme(const QString& name)
{
    if (!d->themeMenuAction || !d->themeMenuActionGroup)
    {
        return;
    }

    QList<QAction*> list = d->themeMenuActionGroup->actions();

    for (QAction* const action : std::as_const(list))
    {
        if (action->text().remove(QLatin1Char('&')) == name)
        {
            action->setChecked(true);
            slotChangePalette();
        }
    }
}

void ThemeManager::slotChangePalette()
{
    updateCurrentDesktopDefaultThemePreview();

    QString theme(currentThemeName());

    if (
        theme.isEmpty()                                                                              ||
        (theme == defaultThemeName())                                                                ||
        (qApp->style()->objectName().compare(QLatin1String("windowsvista"), Qt::CaseInsensitive) == 0)
       )
    {
        theme = currentDesktopdefaultTheme();
    }

    QString filePath        = d->themeMap.value(theme);
    KSharedConfigPtr config = KSharedConfig::openConfig(filePath);

    // hint for the style to synchronize the color scheme with the window manager/compositor

    qApp->setProperty("KDE_COLOR_SCHEME_PATH", filePath);
    qApp->setPalette(SchemeManager::createApplicationPalette(config));
    qApp->style()->polish(qApp);

    qCDebug(DIGIKAM_WIDGETS_LOG) << theme << " :: " << filePath;

    Q_EMIT signalThemeChanged();
}

void ThemeManager::setThemeMenuAction(QMenu* const action)
{
    d->themeMenuAction = action;
    populateThemeMenu();
}

void ThemeManager::registerThemeActions(DXmlGuiWindow* const win)
{
    if (!win)
    {
        return;
    }

    if (!d->themeMenuAction)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Cannot register theme actions to " << win->windowTitle();
        return;
    }

    win->actionCollection()->addAction(QLatin1String("theme_menu"), d->themeMenuAction->menuAction());
}

void ThemeManager::populateThemeMenu()
{
    if (!d->themeMenuAction)
    {
        return;
    }

    QString theme(currentThemeName());

    d->themeMap.clear();
    d->themeMenuAction->clear();
    delete d->themeMenuActionGroup;

    d->themeMenuActionGroup = new QActionGroup(d->themeMenuAction);

    QAction* const action   = new QAction(defaultThemeName(), d->themeMenuActionGroup);
    action->setCheckable(true);
    action->setChecked(true);
    d->themeMenuAction->addAction(action);

    if (qApp->style()->objectName().compare(QLatin1String("windowsvista"), Qt::CaseInsensitive) != 0)
    {
        QMap<QString, QAction*> actionMap;
        QStringList dirs;

        // digiKam colors scheme

        dirs << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                          QLatin1String("digikam/colorschemes"),
                                          QStandardPaths::LocateDirectory);

        qCDebug(DIGIKAM_WIDGETS_LOG) << "Paths to color scheme : " << dirs;

        for (const QString& dir : std::as_const(dirs))
        {
            QDirIterator it(dir, QStringList() << QLatin1String("*.colors"));

            while (it.hasNext())
            {
                const QString filePath  = it.next();
                KSharedConfigPtr config = KSharedConfig::openConfig(filePath);
                QIcon icon              = d->createSchemePreviewIcon(config);
                KConfigGroup group(config, QLatin1String("General"));
                const QString name      = group.readEntry("Name",
                                                          it.fileInfo().baseName());
                QAction* const ac       = new QAction(name, d->themeMenuActionGroup);
                d->themeMap.insert(name, filePath);
                ac->setIcon(icon);
                ac->setCheckable(true);
                actionMap.insert(name, ac);
            }
        }

        const auto amap = actionMap.values();

        for (QAction* const menuAction : amap)
        {
            d->themeMenuAction->addAction(menuAction);
        }
    }

    connect(d->themeMenuActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotChangePalette()));

    updateCurrentDesktopDefaultThemePreview();
    setCurrentTheme(theme);
}

void ThemeManager::updateCurrentDesktopDefaultThemePreview()
{
    if (!d->themeMenuActionGroup)
    {
        return;
    }

    QList<QAction*> list = d->themeMenuActionGroup->actions();

    for (QAction* const action : std::as_const(list))
    {
        if (action->text().remove(QLatin1Char('&')) == defaultThemeName())
        {
            KSharedConfigPtr config = KSharedConfig::openConfig(d->themeMap.value(currentDesktopdefaultTheme()));
            QIcon icon              = d->createSchemePreviewIcon(config);
            action->setIcon(icon);
        }
    }
}

QString ThemeManager::currentDesktopdefaultTheme() const
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("kdeglobals"));
    KConfigGroup group(config, QLatin1String("General"));

    return group.readEntry("ColorScheme");
}

void ThemeManager::updateThemeMenu()
{
    populateThemeMenu();
    slotChangePalette();
}

} // namespace Digikam

#include "moc_thememanager.cpp"
