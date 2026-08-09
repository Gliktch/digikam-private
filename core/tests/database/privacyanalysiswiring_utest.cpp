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
    void testProtectedMutationsCannotBypassDioGate();
    void testProtectedPublicWritesRemainGated();
    void testEditorSavesCannotOverwriteProtectedItems();
    void testUnlockedOriginalsUseRevocableMemorySources();
    void testPreparedPluginSourcesOwnLifetime();
    void testDefaultExternalOpenUsesWritableCheckout();
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
    const QString settings = source(QStringLiteral(
        "core/app/main/privacycategorysettingsdialog.cpp"));
    const QString guard = source(QStringLiteral(
        "core/app/main/privacycompatibilityguard_main.cpp"));
    const QString runtime = source(QStringLiteral(
        "core/libs/database/privacy/privacyruntime.cpp"));
    const QString main = source(QStringLiteral("core/app/main/main.cpp"));

    QVERIFY2(!owner.isEmpty(), "Unable to read still-item transaction owner");
    QVERIFY2(!itemView.isEmpty(), "Unable to read item-view action source");
    QVERIFY2(!settings.isEmpty(), "Unable to read category settings source");
    QVERIFY2(!guard.isEmpty(), "Unable to read Compatibility guard source");
    QVERIFY2(!runtime.isEmpty(), "Unable to read privacy runtime source");
    QVERIFY2(!main.isEmpty(), "Unable to read application main source");
    QVERIFY(owner.contains(QStringLiteral(
        "d->engine.compatibilityUnlock(request, password)")));
    QVERIFY(owner.contains(QStringLiteral(
        "setCompatibilityGuardArmHook(")));
    QVERIFY(owner.contains(QStringLiteral(
        "PrivacyCompatibilityExposureGuardEngine::relock(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->engine.recover(*root, unlock->uuid)")));
    QVERIFY(owner.contains(QStringLiteral("runWithUnlockedSecret(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->engine.compatibilityUnlockBatch(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->engine.compatibilityRelockBatch(")));
    QVERIFY(owner.contains(QStringLiteral(
        "compatibilityGuardProcesses.value(root.uuid)")));
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
    QVERIFY(settings.contains(QStringLiteral(
        "owner->compatibilityUnlockCategory(")));
    QVERIFY(settings.contains(QStringLiteral(
        "owner->compatibilityRelockCategory(")));
    QVERIFY(settings.contains(QStringLiteral("Screen lock and ")));
    QVERIFY(settings.contains(QStringLiteral(
        "system suspend will not relock them")));
    QVERIFY(settings.contains(QStringLiteral("failure of both ")));
    QVERIFY(settings.contains(QStringLiteral(
        "digiKam and its guard")));
    QVERIFY(settings.contains(QStringLiteral(
        "remain publicly exposed until explicitly relocked")));
    QVERIFY(guard.contains(QStringLiteral("all-compatibility")));
    QVERIFY(guard.contains(QStringLiteral("store->transactionUuids(")));
    QVERIFY(owner.contains(QStringLiteral(
        "PrivacyThreadImageIOStillItemTransactionOwner::prepareForShutdown()")));
    QVERIFY(owner.contains(QStringLiteral("compatibilityRelockAll().succeeded()")));
    const int orderlyRelock = runtime.indexOf(QStringLiteral(
        "shutdownRuntime->prepareForShutdown()"));
    const int replacementRuntime = runtime.indexOf(QStringLiteral(
        "new PrivacyRuntimeCoordinator"), orderlyRelock);
    const int privacyReset = main.indexOf(QStringLiteral(
        "PrivacyStartupRecovery::reset()"));
    const int databaseCleanup = main.indexOf(QStringLiteral(
        "CoreDbAccess::cleanUpDatabase()"), privacyReset);
    QVERIFY(orderlyRelock >= 0);
    QVERIFY(replacementRuntime > orderlyRelock);
    QVERIFY(privacyReset >= 0);
    QVERIFY(databaseCleanup > privacyReset);
}

