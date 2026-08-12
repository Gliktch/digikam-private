/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Face classifier training data provider
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "identityprovider.h"

// Qt includes

#include <QReadWriteLock>
#include <QUuid>
#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrentRun>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "facedboperationgroup.h"
#include "facedbaccess.h"
#include "facedb.h"
#include "facescansettings.h"
#include "faceclassifier.h"
#include "recognitiontrainingupdatequeue.h"
#include "actionthreadbase.h"

namespace Digikam
{

QString IdentityProvider::FaceTrainingVersion = QLatin1String("8.6.0");
QString IdentityProvider::ExtractorModel      = QLatin1String("SFace");

class Q_DECL_HIDDEN IdentityProvider::Private
{
public:

    Private() = default;

public:

    bool                            dbAvailable             = false;
    int                             seedMax                 = 0;

    QHash<int, Identity>            identityCache;

    QReadWriteLock                  trainingLock;
    RecognitionTrainingUpdateQueue* removeQueue;
    QThreadPool*                    removeThreadPool        = nullptr;
    QFuture<bool>                   removeThreadResult;
};

class Q_DECL_HIDDEN IdentityProviderCreator
{
public:

    IdentityProvider object;
};

Q_GLOBAL_STATIC(IdentityProviderCreator, creator)

// ---------------------------------------------------------------------------

IdentityProvider::IdentityProvider()
    : QObject(),
      d      (new Private)
{
    // Save face settings to remove old Detector and Recognizer models.

    FaceScanSettings prm;
    prm.readFromConfig();
    prm.detectModel     = FaceScanSettings::FaceDetectionModel::YuNet;
    prm.recognizeModel  = FaceScanSettings::FaceRecognitionModel::SFace;
    prm.writeToConfig();

    d->removeQueue      = new RecognitionTrainingUpdateQueue();

    // initialize the database

    if (!initialize())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "IdentityProvider::initialize() failed!";
    }

    // Create a thread pool for the training remover.

    d->removeThreadPool = new QThreadPool(this);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    // Priority should be equal or greater than trainer or recognizer threads.

    d->removeThreadPool->setThreadPriority(QThread::NormalPriority);

#endif

    // We only need 1 thread for the training remover.

    d->removeThreadPool->setMaxThreadCount(1);

    // Run the remove queue listener thread.

    d->removeThreadResult = QtConcurrent::run(d->removeThreadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                              &IdentityProvider::trainingRemoveConcurrent,
                                              this

#else

                                              this,
                                              &IdentityProvider::trainingRemoveConcurrent

#endif

                                             );
}

IdentityProvider::~IdentityProvider()
{
    cancel();

    // final cleanup

    delete d;
}

IdentityProvider* IdentityProvider::instance()
{
    return &creator->object;
}

bool IdentityProvider::initialize()
{
    // Initialize the database.

    DbEngineParameters params   = CoreDbAccess::parameters().faceParameters();
    params.setFaceDatabasePath(CoreDbAccess::parameters().faceParameters().getFaceDatabaseNameOrDir());
    FaceDbAccess::setParameters(params, FaceScanSettings::FaceRecognitionModel::SFace);

    // Check if the database is available.

    d->dbAvailable              = FaceDbAccess::checkReadyForUse(nullptr);

    // do an integrity check

    if (!d->dbAvailable || !integrityCheck())
    {
        return false;
    }

    // load the identities

    const auto ids = FaceDbAccess().db()->identities();

    for (const Identity& _identity : ids)
    {
        d->identityCache[_identity.id()] = _identity;
    }

    return true;
}

void IdentityProvider::cancel()
{
    if (d->removeThreadResult.isRunning())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "IdentityProvider::cancel: sent queue end signal";

        // Signal the remove thread to terminate.

        d->removeQueue->push(d->removeQueue->endSignal());

        // Wait for the remove thread to finish.

        d->removeThreadResult.waitForFinished();
    }

    // delete the remove queue

    if (d->removeQueue)
    {
        delete d->removeQueue;
        d->removeQueue = nullptr;
    }
}

