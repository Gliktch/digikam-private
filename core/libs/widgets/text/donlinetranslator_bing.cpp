/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Bing methods.
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

void DOnlineTranslator::slotRequestBingCredentials()
{
    const QUrl url(QStringLiteral("https://www.bing.com/translator"));
    d->currentReply = d->networkManager->get(QNetworkRequest(url));
}

void DOnlineTranslator::slotParseBingCredentials()
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

    const QByteArray webSiteData            = d->currentReply->readAll();
    const QByteArray credentialsBeginString = "var params_RichTranslateHelper = [";
    const int credentialsBeginPos           = webSiteData.indexOf(credentialsBeginString);

    if (credentialsBeginPos == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to find Bing credentials in web version."));
        return;
    }

    const int keyBeginPos = credentialsBeginPos + credentialsBeginString.size();
    const int keyEndPos   = webSiteData.indexOf(',', keyBeginPos);

    if (keyEndPos == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to extract Bing key from web version."));
        return;
    }

    Private::s_bingKey               = webSiteData.mid(keyBeginPos, keyEndPos - keyBeginPos);
    const int tokenBeginPos = keyEndPos + 2; // Skip two symbols instead of one because the value is enclosed in quotes
    const int tokenEndPos   = webSiteData.indexOf('"', tokenBeginPos);

    if (tokenEndPos == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to extract Bing token from web version."));
        return;
    }

    Private::s_bingToken          = webSiteData.mid(tokenBeginPos, tokenEndPos - tokenBeginPos);
    const int igBeginPos = webSiteData.indexOf("IG");
    const int igEndPos   = webSiteData.indexOf('"', igBeginPos + 2);

    if (igEndPos == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to extract additional Bing information from web version."));
        return;
    }

    Private::s_bingIg              = QString::fromUtf8(webSiteData.mid(igBeginPos, igEndPos - igBeginPos));
    const int iidBeginPos = webSiteData.indexOf("data-iid");
    const int iidEndPos   = webSiteData.indexOf('"', iidBeginPos + 2);

    if (iidEndPos == -1)
    {
        resetData(ParsingError, i18n("Error: Unable to extract additional Bing information from web version."));
        return;
    }

    Private::s_bingIid = QString::fromUtf8(webSiteData.mid(iidBeginPos, iidEndPos - iidBeginPos));
}

