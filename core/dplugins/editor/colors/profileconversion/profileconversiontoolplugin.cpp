/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert to color space
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "profileconversiontoolplugin.h"

// Qt includes

#include <QPointer>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "profileconversiontool.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "imageiface.h"

namespace DigikamEditorProfileConversionToolPlugin
{

ProfileConversionToolPlugin::ProfileConversionToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString ProfileConversionToolPlugin::name() const
{
    return i18nc("@title", "Color Profile Conversion");
}

QString ProfileConversionToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ProfileConversionToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-desktop-display-color"));
}

QString ProfileConversionToolPlugin::description() const
{
    return i18nc("@info", "A tool to convert image to a color space");
}

QString ProfileConversionToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can convert image to a different color space.");
}

QString ProfileConversionToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString ProfileConversionToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString ProfileConversionToolPlugin::handbookReference() const
{
    return QLatin1String("color-cm");
}

QList<DPluginAuthor> ProfileConversionToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2009-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void ProfileConversionToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    m_profileMenuAction     = new IccProfilesMenuAction(icon(), QString(), parent);

    connect(m_profileMenuAction, SIGNAL(signalTriggeredProfile(IccProfile)),
            this, SLOT(slotConvertToColorSpace(IccProfile)));

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotUpdateColorSpaceMenu()));

    ac->setMenu(m_profileMenuAction);
    ac->setText(i18nc("@action", "Color Spaces"));
    ac->setObjectName(QLatin1String("editorwindow_colormanagement"));
    ac->setActionCategory(DPluginAction::EditorColors);

    addAction(ac);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Color Space Converter..."));
    ac2->setObjectName(QLatin1String("editorwindow_color_spaceconverter"));
    ac2->setActionCategory(DPluginAction::EditorColors);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotProfileConversionTool()));

    m_colorSpaceConverter = ac2;

    addAction(ac2);

    slotUpdateColorSpaceMenu();
}

void ProfileConversionToolPlugin::slotConvertToColorSpace(const IccProfile& profile)
{
    ImageIface iface;

    if (iface.originalIccProfile().isNull())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18nc("@info", "This image is not color managed."));
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    ProfileConversionTool::fastConversion(profile);
    qApp->restoreOverrideCursor();
}

void ProfileConversionToolPlugin::slotUpdateColorSpaceMenu()
{
    m_profileMenuAction->clear();

    EditorWindow* const editor = dynamic_cast<EditorWindow*>(m_profileMenuAction->parentObject());

    if (!IccSettings::instance()->isEnabled())
    {
        QAction* const action = new QAction(i18nc("@action", "Color Management is disabled..."), this);
        m_profileMenuAction->addAction(action);

        if (editor)
        {
            connect(action, SIGNAL(triggered()),
                    editor, SLOT(slotSetupICC()));
        }
    }
    else
    {
        ICCSettingsContainer settings = IccSettings::instance()->settings();

        QList<IccProfile> standardProfiles;
        QList<IccProfile> favoriteProfiles;
        QSet<QString> standardProfilePaths;

        standardProfiles << IccProfile::sRGB()
                         << IccProfile::adobeRGB()
                         << IccProfile::wideGamutRGB()
                         << IccProfile::proPhotoRGB();

        for (IccProfile profile : std::as_const(standardProfiles))
        {
            m_profileMenuAction->addProfile(profile, profile.description());
            standardProfilePaths << profile.filePath();
        }

        m_profileMenuAction->addSeparator();

        QStringList profileList = ProfileConversionTool::favoriteProfiles();

        QSet<QString> favoriteProfilePaths(profileList.begin(), profileList.end());
        favoriteProfilePaths   -= standardProfilePaths;

        for (const QString& path : std::as_const(favoriteProfilePaths))
        {
            favoriteProfiles << IccProfile(path);
        }

        m_profileMenuAction->addProfiles(favoriteProfiles);
    }

    m_profileMenuAction->addSeparator();
    m_profileMenuAction->addAction(m_colorSpaceConverter);

    if (editor)
    {
        m_colorSpaceConverter->setEnabled(editor->actionEnabledState() &&
                                          IccSettings::instance()->isEnabled());
    }
}

void ProfileConversionToolPlugin::slotProfileConversionTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ProfileConversionTool* const tool = new ProfileConversionTool(this);
        tool->setPlugin(this);

        connect(tool, SIGNAL(okClicked()),
                this, SLOT(slotUpdateColorSpaceMenu()));

        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorProfileConversionToolPlugin

#include "moc_profileconversiontoolplugin.cpp"