bool IdentityProvider::checkRetrainingRequired() const
{
    if (!d->dbAvailable)
    {
        // don't use this if we're not initialized

        qCWarning(DIGIKAM_FACESENGINE_LOG) << "IdentityProvider not initialized or database not available";

        return false;
    }

    // return false if there are no identities in the database

    if (d->identityCache.isEmpty())
    {
        // write the current version to the database

        FaceDbAccess().db()->setTrainingVersionInfo(IdentityProvider::FaceTrainingVersion, IdentityProvider::ExtractorModel);

        return false;
    }

    // Check if the face training version is up-to-date.

    QString version;
    QString model;

    FaceDbAccess().db()->getTrainingVersionInfo(version, model);

    if (
        (IdentityProvider::FaceTrainingVersion == version) &&
        (IdentityProvider::ExtractorModel      == model)
       )
    {
        // retraining not needed

        return false;
    }

    // retraining needed

    return true;
}

bool IdentityProvider::integrityCheck()
{
    if (!d->dbAvailable)
    {
        return false;
    }

    d->trainingLock.lockForRead();

    bool result = FaceDbAccess().db()->integrityCheck();

    d->trainingLock.unlock();

    return result;
}

void IdentityProvider::vacuum()
{
    if (!d->dbAvailable)
    {
        return;
    }

    d->trainingLock.lockForWrite();

    FaceDbAccess().db()->vacuum();

    d->trainingLock.unlock();
}

// -----------------------------------------------------------------------

const QList<Identity> IdentityProvider::allIdentities() const
{
    // Return a copy of the identity cache.

    if (!d->dbAvailable)
    {
        return QList<Identity>();
    }

    d->trainingLock.lockForRead();

    QList<Identity> result = (d->identityCache.values());

    d->trainingLock.unlock();

    return result;
}

Identity IdentityProvider::identity(int id) const
{
    if (!d->dbAvailable)
    {
        return Identity();
    }

    d->trainingLock.lockForRead();

    Identity result = d->identityCache.value(id);

    d->trainingLock.unlock();

    return result;
}

Identity IdentityProvider::findIdentity(const QString& attribute, const QString& value) const
{
    if (!d->dbAvailable || attribute.isEmpty())
    {
        return Identity();
    }

    d->trainingLock.lockForRead();

    Identity result = findByAttribute(attribute, value);

    d->trainingLock.unlock();

    return result;
}

Identity IdentityProvider::findIdentity(const QMultiMap<QString, QString>& attributes) const
{
    if (!d->dbAvailable || attributes.isEmpty())
    {
        return Identity();
    }

    d->trainingLock.lockForRead();

    Identity match;

    // First and foremost, UUID.

    QString uuid = attributes.value(QLatin1String("uuid"));
    match        = findByAttribute(QLatin1String("uuid"), uuid);

    if (!match.isNull())
    {
        d->trainingLock.unlock();

        return match;
    }

    // A negative UUID match, with a given UUID, precludes any further search.

    if (!uuid.isNull())
    {
        d->trainingLock.unlock();

        return Identity();
    }

    // Full name.

    match = findByAttributes(QLatin1String("fullName"), attributes);

    if (!match.isNull())
    {
        d->trainingLock.unlock();

        return match;
    }

    // Name.

    match = findByAttributes(QLatin1String("name"), attributes);

    if (!match.isNull())
    {
        d->trainingLock.unlock();

        return match;
    }

    QMultiMap<QString, QString>::const_iterator it;

    for (it = attributes.begin() ; it != attributes.end() ; ++it)
    {
        if (
            (it.key() == QLatin1String("uuid"))     ||
            (it.key() == QLatin1String("fullName")) ||
            (it.key() == QLatin1String("name"))
           )
        {
            continue;
        }

        match = findByAttribute(it.key(), it.value());

        if (!match.isNull())
        {
            d->trainingLock.unlock();

            return match;
        }
    }

    d->trainingLock.unlock();

    return Identity();
}

