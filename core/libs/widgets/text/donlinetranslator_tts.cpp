/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - TTS support.
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

DOnlineTts::DOnlineTts(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

DOnlineTts::~DOnlineTts()
{
    delete d;
}

void DOnlineTts::generateUrls(const QString& text,
                              DOnlineTranslator::Engine engine,
                              DOnlineTranslator::Language lang,
                              Voice voice,
                              Emotion emotion)
{
    // Get speech

    QString unparsedText = text;

    switch (engine)
    {
        case DOnlineTranslator::Google:
        {
            if (voice != NoVoice)
            {
                setError(UnsupportedVoice, i18nc("@info", "Selected engine %1 does not support voice settings",
                                                 QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Engine>().valueToKey(engine))));
                return;
            }

            if (emotion != NoEmotion)
            {
                setError(UnsupportedEmotion, i18nc("@info", "Selected engine %1 does not support emotion settings",
                                                   QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Engine>().valueToKey(engine))));
                return;
            }

            const QString langString = languageApiCode(engine, lang);

            if (langString.isNull())
            {
                return;
            }

            // Google has a limit of characters per tts request. If the query is larger, then it should be split into several

            while (!unparsedText.isEmpty())
            {
                const int splitIndex = DOnlineTranslator::getSplitIndex(unparsedText, d->s_googleTtsLimit); // Split the part by special symbol

                // Generate URL API for adding it to the playlist

                QUrl apiUrl(QStringLiteral("https://translate.googleapis.com/translate_tts"));
                const QString query = QStringLiteral("ie=UTF-8&client=gtx&tl=%1&q=%2")
                                    .arg(langString, QString::fromUtf8(QUrl::toPercentEncoding(unparsedText.left(splitIndex))));

#if defined(Q_OS_LINUX)

                apiUrl.setQuery(query);

#elif defined(Q_OS_WIN)

                apiUrl.setQuery(query, QUrl::DecodedMode);

#endif
                d->media.append(apiUrl);

                // Remove the said part from the next saying

                unparsedText = unparsedText.mid(splitIndex);
            }

            break;
        }

        case DOnlineTranslator::Yandex:
        {
            const QString langString = languageApiCode(engine, lang);

            if (langString.isNull())
            {
                return;
            }

            const QString voiceString = voiceApiCode(engine, voice);

            if (voiceString.isNull())
            {
                return;
            }

            const QString emotionString = emotionApiCode(engine, emotion);

            if (emotionString.isNull())
            {
                return;
            }

            // Yandex has a limit of characters per tts request. If the query is larger, then it should be split into several

            while (!unparsedText.isEmpty())
            {
                const int splitIndex = DOnlineTranslator::getSplitIndex(unparsedText, d->s_yandexTtsLimit); // Split the part by special symbol

                // Generate URL API for adding to the playlist
                QUrl apiUrl(QStringLiteral("https://tts.voicetech.yandex.net/tts"));
                const QString query = QStringLiteral("text=%1&lang=%2&speaker=%3&emotion=%4&format=mp3")
                                        .arg(QString::fromUtf8(QUrl::toPercentEncoding(unparsedText.left(splitIndex))), langString, voiceString, emotionString);

#if defined(Q_OS_LINUX)

                apiUrl.setQuery(query);

#elif defined(Q_OS_WIN)

                apiUrl.setQuery(query, QUrl::DecodedMode);

#endif

                d->media.append(apiUrl);

                // Remove the said part from the next saying

                unparsedText = unparsedText.mid(splitIndex);
            }

            break;
        }

        case DOnlineTranslator::Bing:
        case DOnlineTranslator::LibreTranslate:
        case DOnlineTranslator::Lingva:
        {
            // NOTE:
            // Lingva returns audio in strange format, use placeholder, until we'll figure it out
            //
            // Example: https://lingva.ml/api/v1/audio/en/Hello%20World!
            // Will return json with uint bytes array, according to documentation
            // See: https://github.com/TheDavidDelta/lingva-translate#public-apis
            setError(UnsupportedEngine, i18nc("@info", "%1 engine does not support TTS",
                                              QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Engine>().valueToKey(engine))));
            break;
        }
    }
}

QList<QUrl> DOnlineTts::media() const
{
    return d->media;
}

QString DOnlineTts::errorString() const
{
    return d->errorString;
}

DOnlineTts::TtsError DOnlineTts::error() const
{
    return d->error;
}

QString DOnlineTts::voiceCode(Voice voice)
{
    return Private::s_voiceCodes.value(voice);
}

QString DOnlineTts::emotionCode(Emotion emotion)
{
    return Private::s_emotionCodes.value(emotion);
}

DOnlineTts::Emotion DOnlineTts::emotion(const QString& emotionCode)
{
    return Private::s_emotionCodes.key(emotionCode, NoEmotion);
}

DOnlineTts::Voice DOnlineTts::voice(const QString& voiceCode)
{
    return Private::s_voiceCodes.key(voiceCode, NoVoice);
}

void DOnlineTts::setError(TtsError error, const QString& errorString)
{
    d->error       = error;
    d->errorString = errorString;
}

// Returns engine-specific language code for tts
QString DOnlineTts::languageApiCode(DOnlineTranslator::Engine engine, DOnlineTranslator::Language lang)
{
    switch (engine)
    {
        case DOnlineTranslator::Google:
        case DOnlineTranslator::Lingva: // Lingva is a frontend to Google Translate
        {
            if (lang != DOnlineTranslator::Auto)
            {
                return DOnlineTranslator::languageApiCode(engine, lang); // Google use the same codes for tts (except 'auto')
            }

            break;
        }

        case DOnlineTranslator::Yandex:
        {
            switch (lang)
            {
                case DOnlineTranslator::Russian:
                {
                    return QStringLiteral("ru_RU");
                }

                case DOnlineTranslator::Tatar:
                {
                    return QStringLiteral("tr_TR");
                }

                case DOnlineTranslator::English:
                {
                    return QStringLiteral("en_GB");
                }

                default:
                {
                    break;
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }

    setError(UnsupportedLanguage, i18nc("@info", "Selected language %1 is not supported for %2",
                                        QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Language>().valueToKey(lang)),
                                        QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Engine>().valueToKey(engine))));

    return QString();
}

QString DOnlineTts::voiceApiCode(DOnlineTranslator::Engine engine, Voice voice)
{
    if (engine == DOnlineTranslator::Yandex)
    {
        if (voice == NoVoice)
        {
            return voiceCode(Zahar);
        }

        return voiceCode(voice);
    }

    setError(UnsupportedVoice, i18nc("@info", "Selected voice %1 is not supported for %2",
                                     QString::fromUtf8(QMetaEnum::fromType<Voice>().valueToKey(voice)),
                                     QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Engine>().valueToKey(engine))));

    return QString();
}

QString DOnlineTts::emotionApiCode(DOnlineTranslator::Engine engine, Emotion emotion)
{
    if (engine == DOnlineTranslator::Yandex)
    {
        if (emotion == NoEmotion)
        {
            return emotionCode(Neutral);
        }

        return emotionCode(emotion);
    }

    setError(UnsupportedEmotion, i18nc("@info", "Selected emotion %1 is not supported for %2",
                                       QString::fromUtf8(QMetaEnum::fromType<Emotion>().valueToKey(emotion)),
                                       QString::fromUtf8(QMetaEnum::fromType<DOnlineTranslator::Engine>().valueToKey(engine))));

    return QString();
}

} // namespace Digikam
