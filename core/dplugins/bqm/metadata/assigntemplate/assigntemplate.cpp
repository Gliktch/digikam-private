/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : assign metadata template batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assigntemplate.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "template.h"
#include "templatemanager.h"
#include "dfileoperations.h"

namespace DigikamBqmAssignTemplatePlugin
{

AssignTemplate::AssignTemplate(QObject* const parent)
    : BatchTool(QLatin1String("AssignTemplate"), MetadataTool, parent)
{
}

BatchTool* AssignTemplate::clone(QObject* const parent) const
{
    return new AssignTemplate(parent);
}

void AssignTemplate::registerSettingsWidget()
{
    DVBox* const vbox  = new DVBox;
    m_templateSelector = new TemplateSelector(vbox);
    m_templateViewer   = new TemplateViewer(vbox);
    m_settingsWidget   = vbox;

    connect(m_templateSelector, SIGNAL(signalTemplateSelected()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AssignTemplate::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("TemplateTitle"), QString());
    return settings;
}

void AssignTemplate::slotAssignSettings2Widget()
{
    QString title = settings().value(QLatin1String("TemplateTitle")).toString();

    Template t;

    if      (title == Template::removeTemplateTitle())
    {
        t.setTemplateTitle(Template::removeTemplateTitle());
    }
    else if (title.isEmpty())
    {
        // Nothing to do.
    }
    else
    {
        t = TemplateManager::defaultManager()->findByTitle(title);
    }

    m_templateSelector->setTemplate(t);
    m_templateViewer->setTemplate(t);
}

void AssignTemplate::slotSettingsChanged()
{
    m_templateViewer->setTemplate(m_templateSelector->getTemplate());
    BatchToolSettings settings;
    settings.insert(QLatin1String("TemplateTitle"), m_templateSelector->getTemplate().templateTitle());
    BatchTool::slotSettingsChanged(settings);
}

bool AssignTemplate::toolOperations()
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (image().isNull())
    {
        if (!meta->load(inputUrl().toLocalFile()))
        {
            return false;
        }
    }
    else
    {
        meta->setData(image().getMetadata());
    }

    QString title = settings().value(QLatin1String("TemplateTitle")).toString();

    if      (title == Template::removeTemplateTitle())
    {
        meta->removeMetadataTemplate();
    }
    else if (!title.isEmpty())
    {
        Template t = meta->getMetadataTemplate();
        t.merge(TemplateManager::defaultManager()->findByTitle(title));

        meta->setMetadataTemplate(t);
    }

    bool ret = true;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (ret && !title.isEmpty())
        {
            ret = meta->save(outputUrl().toLocalFile());
        }
    }
    else
    {
        if (!title.isEmpty())
        {
            image().setMetadata(meta->data());
        }

        ret = savefromDImg();
    }

    return ret;
}

} // namespace DigikamBqmAssignTemplatePlugin

#include "moc_assigntemplate.cpp"