void PrivacyAnalysisWiringTest::testProtectedMutationsCannotBypassDioGate()
{
    const QString dio = source(QStringLiteral(
        "core/libs/database/utils/ifaces/dio.cpp"));
    const QString runtime = source(QStringLiteral(
        "core/libs/database/privacy/privacyruntime.cpp"));

    QVERIFY2(!dio.isEmpty(), "Unable to read DIO source");
    QVERIFY2(!runtime.isEmpty(), "Unable to read privacy runtime source");
    QVERIFY(dio.contains(QStringLiteral(
        "PrivacyActionGate::classify(request)")));
    QVERIFY(dio.contains(QStringLiteral(
        "PrivacyActionKind::MoveRenameDelete")));
    QVERIFY(dio.contains(QStringLiteral(
        "PrivacyMutationPolicy::DestructiveMutation")));
    QCOMPARE(occurrences(dio, QStringLiteral("!allowPrivacyMutation(")), 6);
    QVERIFY(dio.contains(QStringLiteral(
        "!allowPrivacyMutation(trackedItemInfos(srcList))")));
    QVERIFY(dio.contains(QStringLiteral(
        "(operation == IOJobData::MoveImage) &&\n"
        "            !allowPrivacyMutation(data->itemInfos())")));
    QCOMPARE(occurrences(runtime,
                         QStringLiteral("PrivacyActionGate::setProvider(runtime)")), 2);
}

void PrivacyAnalysisWiringTest::testProtectedPublicWritesRemainGated()
{
    const QString metadataHub = source(QStringLiteral(
        "core/libs/fileactionmanager/metadatahub.cpp"));
    const QString fileWorker = source(QStringLiteral(
        "core/libs/fileactionmanager/fileworkeriface.cpp"));
    const QString focusPoints = source(QStringLiteral(
        "core/utilities/focuspointmanagement/focuspointgroup.cpp"));
    const QString metadataPlugin = source(QStringLiteral(
        "core/dplugins/generic/metadata/metadataedit/dialog/metadataeditdialog.cpp"));

    QVERIFY2(!metadataHub.isEmpty(), "Unable to read MetadataHub source");
    QVERIFY2(!fileWorker.isEmpty(), "Unable to read file worker source");
    QVERIFY2(!focusPoints.isEmpty(), "Unable to read focus-point source");
    QVERIFY2(!metadataPlugin.isEmpty(), "Unable to read metadata plugin source");
    QVERIFY(metadataHub.contains(QStringLiteral(
        "PrivacyActionKind::MetadataWrite")));
    QVERIFY(occurrences(metadataHub,
                        QStringLiteral("privacyAllowsMetadataWrite(")) >= 7);
    QCOMPARE(occurrences(fileWorker,
                         QStringLiteral("PrivacyActionGate::mayMutatePublicItem(")), 2);
    QVERIFY(fileWorker.contains(QStringLiteral("PrivacyActionKind::MetadataWrite")));
    QVERIFY(fileWorker.contains(QStringLiteral("PrivacyActionKind::InternalEdit")));
    QCOMPARE(occurrences(focusPoints,
                         QStringLiteral("PrivacyActionGate::mayMutatePublicItem(")), 1);
    QVERIFY(!metadataPlugin.contains(QStringLiteral("privacyactionpolicy.h")));
}

void PrivacyAnalysisWiringTest::testEditorSavesCannotOverwriteProtectedItems()
{
    const QString editorWindow = source(QStringLiteral(
        "core/utilities/imageeditor/editor/editorwindow.cpp"));
    const QString imageWindow = source(QStringLiteral(
        "core/utilities/imageeditor/main/imagewindow.cpp"));

    QVERIFY2(!editorWindow.isEmpty(), "Unable to read editor-window source");
    QVERIFY2(!imageWindow.isEmpty(), "Unable to read image-window source");
    QVERIFY(occurrences(editorWindow,
                        QStringLiteral("!mayCommitPublicFile(")) >= 4);
    QVERIFY(editorWindow.contains(QStringLiteral(
        "VersionFileOperation::SaveAndDelete")));
    QVERIFY(editorWindow.contains(QStringLiteral(
        "QFile::remove(m_savingContext.saveTempFileName)")));
    QCOMPARE(occurrences(imageWindow,
                         QStringLiteral("PrivacyActionGate::mayMutatePublicItem(")), 1);
    QVERIFY(imageWindow.contains(QStringLiteral("PrivacyActionKind::InternalEdit")));
    QVERIFY(imageWindow.contains(QStringLiteral("Cannot Save Protected Item")));
}

