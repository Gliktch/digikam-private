/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to edit items metadata.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadataeditplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "metadataeditdialog.h"

namespace DigikamGenericMetadataEditPlugin
{

MetadataEditPlugin::MetadataEditPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void MetadataEditPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString MetadataEditPlugin::name() const
{
    return i18nc("@title", "Metadata Edit");
}

QString MetadataEditPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon MetadataEditPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("format-text-code"));
}

QString MetadataEditPlugin::description() const
{
    return i18nc("@info", "A tool to edit items metadata");
}

QString MetadataEditPlugin::details() const
{
    return i18nc("@info", "This tool allows users to changes plenty of metadata from items.<br><br>"
                 "Most common Exif, Iptc, and Xmp tags used in photography are listed for editing with standardized values.<br><br>"
                 "For photo agencies, pre-configured subjects can be used to describe the items contents based on Iptc reference codes.");
}

QString MetadataEditPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString MetadataEditPlugin::handbookChapter() const
{
    return QLatin1String("metadata_editor");
}

QList<DPluginAuthor> MetadataEditPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Victor Dodon"),
                             QString::fromUtf8("victor dot dodon at cti dot pub dot ro"),
                             QString::fromUtf8("(C) 2010-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2025"))
            << DPluginAuthor(QString::fromUtf8("Alan Pater"),
                             QString::fromUtf8("alan dot pater at gmail dot com"),
                             QString::fromUtf8("(C) 2014"))
            << DPluginAuthor(QString::fromUtf8("Andi Clemens"),
                             QString::fromUtf8("andi dot clemens at googlemail dot com"),
                             QString::fromUtf8("(C) 2009"))
            ;
}

void MetadataEditPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Edit Metadata..."));
    ac->setObjectName(QLatin1String("metadata_edit"));
    ac->setActionCategory(DPluginAction::GenericMetadata);
    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_M);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotEditMetadata()));

    addAction(ac);
}

void MetadataEditPlugin::slotEditMetadata()
{
    DInfoInterface* const iface = infoIface(sender());

    if (!iface)
    {
        return;
    }

    QList<QUrl> urls = iface->currentSelectedItems();

    if (urls.isEmpty())
    {
        return;
    }

    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new MetadataEditDialog(nullptr, iface);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_metadataeditplugin.cpp"
