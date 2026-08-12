/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-08
 * Description : a Raw Import plugin based on LibRaw.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawimportnativeplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "rawimport.h"
#include "editorcore.h"
#include "editortooliface.h"

namespace DigikamRawImportNativePlugin
{

RawImportNativePlugin::RawImportNativePlugin(QObject* const parent)
    : DPluginRawImport(parent)
{
}

QString RawImportNativePlugin::name() const
{
    return i18n("Import Raw Using Libraw");
}

QString RawImportNativePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RawImportNativePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString RawImportNativePlugin::description() const
{
    return i18n("A tool to import Raw images using Libraw engine");
}

QString RawImportNativePlugin::details() const
{
    return i18n("<p>This Image Editor tool import Raw images using Libraw engine as decoder.</p>"
                "<p>This is the <b>native</b> Raw Import tool included in core application. "
                "It does not require any external engine to work.</p>"
                "<p>See Libraw web site for details: <a href='https://www.libraw.org/'>https://www.libraw.org/</a></p>");
}

QString RawImportNativePlugin::handbookSection() const
{
    return QLatin1String("setup_application");
}

QString RawImportNativePlugin::handbookChapter() const
{
    return QLatin1String("editor_settings");
}

QString RawImportNativePlugin::handbookReference() const
{
    return QLatin1String("setup-raw");
}

QList<DPluginAuthor> RawImportNativePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2025"))
            ;
}

void RawImportNativePlugin::setup(QObject* const)
{
}

bool RawImportNativePlugin::run(const QString& filePath, const DRawDecoding& def)
{
    m_filePath                 = filePath;
    m_defaultSettings          = def;
    EditorCore* const core     = EditorCore::defaultInstance();
    RawImport* const rawImport = new RawImport(QUrl::fromLocalFile(filePath), core);
    rawImport->setProperty("DPluginIId",      iid());
    rawImport->setProperty("DPluginIfaceIId", ifaceIid());
    rawImport->setPlugin(this);

    EditorToolIface::editorToolIface()->loadTool(rawImport);

    connect(rawImport, SIGNAL(okClicked()),
            this, SLOT(slotLoadRawFromTool()));

    connect(rawImport, SIGNAL(cancelClicked()),
            this, SLOT(slotLoadRaw()));

    return true;
}

void RawImportNativePlugin::slotLoadRawFromTool()
{
    const RawImport* const rawImport = dynamic_cast<RawImport*>(EditorToolIface::editorToolIface()->currentTool());

    if (!rawImport)
    {
        return;
    }

    LoadingDescription props(m_filePath, LoadingDescription::ConvertForEditor);
    props.rawDecodingSettings = rawImport->rawDecodingSettings();
    props.rawDecodingHint     = LoadingDescription::RawDecodingCustomSettings;

    if (rawImport->hasPostProcessedImage())
    {
        // Image was previously decoded in Import tool: load pre-decoded image as well in editor.

        Q_EMIT signalDecodedImage(props, rawImport->postProcessedImage());
    }
    else
    {
        // Image was not previously decoded in Import tool: as to editor to post-process image and load it.

        Q_EMIT signalLoadRaw(props);
    }
}

void RawImportNativePlugin::slotLoadRaw()
{
    // Cancel pressed: we load Raw with image editor default Raw decoding settings.

    Q_EMIT signalLoadRaw(LoadingDescription(m_filePath,
                                          m_defaultSettings,
                                          LoadingDescription::RawDecodingGlobalSettings,
                                          LoadingDescription::ConvertForEditor));
}

} // namespace DigikamRawImportNativePlugin

#include "moc_rawimportnativeplugin.cpp"
