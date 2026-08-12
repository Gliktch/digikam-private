/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-10-03
 * Description : auto assign tags batch tool.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2023      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assigntags.h"

// C++ includes

#include <memory>

// Qt includes

#include <QComboBox>
#include <QWidget>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dimg.h"
#include "dnuminput.h"
#include "dmetadata.h"
#include "dpluginbqm.h"
#include "dlayoutbox.h"
#include "dfileoperations.h"
#include "localizeselector.h"
#include "previewloadthread.h"
#include "autotagsscansettings.h"
#include "autotagsscanwidget.h"
#include "autotagspipelineobject.h"

namespace DigikamBqmAssignTagsPlugin
{

class Q_DECL_HIDDEN AssignTags::Private
{
public:

    Private() = default;
    ~Private()
    {
        if (pipeline)
        {
            pipeline->cancel();
            delete pipeline;
            pipeline = nullptr;
        }
    }

public:

    bool                    changeSettings      = true;
    AutotagsPipelineObject* pipeline            = nullptr;
    AutotagsScanWidget*     autotagsScanWidget  = nullptr;
};

AssignTags::AssignTags(QObject* const parent)
    : BatchTool(QLatin1String("AssignTags"), MetadataTool, parent),
      d        (new Private)
{
}

AssignTags::~AssignTags()
{
    delete d;
}

BatchTool* AssignTags::clone(QObject* const parent) const
{
    return new AssignTags(parent);
}

void AssignTags::registerSettingsWidget()
{
    DVBox* const vbox     = new DVBox;
    d->autotagsScanWidget = new AutotagsScanWidget(AutotagsScanWidget::SettingsDisplayMode::BQM, vbox);

    m_settingsWidget      = vbox;

    connect(d->autotagsScanWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AssignTags::defaultSettings()
{
    BatchToolSettings settings;
    AutotagsScanSettings autotagsSettings;

    settings.insert(QLatin1String("AutotagsTagMode"), autotagsSettings.tagMode);
    settings.insert(QLatin1String("AutotagsObjectDetectModel"), autotagsSettings.objectDetectModel);
    settings.insert(QLatin1String("AutotagsObjectDetectAccuracy"), autotagsSettings.uiConfidenceThreshold);
    settings.insert(QLatin1String("TrAutotagsLangs"), autotagsSettings.languages);

    return settings;
}

void AssignTags::slotAssignSettings2Widget()
{
    AutotagsScanSettings autotagsSettings;
    autotagsSettings.objectDetectModel      = settings().value(QLatin1String("AutotagsObjectDetectModel")).toString();
    autotagsSettings.tagMode                = (AutotagsScanSettings::TagMode)settings().value(QLatin1String("AutotagsTagMode")).toInt();
    autotagsSettings.uiConfidenceThreshold  = settings().value(QLatin1String("AutotagsObjectDetectAccuracy")).toInt();
    autotagsSettings.languages              = settings().value(QLatin1String("TrAutotagsLangs")).toStringList();
    autotagsSettings.bqmMode                = true;

    d->autotagsScanWidget->setSettings(autotagsSettings);
}

void AssignTags::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        AutotagsScanSettings autotagsSettings = d->autotagsScanWidget->settings();

        settings.insert(QLatin1String("AutotagsTagMode"), autotagsSettings.tagMode);
        settings.insert(QLatin1String("AutotagsObjectDetectModel"), autotagsSettings.objectDetectModel);
        settings.insert(QLatin1String("AutotagsObjectDetectAccuracy"), autotagsSettings.uiConfidenceThreshold);
        settings.insert(QLatin1String("TrAutotagsLangs"), autotagsSettings.languages);

        BatchTool::slotSettingsChanged(settings);
    }
}

bool AssignTags::toolOperations()
{

    if (nullptr == d->pipeline)
    {
        AutotagsScanSettings pipelineSettings;
        pipelineSettings.objectDetectModel      = settings().value(QLatin1String("AutotagsObjectDetectModel")).toString();
        pipelineSettings.tagMode                = (AutotagsScanSettings::TagMode)settings().value(QLatin1String("AutotagsTagMode")).toInt();
        pipelineSettings.uiConfidenceThreshold  = settings().value(QLatin1String("AutotagsObjectDetectAccuracy")).toInt();
        pipelineSettings.languages              = settings().value(QLatin1String("TrAutotagsLangs")).toStringList();
        pipelineSettings.bqmMode                = true;

        d->pipeline = new AutotagsPipelineObject(pipelineSettings);
        d->pipeline->start();
    }

    bool ret = true;
    std::unique_ptr<DMetadata> meta(new DMetadata);

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (!ret || !meta->load(outputUrl().toLocalFile()))
        {
            return ret;
        }
    }
    else
    {
        ret = savefromDImg();
    }

    if (AutotagsScanSettings::TagMode::Update == (AutotagsScanSettings::TagMode)settings().value(QLatin1String("AutotagsTagMode")).toInt())
    {
        if (image().isNull())
        {
            meta->load(inputUrl().toLocalFile());
        }
        else
        {
            meta->setData(image().getMetadata());
        }
    }

    d->pipeline->bqmSendOne(meta, imageInfo(), outputUrl(), image());

    return ret;
}

} // namespace DigikamBqmAssignTagsPlugin

#include "moc_assigntags.cpp"
