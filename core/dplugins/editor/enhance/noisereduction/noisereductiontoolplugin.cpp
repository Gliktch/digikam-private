/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce noise
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "noisereductiontoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "noisereductiontool.h"

namespace DigikamEditorNoiseReductionToolPlugin
{

NoiseReductionToolPlugin::NoiseReductionToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString NoiseReductionToolPlugin::name() const
{
    return i18nc("@title", "Noise Reduction");
}

QString NoiseReductionToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon NoiseReductionToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("noisereduction"));
}

QString NoiseReductionToolPlugin::description() const
{
    return i18nc("@info", "A tool to remove photograph noise using wavelets");
}

QString NoiseReductionToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can reduce noise in image.");
}

QString NoiseReductionToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString NoiseReductionToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString NoiseReductionToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-nr");
}

QList<DPluginAuthor> NoiseReductionToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void NoiseReductionToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Noise Reduction..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_noisereduction"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotNoiseReduction()));

    addAction(ac);
}

void NoiseReductionToolPlugin::slotNoiseReduction()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        NoiseReductionTool* const tool = new NoiseReductionTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorNoiseReductionToolPlugin

#include "moc_noisereductiontoolplugin.cpp"
