/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Lingva methods.
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

void DOnlineTranslator::slotRequestLingvaTranslate()
{
    const QString sourceText = sender()->property(Private::s_textProperty).toString();

    // Generate API url

    QUrl url(QString::fromUtf8("%1/api/v1/%2/%3/%4")
                    .arg(d->lingvaUrl)
                    .arg(languageApiCode(Lingva, d->sourceLang))
                    .arg(languageApiCode(Lingva, d->translationLang))
                    .arg(QString::fromUtf8(QUrl::toPercentEncoding(sourceText))));

    d->currentReply = d->networkManager->get(QNetworkRequest(url));
}

void DOnlineTranslator::slotParseLingvaTranslate()
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

    // Parse translation data

    const QJsonDocument jsonResponse = QJsonDocument::fromJson(d->currentReply->readAll());
    const QJsonObject responseObject = jsonResponse.object();
    d->translation                    = responseObject.value(QStringLiteral("translation")).toString();
}

void DOnlineTranslator::buildLingvaStateMachine()
{
    // States

    auto* translationState = new QState(d->stateMachine);
    auto* finalState       = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(translationState);

    // Transitions

    translationState->addTransition(translationState, &QState::finished, finalState);

    // Setup translation state

    buildSplitNetworkRequest(translationState,
                             &DOnlineTranslator::slotRequestLingvaTranslate,
                             &DOnlineTranslator::slotParseLingvaTranslate,
                             d->source,
                             Private::s_googleTranslateLimit);
}

void DOnlineTranslator::buildLingvaDetectStateMachine()
{
    // States

    auto* detectState = new QState(d->stateMachine);
    auto* finalState  = new QFinalState(d->stateMachine);
    d->stateMachine->setInitialState(detectState);

    detectState->addTransition(detectState, &QState::finished, finalState);

    // Setup lang detection state

    const QString text = d->source.left(getSplitIndex(d->source, Private::s_googleTranslateLimit));

    buildNetworkRequestState(detectState,
                             &DOnlineTranslator::slotRequestLingvaTranslate,
                             &DOnlineTranslator::slotParseLingvaTranslate,
                             text);
}

} // namespace Digikam
