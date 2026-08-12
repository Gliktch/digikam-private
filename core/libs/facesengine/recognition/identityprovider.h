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

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "digikam_opencv.h"
#include "identity.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT IdentityProvider : public QObject
{
    Q_OBJECT

public:

    IdentityProvider();
    ~IdentityProvider() override;

public:

    static IdentityProvider* instance();
    static QString FaceTrainingVersion;
    static QString ExtractorModel;

public:

    void cancel();

    bool checkRetrainingRequired() const;

    /**
     * @brief Checks the integrity and returns true if everything is fine.
     */
    bool integrityCheck();

    /**
     * @brief Shrinks the database.
     */
    void vacuum();

    /**
     * @return all identities known to the database
     */
    const QList<Identity> allIdentities()                                   const;

    /**
     * @brief retrieves the identity with the given id.
     */
    Identity identity(int id)                                               const;

    /**
     * @brief Finds the first identity with matching attribute - value.
     * @return a null identity if no match is found or attribute is empty.
     */
    Identity findIdentity(const QString& attribute, const QString& value)   const;

    /**
     * @brief Finds the identity matching the given attributes.
     * Attributes are first checked with knowledge of their meaning.
     * Secondly, all unknown attributes are used.
     * @return a null Identity if no match is possible or the map is empty.
     */
    Identity findIdentity(const QMultiMap<QString, QString>& attributes)    const;

    /**
     * @brief Adds a new identity with the specified attributes.
     * Please note that a UUID is automatically generated.
     */
    Identity addIdentity(const QMultiMap<QString, QString>& attributes);

    /**
     * @brief This is the debug version of addIdentity, so the identity is only added
     * to identityCache, but not into the recognition database.
     */
    Identity addIdentityDebug(const QMultiMap<QString, QString>& attributes);

    /**
     * @brief Deletes an identity from the database.
     */
    void deleteIdentity(const Identity& identityToBeDeleted);

    /**
     * @brief Deletes a list of identities from the database.
     */
    void deleteIdentities(QList<Identity> identitiesToBeDeleted);

    /**
     * @brief Renames an identity. Fails if the new name is already in use.
     */
    void renameIdentity(const QString& uuid, const QString& newName);

    /**
     * @brief Deletes the training image for the given hash,
     * leaving the identity as such in the database.
     */
    bool clearTraining(const QString& hash);

    /**
     * @brief Clears all identities and face training
     * from the recognition DB
     */
    void clearAllTraining();

    /**
     * @brief Add the face features and hash to the recognition DB
     * @return the ID of the new row
     */
    int addTraining(const Identity& identity, const QString& hash, const cv::Mat& feature);

    /**
     * @brief Checks if the id exists in the recognition DB
     */
    bool isValidId(int label)                                               const;

protected:

    /**
     * @brief Initializes the identity provider.
     */
    bool initialize();

    /**
     * @brief Deletes a list of identities from the database.
     */
    cv::Ptr<cv::ml::TrainData> getTrainingData()                            const;

    /**
     * @brief add a face matrice to the recognition database
     */
    bool addIdentityFace(const Identity& identity, QString& hash, cv::Mat embedding);

    /**
     * @brief delete a face matrice from the recognition database
     */
    bool deleteIdentityFace(const Identity& identity, QString& hash);

private:

    bool identityContains(const Identity& identity,
                          const QString& attribute,
                          const QString& value)                             const;

    Identity findByAttribute(const QString& attribute,
                             const QString& value)                          const;

    Identity findByAttributes(const QString& attribute,
                              const QMultiMap<QString, QString>& valueMap)  const;

    bool trainingRemoveConcurrent();

private:

    /// @note disabled
    explicit IdentityProvider(QObject*)       = delete;
    IdentityProvider(const IdentityProvider&) = delete;

private:

    class Private;
    Private* const d = nullptr;

private:

    friend class Identity;
    friend class FaceClassifier;
};

} // namespace Digikam
