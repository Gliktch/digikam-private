/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-30
 * Description : translate metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "translate.h"

// Qt includes

#include <QGridLayout>
#include <QCheckBox>
#include <QWidget>
#include <QLabel>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dfileoperations.h"
#include "localizesettings.h"
#include "localizeselector.h"
#include "altlangstredit.h"

namespace DigikamBqmTranslatePlugin
{

class Q_DECL_HIDDEN Translate::Private
{

public:

    enum Entries
    {
        Title       = 0x01,
        Caption     = 0x02,
        Copyrights  = 0x04,
        UsageTerms  = 0x08
    };

public:

    Private() = default;

public:

    QCheckBox*            titleCB           = nullptr;
    QCheckBox*            captionCB         = nullptr;
    QCheckBox*            copyrightsCB      = nullptr;
    QCheckBox*            usageTermsCB      = nullptr;

    QLabel*               trLabel           = nullptr;
    QLabel*               tagsLabel         = nullptr;

    LocalizeSelectorList* trSelectorList    = nullptr;

    bool                  changeSettings    = true;
};

Translate::Translate(QObject* const parent)
    : BatchTool(QLatin1String("Translate"), MetadataTool, parent),
      d        (new Private)
{
}

Translate::~Translate()
{
    delete d;
}

BatchTool* Translate::clone(QObject* const parent) const
{
    return new Translate(parent);
}

void Translate::registerSettingsWidget()
{
    QWidget* const panel     = new QWidget;
    QGridLayout* const grid  = new QGridLayout(panel);

    d->tagsLabel             = new QLabel(panel);
    d->titleCB               = new QCheckBox(i18nc("@option:check metadata entry", "Title"),       panel);
    d->captionCB             = new QCheckBox(i18nc("@option:check metadata entry", "Caption"),     panel);
    d->copyrightsCB          = new QCheckBox(i18nc("@option:check metadata entry", "Copyrights"),  panel);
    d->usageTermsCB          = new QCheckBox(i18nc("@option:check metadata entry", "Usage Terms"), panel);

    d->trSelectorList        = new LocalizeSelectorList(panel);
    d->trSelectorList->setTitle(i18nc("@label", "Translate to:"));

    grid->addWidget(d->tagsLabel,      0, 0, 1, 2);
    grid->addWidget(d->titleCB,        1, 0, 1, 2);
    grid->addWidget(d->captionCB,      2, 0, 1, 2);
    grid->addWidget(d->copyrightsCB,   3, 0, 1, 2);
    grid->addWidget(d->usageTermsCB,   4, 0, 1, 2);
    grid->addWidget(d->trSelectorList, 5, 0, 1, 2);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(6, 10);

    slotLocalizeChanged();

    m_settingsWidget = panel;

    connect(d->titleCB, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->captionCB, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->copyrightsCB, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->usageTermsCB, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->trSelectorList, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(LocalizeSettings::instance(), &LocalizeSettings::signalSettingsChanged,
            this, &Translate::slotLocalizeChanged);

    BatchTool::registerSettingsWidget();
}

void Translate::slotLocalizeChanged()
{
    d->tagsLabel->setText(i18nc("@label", "Entries to Translate with %1:",
                          DOnlineTranslator::engineName(LocalizeSettings::instance()->settings().translatorEngine)));
}

BatchToolSettings Translate::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("Title"),      false);
    settings.insert(QLatin1String("Caption"),    false);
    settings.insert(QLatin1String("Copyrights"), false);
    settings.insert(QLatin1String("UsageTerms"), false);
    settings.insert(QLatin1String("TrLangs"),    QStringList() << QLatin1String("en-US"));

    return settings;
}

void Translate::slotAssignSettings2Widget()
{
    d->changeSettings = false;

    d->titleCB->setChecked(settings().value(QLatin1String("Title")).toBool());
    d->captionCB->setChecked(settings().value(QLatin1String("Caption")).toBool());
    d->copyrightsCB->setChecked(settings().value(QLatin1String("Copyrights")).toBool());
    d->usageTermsCB->setChecked(settings().value(QLatin1String("UsageTerms")).toBool());

    QStringList langs = settings().value(QLatin1String("TrLangs")).toStringList();

    d->trSelectorList->clearLanguages();

    for (const QString& lg : std::as_const(langs))
    {
        d->trSelectorList->addLanguage(lg);
    }

    d->changeSettings = true;
}

