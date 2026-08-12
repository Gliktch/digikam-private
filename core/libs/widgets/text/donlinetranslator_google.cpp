/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Google methods.
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

void DOnlineTranslator::slotRequestGoogleTranslate()
{
    const QString sourceText = sender()->property(Private::s_textProperty).toString();

    // Generate API url

    QUrl url(QStringLiteral("https://translate.googleapis.com/translate_a/single"));

    url.setQuery(QStringLiteral("client=gtx&ie=UTF-8&oe=UTF-8&dt=bd&dt=ex&dt=ld&dt=md&dt=rw&dt=rm&dt=ss&dt=t&dt=at&dt=qc&sl=%1&tl=%2&hl=%3&q=%4")
                     .arg(languageApiCode(Google, d->sourceLang),
                          languageApiCode(Google, d->translationLang),
                          languageApiCode(Google, d->uiLang),
                          QString::fromUtf8(QUrl::toPercentEncoding(sourceText))));

    d->currentReply = d->networkManager->get(QNetworkRequest(url));
}

void DOnlineTranslator::slotParseGoogleTranslate()
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    // Check for error

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        if (d->currentReply->error() == QNetworkReply::ServiceUnavailableError)
        {
            resetData(ServiceError, i18n("Error: Engine systems have detected suspicious traffic from your "
                                         "computer network. Please try your request again later."));
        }
        else
        {
            resetData(NetworkError, d->currentReply->errorString());
        }

        return;
    }

    // Check availability of service

    const QByteArray data = d->currentReply->readAll();

    if (data.startsWith('<'))
    {
        resetData(ServiceError, i18n("Error: Engine systems have detected suspicious traffic from your "
                                     "computer network. Please try your request again later."));
        return;
    }

    // Read Json

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    const QJsonArray jsonData        = jsonResponse.array();

    if (d->sourceLang == Auto)
    {
        // Parse language

        d->sourceLang = language(Google, jsonData.at(2).toString());

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

    addSpaceBetweenParts(d->translation);
    addSpaceBetweenParts(d->translationTranslit);
    addSpaceBetweenParts(d->sourceTranslit);
    const auto tdata = jsonData.at(0).toArray();

    for (const QJsonValue& translationData : tdata)
    {
        const QJsonArray translationArray = translationData.toArray();
        d->translation.append(translationArray.at(0).toString());

        if (d->translationTranslitEnabled)
        {
            d->translationTranslit.append(translationArray.at(2).toString());
        }

        if (d->sourceTranslitEnabled)
        {
            d->sourceTranslit.append(translationArray.at(3).toString());
        }
    }

    if (d->source.size() >= Private::s_googleTranslateLimit)
    {
        return;
    }

    // Translation options

    if (d->translationOptionsEnabled)
    {
        const auto sdata = jsonData.at(1).toArray();

        for (const QJsonValue& typeOfSpeechData : sdata)
        {
            const QJsonArray typeOfSpeechDataArray = typeOfSpeechData.toArray();
            const QString typeOfSpeech             = typeOfSpeechDataArray.at(0).toString();
            const auto wdata                       = typeOfSpeechDataArray.at(2).toArray();

            for (const QJsonValue& wordData : wdata)
            {
                const QJsonArray wordDataArray     = wordData.toArray();
                const QString word                 = wordDataArray.at(0).toString();
                const QString gender               = wordDataArray.at(4).toString();
                const QJsonArray translationsArray = wordDataArray.at(1).toArray();
                QStringList translations;
                translations.reserve(translationsArray.size());

                for (const QJsonValue& wordTranslation : translationsArray)
                {
                    translations.append(wordTranslation.toString());
                }

                d->translationOptions[typeOfSpeech].append({word, gender, translations});
            }
        }
    }
}

void DOnlineTranslator::buildGoogleStateMachine()
{
    // States (Google sends translation, translit and dictionary in one request,
    // that will be split into several by the translation limit)

    auto* translationState = new QState(d->stateMachine);
    auto* finalState       = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(translationState);

    translationState->addTransition(translationState, &QState::finished, finalState);

    // Setup translation state

    buildSplitNetworkRequest(translationState,
                             &DOnlineTranslator::slotRequestGoogleTranslate,
                             &DOnlineTranslator::slotParseGoogleTranslate,
                             d->source,
                             Private::s_googleTranslateLimit);
}

void DOnlineTranslator::buildGoogleDetectStateMachine()
{
    // States

    auto* detectState  = new QState(d->stateMachine);
    auto* finalState   = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(detectState);

    detectState->addTransition(detectState, &QState::finished, finalState);

    // Setup detect state

    const QString text = d->source.left(getSplitIndex(d->source, Private::s_googleTranslateLimit));

    buildNetworkRequestState(detectState,
                             &DOnlineTranslator::slotRequestGoogleTranslate                             ,
                             &DOnlineTranslator::slotParseGoogleTranslate,
                             text);
}

} // namespace Digikam
