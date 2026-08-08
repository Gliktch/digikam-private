/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QFile>
#include <QTest>

namespace
{

QString source(const QString& relativePath)
{
    QFile file(QString::fromUtf8(PRIVACY_ANALYSIS_SOURCE_ROOT) + QLatin1Char('/') + relativePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    return QString::fromUtf8(file.readAll());
}

int occurrences(const QString& text, const QString& needle)
{
    int count  = 0;
    int offset = 0;

    while ((offset = text.indexOf(needle, offset)) >= 0)
    {
        ++count;
        offset += needle.size();
    }

    return count;
}

} // namespace

class PrivacyAnalysisWiringTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testBqmCannotBypassGate();
    void testAutomatedFacePipelinesRemainGated();
};

void PrivacyAnalysisWiringTest::testBqmCannotBypassGate()
{
    const QString base = source(QStringLiteral(
        "core/libs/tags/autoassignment/pipelines/autotagspipelinebase.cpp"));
    const QString object = source(QStringLiteral(
        "core/libs/tags/autoassignment/pipelines/object/autotagspipelineobject.cpp"));

    QVERIFY2(!base.isEmpty(), "Unable to read autotag pipeline base source");
    QVERIFY2(!object.isEmpty(), "Unable to read object autotag pipeline source");
    QVERIFY(base.contains(QStringLiteral("PrivacyAnalysisGate::mayAnalyze(info.id())")));
    QVERIFY(object.contains(QStringLiteral("PrivacyAnalysisGate::mayAnalyze(package->info.id())")));
    QVERIFY(!object.contains(QStringLiteral("settings.bqmMode || PrivacyAnalysisGate")));
    QVERIFY(!object.contains(QStringLiteral("!settings.bqmMode &&\n            !PrivacyAnalysisGate")));
    QVERIFY(occurrences(object,
                        QStringLiteral("PrivacyAnalysisGate::mayAnalyze(package->info.id())")) >= 4);
}

void PrivacyAnalysisWiringTest::testAutomatedFacePipelinesRemainGated()
{
    const QString base = source(QStringLiteral(
        "core/utilities/facemanagement/pipelines/facepipelinebase.h"));
    const QString detect = source(QStringLiteral(
        "core/utilities/facemanagement/pipelines/detectrecognize/facepipelinedetectrecognize.cpp"));
    const QString recognize = source(QStringLiteral(
        "core/utilities/facemanagement/pipelines/recognize/facepipelinerecognize.cpp"));
    const QString retrain = source(QStringLiteral(
        "core/utilities/facemanagement/pipelines/retrain/facepipelineretrain.cpp"));

    QVERIFY2(!base.isEmpty(), "Unable to read face pipeline base header");
    QVERIFY2(!detect.isEmpty(), "Unable to read detect/recognize pipeline source");
    QVERIFY2(!recognize.isEmpty(), "Unable to read recognize pipeline source");
    QVERIFY2(!retrain.isEmpty(), "Unable to read retrain pipeline source");
    QCOMPARE(occurrences(base, QStringLiteral("requireAnalysisPermission = true")), 2);
    QVERIFY(occurrences(detect, QStringLiteral("PrivacyAnalysisGate::mayAnalyze")) >= 6);
    QVERIFY(occurrences(recognize, QStringLiteral("PrivacyAnalysisGate::mayAnalyze")) >= 4);
    QVERIFY(occurrences(retrain, QStringLiteral("PrivacyAnalysisGate::mayAnalyze")) >= 2);
}

QTEST_GUILESS_MAIN(PrivacyAnalysisWiringTest)

#include "privacyanalysiswiring_utest.moc"