void Translate::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("Title"),       d->titleCB->isChecked());
        settings.insert(QLatin1String("Caption"),     d->captionCB->isChecked());
        settings.insert(QLatin1String("Copyrights"),  d->copyrightsCB->isChecked());
        settings.insert(QLatin1String("UsageTerms"),  d->usageTermsCB->isChecked());

        QStringList langs = d->trSelectorList->languagesList();
        settings.insert(QLatin1String("TrLangs"),     langs);

        BatchTool::slotSettingsChanged(settings);
    }
}

bool Translate::toolOperations()
{
    bool ret = true;
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

    bool titleStage      = settings().value(QLatin1String("Title")).toBool();
    bool captionStage    = settings().value(QLatin1String("Caption")).toBool();
    bool copyrightsStage = settings().value(QLatin1String("Copyrights")).toBool();
    bool usageTermsStage = settings().value(QLatin1String("UsageTerms")).toBool();
    QStringList langs    = settings().value(QLatin1String("TrLangs")).toStringList();

    for (const QString& trLang : std::as_const(langs))
    {
        if (titleStage)
        {
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Translate Title";
            ret &= insertTranslation(Private::Title, trLang, meta.data());
        }

        if (captionStage)
        {
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Translate Caption";
            ret &= insertTranslation(Private::Caption, trLang, meta.data());
        }

        if (copyrightsStage)
        {
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Translate Copyrights";
            ret &= insertTranslation(Private::Copyrights, trLang, meta.data());
        }

        if (usageTermsStage)
        {
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Translate Usage Terms";
            ret &= insertTranslation(Private::UsageTerms, trLang, meta.data());
        }

        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Title     :" << meta->getXmpTagStringListLangAlt("Xmp.dc.title", false);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Caption   :" << meta->getXmpTagStringListLangAlt("Xmp.dc.description", false);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "CopyRights:" << meta->getXmpTagStringListLangAlt("Xmp.dc.rights", false);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "UsageTerms:" << meta->getXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", false);

        if (image().isNull())
        {
            QFile::remove(outputUrl().toLocalFile());
            ret &= DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

            if (ret && (titleStage || captionStage || copyrightsStage || usageTermsStage))
            {
                ret &= meta->save(outputUrl().toLocalFile());
                qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Save metadata to file:" << ret;
            }
        }
        else
        {
            if (titleStage || captionStage || copyrightsStage || usageTermsStage)
            {
                qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Save metadata to image";
                image().setMetadata(meta->data());
            }

            ret &= savefromDImg();
        }
    }

    return ret;
}

bool Translate::insertTranslation(int entry, const QString& trLang, DMetadata* const meta) const
{
    bool ret = true;
    CaptionsMap captions;
    DMetadata::AltLangMap map;

    switch (entry)
    {
        case Private::Title:
        {
            captions = meta->getItemTitles();
            map      = captions.toAltLangMap();
            break;
        }

        case Private::Caption:
        {
            captions = meta->getItemComments();
            map      = captions.toAltLangMap();
            break;
        }

        case Private::Copyrights:
        {
            map = meta->getXmpTagStringListLangAlt("Xmp.dc.rights", false);
            break;
        }

        case Private::UsageTerms:
        {
            map = meta->getXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", false);
            break;
        }
    }

    if (!map.isEmpty() && map.contains(QLatin1String("x-default")))
    {
        QString sc = map.value(QLatin1String("x-default"));

        if (!sc.isEmpty())
        {
            QString trOut;
            QString error;
            ret = s_inlineTranslateString(sc, trLang, trOut, error);

            if (ret)
            {
                switch (entry)
                {
                    case Private::Title:
                    {
                        CaptionValues val = captions[QLatin1String("x-default")];
                        val.caption       = trOut;
                        captions[trLang]  = val;
                        meta->setItemTitles(captions);
                        break;
                    }

                    case Private::Caption:
                    {
                        CaptionValues val = captions[QLatin1String("x-default")];
                        val.caption       = trOut;
                        captions[trLang]  = val;
                        meta->setItemComments(captions);
                        break;
                    }

                    case Private::Copyrights:
                    {
                        map[trLang] = trOut;
                        meta->setXmpTagStringListLangAlt("Xmp.dc.rights", map);
                        break;
                    }

                    case Private::UsageTerms:
                    {
                        map[trLang] = trOut;
                        meta->setXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", map);
                        break;
                    }
                }
            }
            else
            {
                qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Cannot process online translation from"
                                                 << meta->getFilePath()
                                                 << " - Error:" << error;
            }
        }
        else
        {
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "No x-default language found from" << meta->getFilePath();
        }
    }
    else
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "No alternative language string found from" << meta->getFilePath();
    }

    return ret;
}

} // namespace DigikamBqmTranslatePlugin

#include "moc_translate.cpp"
