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
    void testManualTagsCannotBypassVisibility();
    void testTagQueriesAndPeopleListingsConsumeVisibility();
    void testCompatibilityActionsUseTransactionOwner();
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

void PrivacyAnalysisWiringTest::testManualTagsCannotBypassVisibility()
{
    const QString itemTags = source(QStringLiteral(
        "core/libs/database/item/containers/iteminfo_tags.cpp"));

    QVERIFY2(!itemTags.isEmpty(), "Unable to read ItemInfo tag source");
    QVERIFY(itemTags.contains(QStringLiteral(
        "PrivacyManualTagVisibilityGate::mayAccess(imageId)")));
    QVERIFY(occurrences(itemTags,
                        QStringLiteral("privacyMayAccessManualTags(m_data->id)")) >= 5);

    const int visibilityCheck = itemTags.indexOf(QStringLiteral(
        "if (!privacyMayAccessManualTags(m_data->id))"));
    const int cacheRead = itemTags.indexOf(QStringLiteral("RETURN_IF_CACHED(tagIds)"));

    QVERIFY(visibilityCheck >= 0);
    QVERIFY(cacheRead > visibilityCheck);
}

void PrivacyAnalysisWiringTest::testTagQueriesAndPeopleListingsConsumeVisibility()
{
    const QString queryBuilder = source(QStringLiteral(
        "core/libs/database/item/query/itemquerybuilder.cpp"));
    const QString tagLister = source(QStringLiteral(
        "core/libs/database/item/lister/itemlister_talbum.cpp"));
    const QString coreDb = source(QStringLiteral(
        "core/libs/database/coredb/coredb.cpp"));
    const QString tagPair = source(QStringLiteral(
        "core/libs/database/item/containers/itemtagpair.cpp"));
    const QString faceEditor = source(QStringLiteral(
        "core/libs/database/tags/facetagseditor.cpp"));
    const QString sessionOwner = source(QStringLiteral(
        "core/libs/database/privacy/privacycategorysessionowner.cpp"));
    const QString albumManager = source(QStringLiteral(
        "core/libs/album/manager/albummanager_talbum.cpp"));

    QVERIFY2(!queryBuilder.isEmpty(), "Unable to read item query builder source");
    QVERIFY2(!tagLister.isEmpty(), "Unable to read tag lister source");
    QVERIFY2(!coreDb.isEmpty(), "Unable to read CoreDB source");
    QVERIFY2(!tagPair.isEmpty(), "Unable to read ItemTagPair source");
    QVERIFY2(!faceEditor.isEmpty(), "Unable to read FaceTagsEditor source");
    QVERIFY2(!sessionOwner.isEmpty(), "Unable to read category session owner source");
    QVERIFY2(!albumManager.isEmpty(), "Unable to read album manager source");
    QVERIFY(queryBuilder.contains(QStringLiteral(
        "PrivacyManualTagVisibilityGate::queryState")));
    QVERIFY(occurrences(queryBuilder,
                        QStringLiteral("manualTagVisibilitySql(")) >= 14);
    QVERIFY(occurrences(tagLister,
                        QStringLiteral("PrivacyManualTagVisibilityGate::mayAccess")) >= 2);
    QVERIFY(coreDb.contains(QStringLiteral(
        "PrivacyManualTagVisibilityGate::queryState")));
    QVERIFY(occurrences(coreDb,
                        QStringLiteral("manualTagVisibilitySql(")) >= 6);
    QVERIFY(tagPair.contains(QStringLiteral(
        "PrivacyManualTagVisibilityGate::mayAccess(info.id())")));
    QVERIFY(occurrences(faceEditor,
                        QStringLiteral("PrivacyManualTagVisibilityGate::mayAccess")) >= 10);
    QVERIFY(sessionOwner.contains(QStringLiteral(
        "ImageTagChangeset::VisibilityChanged")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "case ImageTagChangeset::VisibilityChanged")));
}

void PrivacyAnalysisWiringTest::testCompatibilityActionsUseTransactionOwner()
{
    const QString owner = source(QStringLiteral(
        "core/app/main/privacythreadimagestillitemtransactionowner.cpp"));
    const QString itemView = source(QStringLiteral(
        "core/app/views/stack/itemiconview_views.cpp"));

    QVERIFY2(!owner.isEmpty(), "Unable to read still-item transaction owner");
    QVERIFY2(!itemView.isEmpty(), "Unable to read item-view action source");
    QVERIFY(owner.contains(QStringLiteral(
        "d->engine.compatibilityUnlock(request, password)")));
    QVERIFY(owner.contains(QStringLiteral(
        "setCompatibilityGuardArmHook(")));
    QVERIFY(owner.contains(QStringLiteral(
        "PrivacyCompatibilityExposureGuardEngine::relock(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->engine.recover(*root, unlock->uuid)")));
    QVERIFY(owner.contains(QStringLiteral("runWithUnlockedSecret(")));
    QVERIFY(itemView.contains(QStringLiteral(
        "privacyOwner->compatibilityUnlock(")));
    QVERIFY(itemView.contains(QStringLiteral(
        "privacyOwner->compatibilityRelock(")));
    QVERIFY(itemView.contains(QStringLiteral("Screen lock")));
    QVERIFY(itemView.contains(QStringLiteral(
        "will not automatically relock this exposure")));
    QVERIFY(itemView.contains(QStringLiteral("crash or power loss")));
    QVERIFY(itemView.contains(QStringLiteral(
        "preserve it and require reconciliation rather than overwrite")));
}

QTEST_GUILESS_MAIN(PrivacyAnalysisWiringTest)

#include "privacyanalysiswiring_utest.moc"