Identity IdentityProvider::addIdentity(const QMultiMap<QString, QString>& attributes)
{
    if (!d->dbAvailable)
    {
        return Identity();
    }

    if (attributes.contains(QLatin1String("uuid")))
    {
        d->trainingLock.lockForRead();

        Identity matchByUuid = findIdentity(QLatin1String("uuid"), attributes.value(QLatin1String("uuid")));

        d->trainingLock.unlock();

        if (!matchByUuid.isNull())
        {
            // This situation is not well defined.

            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Called addIdentity with a given UUID, "
                                                "and there is such a UUID already in the database."
                                             << "The existing identity is returned without adjusting properties!";

            return matchByUuid;
        }
    }

    Identity _identity;
    {
        FaceDbOperationGroup group;

        d->trainingLock.lockForWrite();

        int id = FaceDbAccess().db()->addIdentity();
        _identity.setId(id);
        _identity.setAttributesMap(attributes);
        _identity.setAttribute(QLatin1String("uuid"), QUuid::createUuid().toString());

        FaceDbAccess().db()->updateIdentity(_identity);

        d->trainingLock.unlock();
    }

    d->identityCache[_identity.id()] = _identity;

    return _identity;
}

Identity IdentityProvider::addIdentityDebug(const QMultiMap<QString, QString>& attributes)
{
    Identity _identity;
    {
        _identity.setId(attributes.value(QLatin1String("name")).toInt());
        _identity.setAttributesMap(attributes);
        _identity.setAttribute(QLatin1String("uuid"), QUuid::createUuid().toString());
    }

    d->trainingLock.lockForWrite();

    d->identityCache[_identity.id()] = _identity;

    d->trainingLock.unlock();

    return _identity;
}

cv::Ptr<cv::ml::TrainData> IdentityProvider::getTrainingData() const
{
    cv::Ptr<cv::ml::TrainData> trainData = nullptr;

    if (!d->dbAvailable)
    {
        return trainData;
    }

    d->trainingLock.lockForRead();

    trainData = FaceDbAccess().db()->trainData();

    d->trainingLock.unlock();

    return trainData;
}

void IdentityProvider::deleteIdentity(const Identity& identityToBeDeleted)
{
    if (!d->dbAvailable || identityToBeDeleted.isNull())
    {
        return;
    }

    d->trainingLock.lockForWrite();

    FaceDbAccess().db()->deleteIdentity(identityToBeDeleted.id());
    d->identityCache.remove(identityToBeDeleted.id());

    d->trainingLock.unlock();
}

void IdentityProvider::deleteIdentities(QList<Identity> identitiesToBeDeleted)
{
    if (!d->dbAvailable)
    {
        return;
    }

    QList<Identity>::iterator _identity = identitiesToBeDeleted.begin();

    while (_identity != identitiesToBeDeleted.end())
    {
        deleteIdentity(*_identity);

        _identity = identitiesToBeDeleted.erase(_identity);
    }
}

void IdentityProvider::renameIdentity(const QString& uuid, const QString& newName)
{
    if (!d->dbAvailable)
    {
        return;
    }

    Identity _identity = findIdentity(QLatin1String("uuid"), uuid);

    if (!_identity.isNull())
    {
        // remove the old name and fullName attributes

        _identity.deleteAttributeValue(QLatin1String("name"),     _identity.attribute(QLatin1String("name")));
        _identity.deleteAttributeValue(QLatin1String("fullName"), _identity.attribute(QLatin1String("fullName")));

        // add the new name and fullName attributes

        _identity.setAttribute(QLatin1String("name"),     newName);
        _identity.setAttribute(QLatin1String("fullName"), newName);

        // lock for write

        d->trainingLock.lockForWrite();

        // update the identity in the DB

        FaceDbAccess().db()->updateIdentity(_identity);

        // update the identity in the cache

        d->identityCache[_identity.id()] = _identity;

        // unlock

        d->trainingLock.unlock();
    }
}