void DOnlineTranslator::slotRequestBingTranslate()
{
    const QString sourceText = sender()->property(Private::s_textProperty).toString();

    // Generate POST data

    const QByteArray postData = "&text="     + QUrl::toPercentEncoding(sourceText)
                              + "&fromLang=" + languageApiCode(Bing, d->sourceLang).toUtf8()
                              + "&to="       + languageApiCode(Bing, d->translationLang).toUtf8()
                              + "&token="    + Private::s_bingToken
                              + "&key="      + Private::s_bingKey;

    QUrl url(QStringLiteral("https://www.bing.com/ttranslatev3"));
    url.setQuery(QStringLiteral("IG=%1&IID=%2").arg(Private::s_bingIg, Private::s_bingIid));

    // Setup request

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromUtf8("%1/%2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));
    request.setUrl(url);

    // Make reply

    d->currentReply = d->networkManager->post(request, postData);
}

void DOnlineTranslator::slotParseBingTranslate()
{
    d->currentReply->deleteLater();

    // Check for errors

    if (d->currentReply->error() != QNetworkReply::NoError)
    {
        resetData(NetworkError, d->currentReply->errorString());
        return;
    }

    // Parse translation data

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
    const QJsonObject responseObject = jsonResponse.array().first().toObject();

    if (d->sourceLang == Auto)
    {
        const QString langCode = responseObject.value(QStringLiteral("detectedLanguage")).toObject().value(QStringLiteral("language")).toString();
        d->sourceLang           = language(Bing, langCode);

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

    const QJsonObject translationsObject = responseObject.value(QStringLiteral("translations")).toArray().first().toObject();
    d->translation                       += translationsObject.value(QStringLiteral("text")).toString();
    d->translationTranslit               += translationsObject.value(QStringLiteral("transliteration")).toObject().value(QStringLiteral("text")).toString();
}

void DOnlineTranslator::slotRequestBingDictionary()
{
    // Check if language is supported (need to check here because language may be autodetected)

    if (!isSupportDictionary(Bing, d->sourceLang, d->translationLang) && !d->source.contains(QLatin1Char(' ')))
    {
        auto* state = qobject_cast<QState *>(sender());
        state->addTransition(new QFinalState(state->parentState()));
        return;
    }

    // Generate POST data

    const QByteArray postData = "&text=" + QUrl::toPercentEncoding(sender()->property(Private::s_textProperty).toString())
                              + "&from=" + languageApiCode(Bing, d->sourceLang).toUtf8()
                              + "&to="   + languageApiCode(Bing, d->translationLang).toUtf8();

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setUrl(QUrl(QStringLiteral("https://www.bing.com/tlookupv3")));

    d->currentReply = d->networkManager->post(request, postData);
}

void DOnlineTranslator::slotParseBingDictionary()
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
    const auto ddata                 = responseObject.value(QStringLiteral("translations")).toArray();

    for (const QJsonValue& dictionaryData : ddata)
    {
        const QJsonObject dictionaryObject = dictionaryData.toObject();
        const QString typeOfSpeech         = dictionaryObject.value(QStringLiteral("posTag")).toString().toLower();
        const QString word                 = dictionaryObject.value(QStringLiteral("displayTarget")).toString().toLower();
        const QJsonArray translationsArray = dictionaryObject.value(QStringLiteral("backTranslations")).toArray();
        QStringList translations;
        translations.reserve(translationsArray.size());

        for (const QJsonValue& wordTranslation : std::as_const(translationsArray))
        {
            translations.append(wordTranslation.toObject().value(QStringLiteral("displayText")).toString());
        }

        d->translationOptions[typeOfSpeech].append({word, {}, translations});
    }
}

void DOnlineTranslator::buildBingStateMachine()
{
    // States

    auto* credentialsState = new QState(d->stateMachine); // Generate credentials from web version first to access API
    auto* translationState = new QState(d->stateMachine);
    auto* dictionaryState  = new QState(d->stateMachine);
    auto* finalState       = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(credentialsState);

    // Transitions

    credentialsState->addTransition(credentialsState, &QState::finished, translationState);
    translationState->addTransition(translationState, &QState::finished, dictionaryState);
    dictionaryState->addTransition(dictionaryState, &QState::finished, finalState);

    // Setup credentials state

    if (Private::s_bingKey.isEmpty() || Private::s_bingToken.isEmpty())
    {
        buildNetworkRequestState(credentialsState,
                                 &DOnlineTranslator::slotRequestBingCredentials,
                                 &DOnlineTranslator::slotParseBingCredentials);
    }
    else
    {
        credentialsState->setInitialState(new QFinalState(credentialsState));
    }

    // Setup translation state

    buildSplitNetworkRequest(translationState,
                             &DOnlineTranslator::slotRequestBingTranslate,
                             &DOnlineTranslator::slotParseBingTranslate,
                             d->source,
                             Private::s_bingTranslateLimit);

    // Setup dictionary state

    if (d->translationOptionsEnabled && !isContainsSpace(d->source))
    {
        buildNetworkRequestState(dictionaryState,
                                 &DOnlineTranslator::slotRequestBingDictionary,
                                 &DOnlineTranslator::slotParseBingDictionary,
                                 d->source);
    }
    else
    {
        dictionaryState->setInitialState(new QFinalState(dictionaryState));
    }
}

void DOnlineTranslator::buildBingDetectStateMachine()
{
    // States

    auto* detectState = new QState(d->stateMachine);
    auto* finalState  = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(detectState);

    detectState->addTransition(detectState, &QState::finished, finalState);

    // Setup translation state

    const QString text = d->source.left(getSplitIndex(d->source, Private::s_bingTranslateLimit));

    buildNetworkRequestState(detectState,
                             &DOnlineTranslator::slotRequestBingTranslate,
                             &DOnlineTranslator::slotParseBingTranslate,
                             text);
}

} // namespace Digikam
