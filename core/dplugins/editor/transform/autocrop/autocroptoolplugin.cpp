/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to auto-crop an image.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autocroptoolplugin.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"
#include "autocrop.h"

namespace DigikamEditorAutoCropToolPlugin
{

AutoCropToolPlugin::AutoCropToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString AutoCropToolPlugin::name() const
{
    return i18nc("@title", "Auto-Crop");
}

QString AutoCropToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AutoCropToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-crop"));
}

QString AutoCropToolPlugin::description() const
{
    return i18nc("@info", "A tool to auto-crop an image");
}

QString AutoCropToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can crop automatically an image by detection of inner black border, "
                          "generated while panorama stitching for example.");
}

QString AutoCropToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString AutoCropToolPlugin::handbookChapter() const
{
    return QLatin1String("transform_tools");
}

QString AutoCropToolPlugin::handbookReference() const
{
    return QLatin1String("transform-autocrop");
}

QList<DPluginAuthor> AutoCropToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Sayantan Datta"),
                             QString::fromUtf8("sayantan dot knz at gmail dot com"),
                             QString::fromUtf8("(C) 2013"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2013-2024"))
            ;
}

void AutoCropToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "&Auto-Crop"));
    ac->setObjectName(QLatin1String("editorwindow_transform_autocrop"));
    ac->setWhatsThis(i18nc("@info", "This option can be used to crop automatically the image."));
    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_X);
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCrop()));

    addAction(ac);
}

void AutoCropToolPlugin::slotAutoCrop()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ImageIface iface;
    AutoCrop ac(iface.original());
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();
    iface.crop(rect);

    QApplication::restoreOverrideCursor();
}

} // namespace DigikamEditorAutoCropToolPlugin

#include "moc_autocroptoolplugin.cpp"