void PrivacyAnalysisWiringTest::testUnlockedOriginalsUseRevocableMemorySources()
{
    const QString albumManager = source(QStringLiteral(
        "core/libs/album/manager/albummanager_database.cpp"));
    const QString sessionOwner = source(QStringLiteral(
        "core/libs/database/privacy/privacycategorysessionowner.cpp"));

    QVERIFY2(!albumManager.isEmpty(), "Unable to read album-manager source");
    QVERIFY2(!sessionOwner.isEmpty(), "Unable to read session-owner source");
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacyCasualOriginalReader reader")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "m_sessions->runWithUnlockedSecret(")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "QLatin1String(\"/proc/self/fd/\")")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacySourceResult::MemoryOnly")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "MaximumMaterializedOriginalBytes")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacyCacheTransition::begin(")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacyCacheTransition::purge(")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacyCacheTransition::finish(")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacyCacheTransition::rollback(")));
    QVERIFY(sessionOwner.contains(QStringLiteral(
        "if (!d->presentationAvailability(categoryUuid, false))")));
}

void PrivacyAnalysisWiringTest::testPreparedPluginSourcesOwnLifetime()
{
    const QString interface = source(QStringLiteral(
        "core/libs/dplugins/iface/dinfointerface.h"));
    const QString databaseInterface = source(QStringLiteral(
        "core/libs/database/utils/ifaces/dbinfoiface.cpp"));
    const QString accessBroker = source(QStringLiteral(
        "core/libs/database/utils/ifaces/privacyitemaccessbroker.cpp"));
    const QString albumManager = source(QStringLiteral(
        "core/libs/album/manager/albummanager_database.cpp"));
    const QString runtime = source(QStringLiteral(
        "core/libs/database/privacy/privacyruntime.cpp"));
    const QString exportWindow = source(QStringLiteral(
        "core/dplugins/generic/webservices/filecopy/fcexportwindow.cpp"));
    const QString exportTask = source(QStringLiteral(
        "core/dplugins/generic/webservices/filecopy/fctask.cpp"));
    const QString printFinalPage = source(QStringLiteral(
        "core/dplugins/generic/tools/printcreator/wizard/advprintfinalpage.cpp"));
    const QString printTask = source(QStringLiteral(
        "core/dplugins/generic/tools/printcreator/manager/advprinttask.cpp"));

    QVERIFY2(!interface.isEmpty(), "Unable to read generic item interface");
    QVERIFY2(!databaseInterface.isEmpty(), "Unable to read DB item interface");
    QVERIFY2(!accessBroker.isEmpty(), "Unable to read privacy access broker");
    QVERIFY2(!albumManager.isEmpty(), "Unable to read album-manager source");
    QVERIFY2(!runtime.isEmpty(), "Unable to read privacy runtime source");
    QVERIFY2(!exportWindow.isEmpty(), "Unable to read File Copy window");
    QVERIFY2(!exportTask.isEmpty(), "Unable to read File Copy task");
    QVERIFY2(!printFinalPage.isEmpty(), "Unable to read Print Creator final page");
    QVERIFY2(!printTask.isEmpty(), "Unable to read Print Creator task");
    QVERIFY(interface.contains(QStringLiteral(
        "virtual QSharedPointer<DItemAccessHandle> prepareItemAccess")));
    QVERIFY(interface.contains(QStringLiteral(
        "virtual bool validateAccess(const QUrl& physicalUrl) const")));
    QVERIFY(interface.contains(QStringLiteral(
        "DItemAccessConsumerScope consumerScope")));
    QVERIFY(databaseInterface.contains(QStringLiteral(
        "return preparePrivacyItemAccess(request)")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "PrivacySourceResolver::resolve(sourceRequest)")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "new PrivacySourceUseGuard(logicalPath, source)")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "PrivacyPreparedAccessRegistry::acquire(categories)")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "brokerSupportsProtectedRequest(request)")));
    QVERIFY(!accessBroker.contains(QStringLiteral(
        "rootContainsProtectedItems")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "asset.publicRelativePath")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "journal.journalRelativePath")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "snapshotVerifiedPublicProxy(")));
    QVERIFY(albumManager.contains(QStringLiteral(
        "PrivacyPreparedAccessQuiesceGuard preparedAccessQuiesce")));
    QCOMPARE(occurrences(runtime, QStringLiteral(
        "PrivacyPreparedAccessQuiesceGuard preparedAccessQuiesce")), 2);
    QVERIFY(accessBroker.contains(QStringLiteral(
        "class PreparedAccessLifetime")));
    QCOMPARE(occurrences(accessBroker, QStringLiteral(
        "m_lifetime(lifetime)")), 2);
    QVERIFY(accessBroker.contains(QStringLiteral(
        "leases->validate(prepared->lease)")));
    QCOMPARE(occurrences(albumManager, QStringLiteral(
        "PrivacyPreparedAccessRegistry::hasActiveAccess(categoryUuid)")), 2);
    QVERIFY(exportWindow.contains(QStringLiteral(
        "d->iface->prepareItemAccess(accessRequest)")));
    QVERIFY(exportWindow.contains(QStringLiteral(
        "d->preparedAccess = prepared")));
    QVERIFY(exportWindow.contains(QStringLiteral(
        "DItemAccessConsumerScope::SameProcess")));
    QVERIFY(exportWindow.contains(QStringLiteral(
        "DItemAccessConsumerScope::DetachedProcess")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "accessHandle->acquireSource(logicalUrl, cancellation)")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "d->accessHandle.clear()")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "source->validateAccess()")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "source->associatedEntries()")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "d->settings.iface->itemInfo(logicalUrl)")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "DFileOperations::copyFileCancellable(")));
    QVERIFY(exportTask.contains(QStringLiteral(
        "DFileOperations::setPermissionsAndModificationTime(")));
    QVERIFY(printFinalPage.contains(QStringLiteral(
        "d->iface->prepareItemAccess(request)")));
    QVERIFY(printFinalPage.contains(QStringLiteral(
        "DItemAccessPurpose::Print")));
    QVERIFY(printFinalPage.contains(QStringLiteral(
        "d->preparedAccess = prepared")));
    QVERIFY(printTask.contains(QStringLiteral(
        "d->accessHandle->acquireSource(photo->m_url, d->cancellation)")));
    QVERIFY(printTask.contains(QStringLiteral(
        "source->validateAccess()")));
    QVERIFY(printTask.contains(QStringLiteral(
        "accessiblePhotos()")));
    QVERIFY(accessBroker.contains(QStringLiteral(
        "fileFactsForAsset(asset)")));
}

