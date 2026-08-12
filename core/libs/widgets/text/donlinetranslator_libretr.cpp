/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - LibreTranslate methods.
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

void DOnlineTranslator::slotRequestLibreLangDetection()
{
    const QString sourceText  = sender()->property(Private::s_textProperty).toString();

    // Generate POST data

    const QByteArray postData = "&q="       + QUrl::toPercentEncoding(sourceText)
                              + "&api_key=" + d->libreApiKey;

    // Setup request

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setUrl(QUrl(QString::fromUtf8("%1/detect").arg(d->libreUrl)));

    // Make reply

    d->currentReply            = d->networkManager->post(request, postData);
}

void DOnlineTranslator::slotParseLibreLangDetection()
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    // Check for errors

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        resetData(NetworkError, d->currentReply->errorString());
        return;
    }

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
    const QJsonObject responseObject = jsonResponse.array().first().toObject();

    if (d->sourceLang == Auto)
    {
        const QString langCode = responseObject.value(QStringLiteral("language")).toString();
        d->sourceLang           = language(LibreTranslate, langCode);

        if (d->sourceLang == NoLanguage)
        {
            resetData(ParsingError, i18n("Error: Unable to parse autodetected language"));
        }
    }
}

void DOnlineTranslator::slotRequestLibreTranslate()
{
    const QString sourceText  = sender()->property(Private::s_textProperty).toString();

    // Generate POST data

    const QByteArray postData = "&q="       + QUrl::toPercentEncoding(sourceText)
                              + "&source="  + languageApiCode(LibreTranslate, d->sourceLang).toUtf8()
                              + "&target="  + languageApiCode(LibreTranslate, d->translationLang).toUtf8()
                              + "&api_key=" + d->libreApiKey;

    // Setup request

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setUrl(QUrl(QString::fromUtf8("%1/translate").arg(d->libreUrl)));

    // Make reply

    d->currentReply            = d->networkManager->post(request, postData);
}

void DOnlineTranslator::slotParseLibreTranslate()
{
    if ((quintptr)d->currentReply.data() != sender()->property("QNetworkReply").value<quintptr>())
    {
        return;
    }

    d->currentReply->deleteLater();

    // Check for errors

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        resetData(NetworkError, d->currentReply->errorString());
        return;
    }

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
    const QJsonObject responseObject = jsonResponse.object();
    d->translation                   = responseObject.value(QStringLiteral("translatedText")).toString();
}

void DOnlineTranslator::buildLibreDetectStateMachine()
{
    // States

    auto* detectState = new QState(d->stateMachine);
    auto* finalState  = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(detectState);

    detectState->addTransition(detectState, &QState::finished, finalState);

    // Setup lang detection state

    const QString text = d->source.left(getSplitIndex(d->source, Private::s_libreTranslateLimit));

    buildNetworkRequestState(detectState,
                             &DOnlineTranslator::slotRequestLibreLangDetection,
                             &DOnlineTranslator::slotParseLibreLangDetection,
                             text);
}

void DOnlineTranslator::buildLibreStateMachine()
{
    // States

    auto* languageDetectionState = new QState(d->stateMachine);
    auto* translationState       = new QState(d->stateMachine);
    auto* finalState             = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(languageDetectionState);

    // Transitions

    languageDetectionState->addTransition(languageDetectionState, &QState::finished, translationState);
    translationState->addTransition(translationState, &QState::finished, finalState);

    // Setup LibreTranslate lang code detection

    buildNetworkRequestState(languageDetectionState,
                             &DOnlineTranslator::slotRequestLibreLangDetection,
                             &DOnlineTranslator::slotParseLibreLangDetection,
                             d->source);

    // Setup translation state

    buildSplitNetworkRequest(translationState,
                             &DOnlineTranslator::slotRequestLibreTranslate,
                             &DOnlineTranslator::slotParseLibreTranslate,
                             d->source,
                             Private::s_libreTranslateLimit);
}

} // namespace Digikam