bool IdentityProvider::clearTraining(const QString& hash)
{
    if (!d->dbAvailable)
    {
        return false;
    }

    d->trainingLock.lockForWrite();

    bool result = FaceDbAccess().db()->removeFaceVector(hash);

    d->trainingLock.unlock();

    return result;
}

void IdentityProvider::clearAllTraining()
{
    if (!d->dbAvailable)
    {
        return;
    }

    d->trainingLock.lockForWrite();

    d->identityCache.clear();
    FaceDbAccess().db()->clearIdentities();
    FaceDbAccess().db()->clearDNNTraining();

    d->trainingLock.unlock();
}

int IdentityProvider::addTraining(const Identity& identity, const QString& hash, const cv::Mat& feature)
{
    if (!d->dbAvailable)
    {
        return -1;
    }

    d->trainingLock.lockForWrite();

    int result = FaceDbAccess().db()->insertFaceVector(feature, identity.id(), hash);

    d->trainingLock.unlock();

    return result;
}

bool IdentityProvider::isValidId(int label) const
{
/*
    return ((label > d->seedMax) && d->identityCache.contains(label));
*/
    return (d->identityCache.contains(label));
}

//--------------------------------------------------------------
// private methods

/**
 * @note Takes care that there may be multiple values of attribute in identity's attributes.
 */
bool IdentityProvider::identityContains(const Identity& _identity,
                                        const QString&  attribute,
                                        const QString&  value) const
{
    d->trainingLock.lockForRead();

    const QMultiMap<QString, QString> map          = _identity.attributesMap();
    QMultiMap<QString, QString>::const_iterator it = map.constFind(attribute);

    for ( ; (it != map.constEnd()) && (it.key() == attribute) ; ++it)
    {
        if (it.value() == value)
        {
            d->trainingLock.unlock();
            return true;
        }
    }

    d->trainingLock.unlock();

    return false;
}

Identity IdentityProvider::findByAttribute(const QString& attribute,
                                           const QString& value) const
{
    d->trainingLock.lockForRead();

    for (const Identity& _identity : std::as_const(d->identityCache))
    {
        if (identityContains(_identity, attribute, value))
        {
            // cppcheck-suppress useStlAlgorithm
            d->trainingLock.unlock();

            return _identity;
        }
    }

    d->trainingLock.unlock();

    return Identity();
}

/**
 * @note Takes care that there may be multiple values of attribute in valueMap.
 */
Identity IdentityProvider::findByAttributes(const QString& attribute,
                                            const QMultiMap<QString, QString>& valueMap) const
{
    QMultiMap<QString, QString>::const_iterator it = valueMap.find(attribute);

    d->trainingLock.lockForRead();

    for ( ; (it != valueMap.end()) && (it.key() == attribute) ; ++it)
    {
        for (const Identity& _identity : std::as_const(d->identityCache))
        {
            if (identityContains(_identity, attribute, it.value()))
            {
                // cppcheck-suppress useStlAlgorithm
                d->trainingLock.unlock();

                return _identity;
            }
        }
    }

    d->trainingLock.unlock();

    return Identity();
}

bool IdentityProvider::trainingRemoveConcurrent()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    QString hash;

    while (true && d->removeQueue)
    {
        hash = d->removeQueue->pop_front();

        if (d->removeQueue && (d->removeQueue->endSignal() != hash))
        {
            clearTraining(hash);
            hash.clear();

            FaceClassifier::instance()->retrain();
        }
        else
        {
            // send the end signal if anyone else is listening

            d->removeQueue->push(d->removeQueue->endSignal());

            break;
        }
    }

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "IdentityProvider::trainingRemoveConcurrent thread exited";

    return true;
}

} // namespace Digikam

#include "moc_identityprovider.cpp"
