/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Tnadex methods.
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

void DOnlineTranslator::slotRequestYandexKey()
{
    const QUrl url(QStringLiteral("https://translate.yandex.com"));
    d->currentReply = d->networkManager->get(QNetworkRequest(url));
}

void DOnlineTranslator::slotParseYandexKey()
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        resetData(NetworkError, d->currentReply->errorString());
        return;
    }

    // Check availability of service

    const QByteArray webSiteData = d->currentReply->readAll();

    if (webSiteData.isEmpty()                        ||
        webSiteData.contains("<title>Oops!</title>") ||
        webSiteData.contains("<title>302 Found</title>"))
    {
        resetData(ServiceError, i18n("Error: Engine systems have detected suspicious traffic "
                                     "from your computer network. Please try your request again later."));
        return;
    }

    const QByteArray sidBeginString = "SID: '";
    const int sidBeginStringPos     = webSiteData.indexOf(sidBeginString);

    if (sidBeginStringPos == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to find Yandex SID in web version."));
        return;
    }

    const int sidBeginPosition = sidBeginStringPos + sidBeginString.size();
    const int sidEndPosition   = webSiteData.indexOf('\'', sidBeginPosition);

    if (sidEndPosition == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to extract Yandex SID from web version."));
        return;
    }

    // Yandex show reversed parts of session ID, need to decode

    const QString sid    = QString::fromUtf8(webSiteData.mid(sidBeginPosition,
                                                             sidEndPosition - sidBeginPosition));

    QStringList sidParts = sid.split(QLatin1Char('.'));

    for (int i = 0 ; i < sidParts.size() ; ++i)
    {
        std::reverse(sidParts[i].begin(), sidParts[i].end());
    }

    Private::s_yandexKey = sidParts.join(QLatin1Char('.'));
}

void DOnlineTranslator::slotRequestYandexTranslate()
{
    const QString sourceText = sender()->property(Private::s_textProperty).toString();

    QString lang;

    if (d->sourceLang == Auto)
    {
        lang = languageApiCode(Yandex, d->translationLang);
    }
    else
    {
        lang = languageApiCode(Yandex, d->sourceLang) + QLatin1Char('-') + languageApiCode(Yandex, d->translationLang);
    }

    // Generate API url

    QUrl url(QStringLiteral("https://translate.yandex.net/api/v1/tr.json/translate"));

    url.setQuery(QStringLiteral("id=%1-2-0&srv=tr-text&text=%2&lang=%3")
                     .arg(Private::s_yandexKey,
                          QString::fromUtf8(QUrl::toPercentEncoding(sourceText)),
                          lang));

    // Setup request

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setUrl(url);

    // Make reply

    d->currentReply = d->networkManager->post(request, QByteArray());
}

void DOnlineTranslator::slotParseYandexTranslate()
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    // Check for errors

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        // Network errors

        if (d->currentReply->error() < QNetworkReply::ContentAccessDenied)
        {
            resetData(NetworkError, d->currentReply->errorString());
            return;
        }

        // Parse data to get request error type

        Private::s_yandexKey.clear();
        const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
        resetData(ServiceError, jsonResponse.object().value(QStringLiteral("message")).toString());

        return;
    }

    // Read Json

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
    const QJsonObject jsonData       = jsonResponse.object();

    // Parse language

    if (d->sourceLang == Auto)
    {
        QString sourceCode = jsonData.value(QStringLiteral("lang")).toString();
        sourceCode         = sourceCode.left(sourceCode.indexOf(QLatin1Char('-')));
        d->sourceLang      = language(Yandex, sourceCode);

        if (d->sourceLang == NoLanguage)
        {
            resetData(ParsingError, i18n("Error: Unable to parse autodetected language"));
            return;
        }

        if (d->onlyDetectLanguage)
        {
            return;
        }
    }

    // Parse translation data

    d->translation += jsonData.value(QStringLiteral("text")).toArray().at(0).toString();
}

void DOnlineTranslator::slotRequestYandexSourceTranslit()
{
    requestYandexTranslit(d->sourceLang);
}

void DOnlineTranslator::slotParseYandexSourceTranslit()
{
    parseYandexTranslit(d->sourceTranslit);
}