void PrivacyAnalysisWiringTest::testDefaultExternalOpenUsesWritableCheckout()
{
    const QString owner = source(QStringLiteral(
        "core/app/main/privacythreadimagestillitemtransactionowner.cpp"));
    const QString utilities = source(QStringLiteral(
        "core/app/items/utils/itemviewutilities.cpp"));

    QVERIFY2(!owner.isEmpty(), "Unable to read checkout transaction owner");
    QVERIFY2(!utilities.isEmpty(), "Unable to read item-view utilities");
    QVERIFY(owner.contains(QStringLiteral(
        "PrivacyThreadImageIOStillItemTransactionOwner::prepareExternalOpen(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->archive.restoreMember(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->checkoutEngine.create(request)")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->checkoutEngine.authorizeLaunch(")));
    QVERIFY(owner.contains(QStringLiteral(
        "d->checkoutEngine.reconcile(")));
    QVERIFY(owner.contains(QStringLiteral(
        "transaction.type == PrivacyTransactionType::ExternalCheckout")));
    QVERIFY(utilities.contains(QStringLiteral(
        "owner->prepareExternalOpen(info, *secret)")));
    QVERIFY(utilities.contains(QStringLiteral(
        "asset.checkoutUrl")));
    QVERIFY(utilities.contains(QStringLiteral(
        "asset.role == PrivacyAsset::PrimaryMediaRole")));
    QVERIFY(utilities.contains(QStringLiteral(
        "ExternalApplicationRiskAcknowledged")));
    QVERIFY(utilities.contains(QStringLiteral(
        "The external application may create recent-file records")));
}

QTEST_GUILESS_MAIN(PrivacyAnalysisWiringTest)

#include "privacyanalysiswiring_utest.moc"
