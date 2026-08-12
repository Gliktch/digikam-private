/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-01-05
 * Description : an unit test to check text online-translators.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QTest>
#include <QSignalSpy>
#include <QObject>
#include <QElapsedTimer>

// Local includes

#include "digikam_debug.h"
#include "donlinetranslator.h"

using namespace Digikam;

bool do_online_translation(const QString& text,
                           DOnlineTranslator::Engine engine,
                           DOnlineTranslator::Language trLang,
                           DOnlineTranslator::Language srcLang)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "-----------------------------------------------------:";
    qCDebug(DIGIKAM_TESTS_LOG) << "Text to translate        :" << text;
    qCDebug(DIGIKAM_TESTS_LOG) << "With WebService          :" << engine;
    qCDebug(DIGIKAM_TESTS_LOG) << "To target language       :" << trLang;
    qCDebug(DIGIKAM_TESTS_LOG) << "With source language     :" << srcLang;

    DOnlineTranslator* const trengine = new DOnlineTranslator;

    QSignalSpy spy(trengine, SIGNAL(signalFinished()));
    QElapsedTimer timer;
    timer.start();

    trengine->translate(text,       // String to translate
                        engine,     // Web service
                        trLang,     // Target language
                        srcLang,    // Source language
                        DOnlineTranslator::Auto);

    bool b = spy.wait(5000);
    qCDebug(DIGIKAM_TESTS_LOG) << "Elapsed time to translate:" << timer.elapsed() << "ms";

    if (b)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Translated text          :" << trengine->translation();
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Time-out with online translator";
        qCDebug(DIGIKAM_TESTS_LOG) << "Error                    :" << trengine->error();
    }

    delete trengine;

    return b;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    do_online_translation(QString::fromUtf8("Ma grande maison rouge au bord de la plage"),          // codespell:ignore
                          DOnlineTranslator::Google,
                          DOnlineTranslator::English,
                          DOnlineTranslator::Auto);

    do_online_translation(QString::fromUtf8("I was seen a big light on the sky during this night"),
                          DOnlineTranslator::Google,
                          DOnlineTranslator::French,
                          DOnlineTranslator::Auto);

    do_online_translation(QString::fromUtf8("Bienvenidos a mi casa, les presento a mi familia"),    // codespell:ignore
                          DOnlineTranslator::Google,
                          DOnlineTranslator::German,
                          DOnlineTranslator::Spanish);

    do_online_translation(QString::fromUtf8("Non parlo italiano molto bene"),                       // codespell:ignore
                          DOnlineTranslator::Google,
                          DOnlineTranslator::TraditionalChinese,
                          DOnlineTranslator::Italian);

    do_online_translation(QString::fromUtf8("My dog speak very well French"),
                          DOnlineTranslator::Bing,
                          DOnlineTranslator::French,
                          DOnlineTranslator::English);
/*
    do_online_translation(QString::fromUtf8("Mon vélo a crevé ce matin sur la route du travail"),   // codespell:ignore
                          DOnlineTranslator::LibreTranslate,
                          DOnlineTranslator::German,
                          DOnlineTranslator::French);
*/
    do_online_translation(QString::fromUtf8("Hier mon réveil a sonné à 5h00 du matin"),             // codespell:ignore
                          DOnlineTranslator::Lingva,
                          DOnlineTranslator::English,
                          DOnlineTranslator::French);

    do_online_translation(QString::fromUtf8("Les pates c'est bon avec de la sauce tomate"),         // codespell:ignore
                          DOnlineTranslator::Yandex,
                          DOnlineTranslator::Russian,
                          DOnlineTranslator::French);

    return 0;
}