void DOnlineTranslator::slotRequestYandexTranslationTranslit()
{
    requestYandexTranslit(d->translationLang);
}

void DOnlineTranslator::slotParseYandexTranslationTranslit()
{
    parseYandexTranslit(d->translationTranslit);
}

void DOnlineTranslator::slotRequestYandexDictionary()
{
    // Check if language is supported (need to check here because language may be autodetected)

    if (
        !isSupportDictionary(Yandex, d->sourceLang, d->translationLang) &&
        !d->source.contains(QLatin1Char(' '))
       )
    {
        auto* state = qobject_cast<QState*>(sender());
        state->addTransition(new QFinalState(state->parentState()));
        return;
    }

    // Generate API url

    const QString text = sender()->property(Private::s_textProperty).toString();
    QUrl url(QStringLiteral("https://dictionary.yandex.net/dicservice.json/lookupMultiple"));

    url.setQuery(QStringLiteral("text=%1&ui=%2&dict=%3-%4")
                     .arg(QString::fromUtf8(QUrl::toPercentEncoding(text)),
                          languageApiCode(Yandex, d->uiLang),
                          languageApiCode(Yandex, d->sourceLang),
                          languageApiCode(Yandex, d->translationLang)));

    d->currentReply = d->networkManager->get(QNetworkRequest(url));
}

void DOnlineTranslator::slotParseYandexDictionary()
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        resetData(NetworkError, d->currentReply->errorString());
        return;
    }

    // Parse reply

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
    const QJsonValue jsonData        = jsonResponse.object().value(languageApiCode(Yandex, d->sourceLang) +
                                       QLatin1Char('-')                                                  +
                                       languageApiCode(Yandex, d->translationLang)).toObject().value(QStringLiteral("regular"));

    if (d->sourceTranscriptionEnabled)
    {
        d->sourceTranscription = jsonData.toArray().at(0).toObject().value(QStringLiteral("ts")).toString();
    }

    const auto sdata = jsonData.toArray();

    for (const QJsonValue& typeOfSpeechData : sdata)
    {
        QJsonObject typeOfSpeechObject = typeOfSpeechData.toObject();
        const QString typeOfSpeech     = typeOfSpeechObject.value(QStringLiteral("pos")).toObject().value(QStringLiteral("text")).toString();
        const auto wdata               = typeOfSpeechObject.value(QStringLiteral("tr")).toArray();

        for (const QJsonValue& wordData : wdata)
        {
            // Parse translation options

            const QJsonObject wordObject       = wordData.toObject();
            const QString word                 = wordObject.value(QStringLiteral("text")).toString();
            const QString gender               = wordObject.value(QStringLiteral("gen")).toObject().value(QStringLiteral("text")).toString();
            const QJsonArray translationsArray = wordObject.value(QStringLiteral("mean")).toArray();
            QStringList translations;
            translations.reserve(translationsArray.size());

            for (const QJsonValue& wordTranslation : std::as_const(translationsArray))
            {
                translations.append(wordTranslation.toObject().value(QStringLiteral("text")).toString());
            }

            d->translationOptions[typeOfSpeech].append({word, gender, translations});
        }
    }
}

