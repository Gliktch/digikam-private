/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to generate HTML image galleries.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "htmlgalleryplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "htmlwizard.h"

namespace DigikamGenericHtmlGalleryPlugin
{

HtmlGalleryPlugin::HtmlGalleryPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void HtmlGalleryPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString HtmlGalleryPlugin::name() const
{
    return i18n("Html Gallery");
}

QString HtmlGalleryPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HtmlGalleryPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-html"));
}

QString HtmlGalleryPlugin::description() const
{
    return i18n("A tool to generate HTML gallery from images");
}

QString HtmlGalleryPlugin::details() const
{
    return i18n("<p>This tool allows users to back-process items (as resize) before to create W3C compliant html gallery.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Themable HTML template with different layout can be used to assemble files on a gallery.</p>");
}

QString HtmlGalleryPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString HtmlGalleryPlugin::handbookChapter() const
{
    return QLatin1String("html_gallery");
}

QList<DPluginAuthor> HtmlGalleryPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Aurelien Gateau"),
                             QString::fromUtf8("agateau at kde dot org"),
                             QString::fromUtf8("(C) 2006-2009"),
                             i18n("Former Author and Maintainer"))
            << DPluginAuthor(QString::fromUtf8("Gianluca Urgese"),
                             QString::fromUtf8("giasone dot 82 at gmail dot com"),
                             QString::fromUtf8("(C) 2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2026"),
                             i18n("Developer and Maintainer"))
            ;
}

void HtmlGalleryPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create Html gallery..."));
    ac->setObjectName(QLatin1String("htmlgallery"));
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_H);
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotHtmlGallery()));

    addAction(ac);
}

void HtmlGalleryPlugin::slotHtmlGallery()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new HTMLWizard(nullptr, infoIface(sender()));
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmlgalleryplugin.cpp"
