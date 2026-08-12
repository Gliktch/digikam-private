/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-16
 * Description : Spell-check and localize Settings Container.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localizecontainer.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "localizesettings.h"
#include "digikam_debug.h"

namespace Digikam
{

void LocalizeContainer::readFromConfig(const KConfigGroup& group)
{
    enableSpellCheck = group.readEntry("EnableSpellCheck", false);
    defaultLanguage  = group.readEntry("DefaultLanguage",  QString());      // empty for auto-detect

    ignoredWords     = group.readEntry("IgnoredWords", QStringList() << QLatin1String("digiKam")
                                                                     << QLatin1String("Showfoto")
                                                                     << QLatin1String("Apple")
                                                                     << QLatin1String("Nikon")
                                                                     << QLatin1String("Android")
                                                                     << QLatin1String("Canon")
                                                                     << QLatin1String("Sony")
                                                                     << QLatin1String("Samsung")
                                                                     << QLatin1String("Panasonic")
                                                                     << QLatin1String("Olympus")
                                                                     << QLatin1String("Exif")
                                                                     << QLatin1String("IPTC")
                                                                     << QLatin1String("Xmp"));
    ignoredWords.removeDuplicates();

    translatorEngine = (DOnlineTranslator::Engine)group.readEntry("TranslatorEngine", (int)DOnlineTranslator::Engine::Google);
    translatorLang   = group.readEntry("TranslatorLang", QStringList() << QLatin1String("en-US")
                                                                       << QLatin1String("fr-FR")
                                                                       << QLatin1String("es-ES")
                                                                       << QLatin1String("it-IT")
                                                                       << QLatin1String("de-DE")
                                                                       << QLatin1String("pt-PT")
    );

    alternativeLang  = group.readEntry("AlternativeLang", QStringList() << QLatin1String("x-default")
                                                                        << QLatin1String("en-US")
                                                                        << QLatin1String("fr-FR")
                                                                        << QLatin1String("es-ES")
                                                                        << QLatin1String("it-IT")
                                                                        << QLatin1String("de-DE")
                                                                        << QLatin1String("pt-PT")
    );
}

void LocalizeContainer::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("EnableSpellCheck", enableSpellCheck);
    group.writeEntry("DefaultLanguage",  defaultLanguage);
    group.writeEntry("IgnoredWords",     ignoredWords);
    group.writeEntry("TranslatorEngine", (int)translatorEngine);
    group.writeEntry("TranslatorLang",   translatorLang);
    group.writeEntry("AlternativeLang",  alternativeLang);
    group.sync();
}

QDebug operator<<(QDebug dbg, const LocalizeContainer& inf)
{
    dbg.nospace() << "[LocalizeContainer] enableSpellCheck("
                  << inf.enableSpellCheck << "), "
                  << "defaultLanguage("
                  << inf.defaultLanguage << "), "
                  << "ignoredWords("
                  << inf.ignoredWords << "), "
                  << "translatorEngine("
                  << inf.translatorEngine << "), "
                  << "translatorLang("
                  << inf.translatorLang << "), "
                  << "alternativeLang("
                  << inf.alternativeLang << "), ";
    return dbg.space();
}

} // namespace Digikam
