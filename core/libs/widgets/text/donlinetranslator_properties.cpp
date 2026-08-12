/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Properties methods.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Hennadii Chernyshchyk <genaloner at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "donlinetranslator_p.h"

namespace Digikam
{

bool DOnlineTranslator::isSourceTranslitEnabled() const
{
    return d->sourceTranslitEnabled;
}

void DOnlineTranslator::setSourceTranslitEnabled(bool enable)
{
    d->sourceTranslitEnabled = enable;
}

bool DOnlineTranslator::isTranslationTranslitEnabled() const
{
    return d->translationTranslitEnabled;
}

void DOnlineTranslator::setTranslationTranslitEnabled(bool enable)
{
    d->translationTranslitEnabled = enable;
}

bool DOnlineTranslator::isSourceTranscriptionEnabled() const
{
    return d->sourceTranscriptionEnabled;
}

void DOnlineTranslator::setSourceTranscriptionEnabled(bool enable)
{
    d->sourceTranscriptionEnabled = enable;
}

bool DOnlineTranslator::isTranslationOptionsEnabled() const
{
    return d->translationOptionsEnabled;
}

void DOnlineTranslator::setTranslationOptionsEnabled(bool enable)
{
    d->translationOptionsEnabled = enable;
}

void DOnlineTranslator::setEngineUrl(Engine engine, const QString& url)
{
    switch (engine)
    {
        case LibreTranslate:
        {
            d->libreUrl  = url;
            break;
        }

        case Lingva:
        {
            d->lingvaUrl = url;
            break;
        }

        default:
        {
            break;
        }
    }
}

void DOnlineTranslator::setEngineApiKey(Engine engine, const QByteArray& apiKey)
{
    switch (engine)
    {
        case LibreTranslate:
        {
            d->libreApiKey = apiKey;
            break;
        }

        default:
        {
            break;
        }
    }
}

QJsonDocument DOnlineTranslator::toJson() const
{
    QJsonObject translateOptions;

    for (auto it = d->translationOptions.cbegin() ; it != d->translationOptions.cend() ; ++it)
    {
        QJsonArray arr;

        for (const DOnlineTranslatorOption& option : it.value())
        {
            arr.append(option.toJson());
        }

        translateOptions.insert(it.key(), arr);
    }

    QJsonObject object
    {
        { QLatin1String("source"),               d->source                   },
        { QLatin1String("sourceTranscription"),  d->sourceTranscription      },
        { QLatin1String("sourceTranslit"),       d->sourceTranslit           },
        { QLatin1String("translation"),          d->translation              },
        { QLatin1String("translationOptions"),   qMove(translateOptions)     },
        { QLatin1String("translationTranslit"),  d->translationTranslit      },
    };

    return QJsonDocument(object);
}

QString DOnlineTranslator::source() const
{
    return d->source;
}

QString DOnlineTranslator::sourceTranslit() const
{
    return d->sourceTranslit;
}

QString DOnlineTranslator::sourceTranscription() const
{
    return d->sourceTranscription;
}

QString DOnlineTranslator::sourceLanguageName() const
{
    return languageName(d->sourceLang);
}

DOnlineTranslator::Language DOnlineTranslator::sourceLanguage() const
{
    return d->sourceLang;
}

QString DOnlineTranslator::translation() const
{
    return d->translation;
}

QString DOnlineTranslator::translationTranslit() const
{
    return d->translationTranslit;
}

QString DOnlineTranslator::translationLanguageName() const
{
    return languageName(d->translationLang);
}

DOnlineTranslator::Language DOnlineTranslator::translationLanguage() const
{
    return d->translationLang;
}

QMap<QString, QVector<DOnlineTranslatorOption>> DOnlineTranslator::translationOptions() const
{
    return d->translationOptions;
}

DOnlineTranslator::TranslationError DOnlineTranslator::error() const
{
    return d->error;
}

QString DOnlineTranslator::errorString() const
{
    return d->errorString;
}

QString DOnlineTranslator::languageCode(Language lang)
{
    return DOnlineTranslator::Private::s_genericLanguageCodes.value(lang);
}

DOnlineTranslator::Language DOnlineTranslator::language(const QString& langCode)
{
    return DOnlineTranslator::Private::s_genericLanguageCodes.key(langCode, NoLanguage);
}

} // namespace Digikam