void DOnlineTranslator::buildYandexStateMachine()
{
    // States

    auto* keyState                  = new QState(d->stateMachine); // Generate SID from web version first to access API
    auto* translationState          = new QState(d->stateMachine);
    auto* sourceTranslitState       = new QState(d->stateMachine);
    auto* translationTranslitState  = new QState(d->stateMachine);
    auto* dictionaryState           = new QState(d->stateMachine);
    auto* finalState                = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(keyState);

    // Transitions

    keyState->addTransition(keyState, &QState::finished, translationState);
    translationState->addTransition(translationState, &QState::finished, sourceTranslitState);
    sourceTranslitState->addTransition(sourceTranslitState, &QState::finished, translationTranslitState);
    translationTranslitState->addTransition(translationTranslitState, &QState::finished, dictionaryState);
    dictionaryState->addTransition(dictionaryState, &QState::finished, finalState);

    // Setup key state

    if (Private::s_yandexKey.isEmpty())
    {
        buildNetworkRequestState(keyState,
                                 &DOnlineTranslator::slotRequestYandexKey,
                                 &DOnlineTranslator::slotParseYandexKey);
    }
    else
    {
        keyState->setInitialState(new QFinalState(keyState));
    }

    // Setup translation state

    buildSplitNetworkRequest(translationState,
                             &DOnlineTranslator::slotRequestYandexTranslate,
                             &DOnlineTranslator::slotParseYandexTranslate,
                             d->source,
                             Private::s_yandexTranslateLimit);

    // Setup source translit state

    if (d->sourceTranslitEnabled)
    {
        buildSplitNetworkRequest(sourceTranslitState,
                                 &DOnlineTranslator::slotRequestYandexSourceTranslit,
                                 &DOnlineTranslator::slotParseYandexSourceTranslit,
                                 d->source,
                                 Private::s_yandexTranslitLimit);
    }
    else
    {
        sourceTranslitState->setInitialState(new QFinalState(sourceTranslitState));
    }

    // Setup translation translit state

    if (d->translationTranslitEnabled)
    {
        buildSplitNetworkRequest(translationTranslitState,
                                 &DOnlineTranslator::slotRequestYandexTranslationTranslit,
                                 &DOnlineTranslator::slotParseYandexTranslationTranslit,
                                 d->translation,
                                 Private::s_yandexTranslitLimit);
    }
    else
    {
        translationTranslitState->setInitialState(new QFinalState(translationTranslitState));
    }

    // Setup dictionary state

    if (d->translationOptionsEnabled && !isContainsSpace(d->source))
    {
        buildNetworkRequestState(dictionaryState,
                                 &DOnlineTranslator::slotRequestYandexDictionary,
                                 &DOnlineTranslator::slotParseYandexDictionary,
                                 d->source);
    }
    else
    {
        dictionaryState->setInitialState(new QFinalState(dictionaryState));
    }
}

void DOnlineTranslator::buildYandexDetectStateMachine()
{
    // States

    auto* keyState    = new QState(d->stateMachine); // Generate SID from web version first to access API
    auto* detectState = new QState(d->stateMachine);
    auto* finalState  = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(keyState);

    // Transitions

    keyState->addTransition(keyState, &QState::finished, detectState);
    detectState->addTransition(detectState, &QState::finished, finalState);

    // Setup key state

    if (Private::s_yandexKey.isEmpty())
    {
        buildNetworkRequestState(keyState,
                                 &DOnlineTranslator::slotRequestYandexKey,
                                 &DOnlineTranslator::slotParseYandexKey);
    }
    else
    {
        keyState->setInitialState(new QFinalState(keyState));
    }

    // Setup detect state

    const QString text = d->source.left(getSplitIndex(d->source, Private::s_yandexTranslateLimit));

    buildNetworkRequestState(detectState,
                             &DOnlineTranslator::slotRequestYandexTranslate,
                             &DOnlineTranslator::slotParseYandexTranslate,
                             text);
}

void DOnlineTranslator::requestYandexTranslit(Language language)
{
    // Check if language is supported (need to check here because language may be autodetected)

    if (!isSupportTranslit(Yandex, language))
    {
        auto* state = qobject_cast<QState *>(sender());
        state->addTransition(new QFinalState(state->parentState()));

        return;
    }

    const QString text = sender()->property(Private::s_textProperty).toString();

    // Generate API url

    QUrl url(QStringLiteral("https://translate.yandex.net/translit/translit"));
    url.setQuery(QString::fromUtf8("text=%1&lang=%2")
                 .arg(QString::fromUtf8(QUrl::toPercentEncoding(text)))
                 .arg(languageApiCode(Yandex, language)));

    d->currentReply = d->networkManager->get(QNetworkRequest(url));
}

void DOnlineTranslator::parseYandexTranslit(QString& text)
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        resetData(NetworkError, d->currentReply->errorString());

        return;
    }

    const QByteArray reply = d->currentReply->readAll();
    text                  += QString::fromUtf8(reply.mid(1).chopped(1));
}

} // namespace Digikam
