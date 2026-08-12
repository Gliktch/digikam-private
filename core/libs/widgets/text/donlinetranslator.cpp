/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services.
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

DOnlineTranslator::DOnlineTranslator(QObject* const parent)
    : QObject(parent),
      d      (new Private(this))
{
    connect(d->stateMachine, &QStateMachine::finished,
            this, &DOnlineTranslator::signalFinished);

    connect(d->stateMachine, &QStateMachine::stopped,
            this, &DOnlineTranslator::signalFinished);
}

DOnlineTranslator::~DOnlineTranslator()
{
    delete d;
}

void DOnlineTranslator::translate(const QString& text,
                                  Engine engine,
                                  Language translationLang,
                                  Language sourceLang,
                                  Language uiLang)
{
    abort();
    resetData();

    d->onlyDetectLanguage = false;
    d->source             = text;
    d->sourceLang         = sourceLang;
    d->translationLang    = (translationLang == Auto) ? language(QLocale()) : translationLang;
    d->uiLang             = (uiLang == Auto)          ? language(QLocale()) : uiLang;

    // Check if the selected languages are supported by the engine

   if (!isSupportTranslation(engine, d->sourceLang))
    {
        resetData(ParametersError,
                  i18n("Selected source language %1 is not supported for %2",
                       languageName(d->sourceLang),
                       QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

        Q_EMIT signalFinished();

        return;
    }

    if (!isSupportTranslation(engine, d->translationLang))
    {
        resetData(ParametersError,
                  i18n("Selected translation language %1 is not supported for %2",
                       languageName(d->translationLang),
                       QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

        Q_EMIT signalFinished();

        return;
    }

    if (!isSupportTranslation(engine, d->uiLang))
    {
        resetData(ParametersError,
                  i18n("Selected ui language %1 is not supported for %2",
                       languageName(d->uiLang),
                       QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

        Q_EMIT signalFinished();

        return;
    }

    switch (engine)
    {
        case Google:
        {
            buildGoogleStateMachine();
            break;
        }

        case Yandex:
        {
            buildYandexStateMachine();
            break;
        }

        case Bing:
        {
            buildBingStateMachine();
            break;
        }

        case LibreTranslate:
        {
            if (d->libreUrl.isEmpty())
            {
                resetData(ParametersError,
                        i18n("%1 URL cannot be empty.",
                             QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

                Q_EMIT signalFinished();

                return;
            }

            buildLibreStateMachine();
            break;
        }

        case Lingva:
        {
            if (d->lingvaUrl.isEmpty())
            {
                resetData(ParametersError,
                        i18n("%1 URL cannot be empty.",
                             QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

                Q_EMIT signalFinished();

                return;
            }

            buildLingvaStateMachine();
            break;
        }
    }

    d->stateMachine->start();
}

QString DOnlineTranslator::engineName(Engine engine)
{
    switch (engine)
    {
        case Yandex:
        {
            return QLatin1String("Yandex");
        }

        case Bing:
        {
            return QLatin1String("Bing");
        }

        case LibreTranslate:
        {
            return QLatin1String("Libre Translate");
        }

        case Lingva:
        {
            return QLatin1String("Lingva");
        }

        default:
        {
            return QLatin1String("Google");
        }
    }
}

void DOnlineTranslator::detectLanguage(const QString& text, Engine engine)
{
    abort();
    resetData();

    d->onlyDetectLanguage = true;
    d->source             = text;
    d->sourceLang         = Auto;
    d->translationLang    = English;
    d->uiLang             = language(QLocale());

    switch (engine)
    {
        case Google:
        {
            buildGoogleDetectStateMachine();
            break;
        }

        case Yandex:
        {
            buildYandexDetectStateMachine();
            break;
        }

        case Bing:
        {
            buildBingDetectStateMachine();
            break;
        }

        case LibreTranslate:
        {
            if (d->libreUrl.isEmpty())
            {
                resetData(ParametersError,
                        i18n("%1 URL cannot be empty.",
                             QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

                Q_EMIT signalFinished();

                return;
            }

            buildLibreDetectStateMachine();
            break;
        }

        case Lingva:
        {
            if (d->lingvaUrl.isEmpty())
            {
                resetData(ParametersError,
                        i18n("%1 URL cannot be empty.",
                             QString::fromUtf8(QMetaEnum::fromType<Engine>().valueToKey(engine))));

                Q_EMIT signalFinished();

                return;
            }

            buildLingvaDetectStateMachine();
            break;
        }
    }

    d->stateMachine->start();
}

void DOnlineTranslator::abort()
{
    if (d->currentReply != nullptr)
    {
        d->currentReply->abort();
    }
}

bool DOnlineTranslator::isRunning() const
{
    return d->stateMachine->isRunning();
}

void DOnlineTranslator::slotSkipGarbageText()
{
    d->translation.append(sender()->property(Private::s_textProperty).toString());
}

void DOnlineTranslator::buildSplitNetworkRequest(QState* const parent,
                                                 void (DOnlineTranslator::*requestMethod)(),
                                                 void (DOnlineTranslator::*parseMethod)(),
                                                 const QString& text,
                                                 int textLimit)
{
    QString unsendedText       = text;
    auto* nextTranslationState = new QState(parent);
    parent->setInitialState(nextTranslationState);

    while (!unsendedText.isEmpty())
    {
        auto* currentTranslationState = nextTranslationState;
        nextTranslationState          = new QState(parent);

        // Do not translate the part if it looks like garbage

        const int splitIndex          = getSplitIndex(unsendedText, textLimit);

        if (splitIndex == -1)
        {
            currentTranslationState->setProperty(Private::s_textProperty, unsendedText.left(textLimit));
            currentTranslationState->addTransition(nextTranslationState);

            connect(currentTranslationState, &QState::entered,
                    this, &DOnlineTranslator::slotSkipGarbageText);

            // Remove the parsed part from the next parsing

            unsendedText = unsendedText.mid(textLimit);
        }
        else
        {
            buildNetworkRequestState(currentTranslationState, requestMethod, parseMethod, unsendedText.left(splitIndex));
            currentTranslationState->addTransition(currentTranslationState, &QState::finished, nextTranslationState);

            // Remove the parsed part from the next parsing

            unsendedText = unsendedText.mid(splitIndex);
        }
    }

    nextTranslationState->addTransition(new QFinalState(parent));
}

void DOnlineTranslator::buildNetworkRequestState(QState* const parent,
                                                 void (DOnlineTranslator::*requestMethod)(),
                                                 void (DOnlineTranslator::*parseMethod)(),
                                                 const QString& text)
{
    // Network substates

    auto* requestingState = new QState(parent);
    auto* parsingState    = new QState(parent);

    parent->setInitialState(requestingState);

    connect(d->networkManager, &QNetworkAccessManager::finished,
            parsingState, [parsingState](QNetworkReply* reply)
        {
            parsingState->setProperty("QNetworkReply", (quintptr)reply);
        }
    );

    // Substates transitions

    requestingState->addTransition(d->networkManager, &QNetworkAccessManager::finished, parsingState);
    parsingState->addTransition(new QFinalState(parent));

    // Setup requesting state

    requestingState->setProperty(Private::s_textProperty, text);

    connect(requestingState, &QState::entered,
            this, requestMethod);

    // Setup parsing state

    connect(parsingState, &QState::entered,
            this, parseMethod);
}

void DOnlineTranslator::resetData(TranslationError error, const QString& errorString)
{
    d->error       = error;
    d->errorString = errorString;
    d->translation.clear();
    d->translationTranslit.clear();
    d->sourceTranslit.clear();
    d->sourceTranscription.clear();
    d->translationOptions.clear();

    d->stateMachine->stop();
    const auto states = d->stateMachine->findChildren<QAbstractState*>();

    for (QAbstractState* const state : states)
    {
        if (!d->stateMachine->configuration().contains(state))
        {
            state->deleteLater();
        }
    }
}

QString DOnlineTranslator::languageApiCode(Engine engine, Language lang)
{
    if (!isSupportTranslation(engine, lang))
    {
        return QString();
    }

    switch (engine)
    {
        case Google:
        {
            return DOnlineTranslator::Private::s_googleLanguageCodes.value(lang, DOnlineTranslator::Private::s_genericLanguageCodes.value(lang));
        }

        case Yandex:
        {
            return DOnlineTranslator::Private::s_yandexLanguageCodes.value(lang, DOnlineTranslator::Private::s_genericLanguageCodes.value(lang));
        }

        case Bing:
        {
            return DOnlineTranslator::Private::s_bingLanguageCodes.value(lang, DOnlineTranslator::Private::s_genericLanguageCodes.value(lang));
        }

        case LibreTranslate:
        {
            return DOnlineTranslator::Private::s_genericLanguageCodes.value(lang);
        }

        case Lingva:
        {
            return DOnlineTranslator::Private::s_lingvaLanguageCodes.value(lang, DOnlineTranslator::Private::s_genericLanguageCodes.value(lang));
        }
    }

    Q_UNREACHABLE();
}

DOnlineTranslator::Language DOnlineTranslator::language(Engine engine, const QString& langCode)
{
    // Engine exceptions

    switch (engine)
    {
        case Google:
        {
            return DOnlineTranslator::Private::s_googleLanguageCodes.key(langCode, DOnlineTranslator::Private::s_genericLanguageCodes.key(langCode, NoLanguage));
        }

        case Yandex:
        {
            return DOnlineTranslator::Private::s_yandexLanguageCodes.key(langCode, DOnlineTranslator::Private::s_genericLanguageCodes.key(langCode, NoLanguage));
        }

        case Bing:
        {
            return DOnlineTranslator::Private::s_bingLanguageCodes.key(langCode, DOnlineTranslator::Private::s_genericLanguageCodes.key(langCode, NoLanguage));
        }

        case LibreTranslate:
        {
            return DOnlineTranslator::Private::s_genericLanguageCodes.key(langCode, NoLanguage);
        }

        case Lingva:
        {
            return DOnlineTranslator::Private::s_lingvaLanguageCodes.key(langCode, DOnlineTranslator::Private::s_genericLanguageCodes.key(langCode, NoLanguage));
        }
    }

    Q_UNREACHABLE();
}

int DOnlineTranslator::getSplitIndex(const QString& untranslatedText, int limit)
{
    if (untranslatedText.size() < limit)
    {
        return limit;
    }

    int splitIndex = untranslatedText.lastIndexOf(QLatin1String(". "), limit - 1);

    if (splitIndex != -1)
    {
        return splitIndex + 1;
    }

    splitIndex = untranslatedText.lastIndexOf(QLatin1Char(' '), limit - 1);

    if (splitIndex != -1)
    {
        return splitIndex + 1;
    }

    splitIndex = untranslatedText.lastIndexOf(QLatin1Char('\n'), limit - 1);

    if (splitIndex != -1)
    {
        return splitIndex + 1;
    }

    // Non-breaking space

    splitIndex = untranslatedText.lastIndexOf(QChar(0x00a0), limit - 1);

    if (splitIndex != -1)
    {
        return splitIndex + 1;
    }

    // If the text has not passed any check and is most likely garbage

    return limit;
}

bool DOnlineTranslator::isContainsSpace(const QString& text)
{
    return std::any_of(text.cbegin(), text.cend(), [](QChar symbol)
        {
            return symbol.isSpace();
        }
    );
}

void DOnlineTranslator::addSpaceBetweenParts(QString& text)
{
    if (text.isEmpty())
    {
        return;
    }

    if (!text.back().isSpace())
    {
        text.append(QLatin1Char(' '));
    }
}

} // namespace Digikam

#include "moc_donlinetranslator.cpp"
