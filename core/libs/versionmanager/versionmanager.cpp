/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "versionmanager.h"

// Qt includes

#include <QDir>
#include <QUrl>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dimgfiltermanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN VersionNameCreator
{
public:

    VersionNameCreator(const VersionFileInfo& lFile,
                       const DImageHistory& rInitHisto,
                       const DImageHistory& cHisto,
                       VersionManager* const qq);

    void checkNeedNewVersion();
    void fork();
    void setSaveDirectory();
    void setSaveFormat();
    void setSaveFileName();
    void setSaveDirectory(const QString& path);
    void setSaveFormat(const QString& format);
    void setSaveFileName(const QString& name);
    void initOperation();
    void checkIntermediates();

protected:

    VersionFileInfo nextIntermediate(const QString& format);
    void setFileSuffix(QString& fileName, const QString& format) const;
    void addFileSuffix(QString& baseName, const QString& format, const QString& originalName = QString()) const;

public:

    VersionManagerSettings settings;

    VersionFileInfo        result;
    VersionFileInfo        loadedFile;

    VersionFileOperation   operation;

    const DImageHistory    resolvedInitialHistory;
    const DImageHistory    currentHistory;

    bool                   fromRaw                  = false;
    bool                   newVersion               = false;

    QVariant               version;
    QVariant               intermediateCounter;
    QString                baseName;
    QString                intermediatePath;

    VersionManager* const  q                        = nullptr;
};

VersionNameCreator::VersionNameCreator(const VersionFileInfo& lFile,
                                       const DImageHistory& rInitHisto,
                                       const DImageHistory& cHisto,
                                       VersionManager* const qq)
    : settings              (qq->settings()),
      loadedFile            (lFile),
      resolvedInitialHistory(rInitHisto),
      currentHistory        (cHisto),
      q                     (qq)
{
    loadedFile.format   = lFile.format.toUpper();
    fromRaw             = (lFile.format.startsWith(QLatin1String("RAW"))); // also accept RAW-... format
    version             = q->namingScheme()->initialCounter();
    intermediateCounter = q->namingScheme()->initialCounter();
}

void VersionNameCreator::checkNeedNewVersion()
{
    // First we check if we have any other files available.
    // The resolved initial history contains only referred files found in the collection
    // Note: The loaded file will have type Current

    qCDebug(DIGIKAM_GENERAL_LOG) << resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Original)
                                 << resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Intermediate)
                                 << fromRaw << q->workspaceFileFormats().contains(loadedFile.format);

    if      (
             !resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Original) &&
             !resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Intermediate)
            )
    {
        newVersion = true;
    }
    else if (fromRaw || !q->workspaceFileFormats().contains(loadedFile.format))
    {
        // We check the loaded format: If it is not one of the workspace formats, or even raw, we need a new version

        newVersion = true;
    }
    else
    {
        newVersion = false;
    }
}

void VersionNameCreator::fork()
{
    newVersion = true;
}

void VersionNameCreator::setSaveDirectory()
{
    result.path      = q->namingScheme()->directory(loadedFile.path, loadedFile.fileName);
    intermediatePath = q->namingScheme()->directory(loadedFile.path, loadedFile.fileName);
}

void VersionNameCreator::setSaveDirectory(const QString& path)
{
    result.path      = path;
    intermediatePath = path;
}

void VersionNameCreator::setSaveFormat()
{
    result.format = settings.format;
/*
    if (fromRaw)
    {
        result.format = settings.formatForStoringRAW;
    }
    else
    {
        result.format = settings.formatForStoringSubversions;
    }
*/
}

void VersionNameCreator::setSaveFormat(const QString& override)
{
    result.format = override;
}

void VersionNameCreator::setSaveFileName()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "need new version" << newVersion;

    VersionNamingScheme* const scheme = q->namingScheme();

    // initialize baseName, version, and intermediateCounter for intermediates

    baseName                        = scheme->baseName(loadedFile.path,
                                                         loadedFile.fileName,
                                                         &version,
                                                         &intermediateCounter);

    qCDebug(DIGIKAM_GENERAL_LOG) << "analyzing file" << loadedFile.fileName << version << intermediateCounter;

    // Update the result format for TIFF and JPG to the existing original suffix in short or long format.

    int lastDot          = loadedFile.fileName.lastIndexOf(QLatin1Char('.'));
    const bool tifFormat = (result.format == QLatin1String("TIFF"));
    const bool jpgFormat = (result.format == QLatin1String("JPG"));

    if ((lastDot != -1) && (tifFormat || jpgFormat))
    {
        int extSize    = loadedFile.fileName.size() - lastDot - 1;
        QString suffix = loadedFile.fileName.right(extSize).toUpper();

        if (
            ((suffix == QLatin1String("TIF"))  && tifFormat) ||
            ((suffix == QLatin1String("JPEG")) && jpgFormat)
           )
        {
            result.format = suffix;
        }
    }

    if (!newVersion)
    {
        result.fileName = loadedFile.fileName;

        if (loadedFile.format != result.format)
        {
            setFileSuffix(result.fileName, result.format);
        }
    }
    else
    {
        QDir dirInfo(result.path);

        // To find the right number for the new version, go through all the items in the given dir,
        // the version number won't be bigger than count()

        for (uint i = 0 ; i <= dirInfo.count() ; ++i)
        {
            QString suggestedName = scheme->versionFileName(result.path, baseName, version);

            // Note: Always give a hard guarantee that the file does not exist

            if (dirInfo.entryList(QStringList() << suggestedName + QLatin1String(".*"), QDir::Files).isEmpty())
            {
                result.fileName = suggestedName;
                addFileSuffix(result.fileName, result.format, loadedFile.fileName);
                break;
            }

            // increment for next attempt

            version = scheme->incrementedCounter(version);
        }
    }
}

void VersionNameCreator::setSaveFileName(const QString& fileName)
{
    result.fileName = fileName;
    baseName        = fileName.section(QLatin1Char('.'), 0, 0);

    // version remains unknown
}

void VersionNameCreator::initOperation()
{
    operation.loadedFile = loadedFile;
    operation.saveFile   = result;

    if (newVersion)
    {
        operation.tasks |= VersionFileOperation::NewFile;
    }
    else
    {
        if (result.fileName == loadedFile.fileName)
        {
            operation.tasks |= VersionFileOperation::Replace;
        }
        else
        {
            operation.tasks |= VersionFileOperation::SaveAndDelete;
        }
    }
}

void VersionNameCreator::checkIntermediates()
{
    // call when task has been determined

    qCDebug(DIGIKAM_GENERAL_LOG) << "Will replace" << bool(operation.tasks & VersionFileOperation::Replace)
                                 << "save after each session" << bool(settings.saveIntermediateVersions & VersionManagerSettings::AfterEachSession)
                                 << "save after raw" << bool(settings.saveIntermediateVersions & VersionManagerSettings::AfterRawConversion)
                                 << "save when not repro" << bool(settings.saveIntermediateVersions & VersionManagerSettings::WhenNotReproducible);

    if (
        (settings.saveIntermediateVersions & VersionManagerSettings::AfterEachSession) &&
        (operation.tasks & VersionFileOperation::Replace)
       )
    {
        // We want a snapshot after each session. The main file will be overwritten by the current state.
        // So we consider the current file as snapshot of the last session and move
        // it to an intermediate before it is overwritten

        operation.tasks |= VersionFileOperation::MoveToIntermediate;
        operation.intermediateForLoadedFile = nextIntermediate(loadedFile.format);

        // this amounts to storing firstStep - 1
    }

    // These are, inclusively, the first and last step the state after which we may have to store.
    // resolvedInitialHistory.size() - 1 is the loaded file
    // currentHistory.size() - 1 is the current version

    int firstStep = resolvedInitialHistory.size();
    int lastStep  = currentHistory.size() - 2; // index of last but one entry

    qCDebug(DIGIKAM_GENERAL_LOG) << "initial history" << resolvedInitialHistory.size()
                                 << "current history" << currentHistory.size()
                                 << "first step" << firstStep << "last step" << lastStep;

    if (lastStep < firstStep)
    {
        // only a single editing step, or even "backwards" in history (possible with redo)

        return;
    }

    if (!firstStep)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid history: resolved initial history has no entries";
        firstStep = 1;
    }

    if (settings.saveIntermediateVersions & VersionManagerSettings::AfterRawConversion)
    {
        int rawConversionStep = -1;

        for (int i = firstStep ; i <= lastStep ; ++i)
        {
            if (DImgFilterManager::instance()->isRawConversion(currentHistory.action(i).identifier()))
            {
                rawConversionStep = i;

                // break? multiple raw conversion steps??
            }
        }

        if (rawConversionStep != -1)
        {
            operation.intermediates.insert(rawConversionStep, VersionFileInfo());
        }
    }

    if (settings.saveIntermediateVersions & VersionManagerSettings::WhenNotReproducible)
    {
        for (int i = firstStep ; i <= lastStep ; ++i)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "step" << i
                                         << "is reproducible"
                                         << (currentHistory.action(i).category() == FilterAction::ReproducibleFilter);

            switch (currentHistory.action(i).category())
            {
                case FilterAction::ReproducibleFilter:
                {
                    break;
                }

                case FilterAction::ComplexFilter:
                case FilterAction::DocumentedHistory:
                {
                    operation.intermediates.insert(i, VersionFileInfo());
                    break;
                }
            }
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Save intermediates after steps" << operation.intermediates.keys();

    if (!operation.intermediates.isEmpty())
    {
        operation.tasks |= VersionFileOperation::StoreIntermediates;

        // now all steps are available, already ordered thanks to QMap. Just fill in the empty VersionFileInfos.

        QMap<int,VersionFileInfo>::iterator it;

        for (it = operation.intermediates.begin() ; it != operation.intermediates.end() ; ++it)
        {
            it.value() = nextIntermediate(result.format);
        }
    }
}

VersionFileInfo VersionNameCreator::nextIntermediate(const QString& format)
{
    VersionNamingScheme* const scheme = q->namingScheme();

    VersionFileInfo intermediate;
    intermediate.path                 = intermediatePath;
    intermediate.format               = format;

    QDir dirInfo(intermediatePath);

    for (uint i = 0 ; i <= dirInfo.count() ; ++i)
    {
        QString suggestedName = scheme->intermediateFileName(intermediatePath, baseName,
                                                             version, intermediateCounter);

        // it is important to increment before returning - we may have to produce a number of files

        intermediateCounter = scheme->incrementedCounter(intermediateCounter);

        // Note: Always give a hard guarantee that the file does not exist

        if (dirInfo.entryList(QStringList() << suggestedName + QLatin1String(".*"), QDir::Files).isEmpty())
        {
            intermediate.fileName = suggestedName;
            setFileSuffix(intermediate.fileName, format);
            break;
        }
    }

    return intermediate;
}

void VersionNameCreator::setFileSuffix(QString& fileName, const QString& format) const
{
    if (fileName.isEmpty())
    {
        return;
    }

    // QFileInfo uses the same string manipulation

    int lastDot  = fileName.lastIndexOf(QLatin1Char('.'));
    bool isLower = true;

    if (lastDot == -1)
    {
        fileName += QLatin1Char('.');
        lastDot   = fileName.size() - 1;
    }
    else
    {
        isLower = fileName.at(fileName.size() - 1).isLower();
    }

    int suffixBegin = lastDot + 1;

    if (fileName.mid(suffixBegin).compare(format, Qt::CaseInsensitive))
    {
        fileName.replace(suffixBegin, fileName.size() - suffixBegin, isLower ? format.toLower() : format);
    }
}

void VersionNameCreator::addFileSuffix(QString& fileName, const QString& format, const QString& originalName) const
{
    if (fileName.isEmpty())
    {
        return;
    }

    bool isLower = true;

    if (!originalName.isEmpty())
    {
        // if original name had upper case suffix, continue using upper case

        isLower = originalName.at(originalName.size() - 1).isLower();
    }

    if (!fileName.endsWith(QLatin1Char('.')))
    {
        fileName += QLatin1Char('.');
    }

    fileName += (isLower ? format.toLower() : format);
}

// ------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN VersionManager::Private
{
public:

    Private() = default;

    VersionManagerSettings     settings;
    VersionNamingScheme*       scheme           = nullptr;

    DefaultVersionNamingScheme defaultScheme;
};

// ------------------------------------------------------------------------------------------------------

VersionManager::VersionManager()
    : d(new Private)
{
}

VersionManager::~VersionManager()
{
    delete d->scheme;
    delete d;
}

void VersionManager::setSettings(const VersionManagerSettings& settings)
{
    d->settings = settings;
}

VersionManagerSettings VersionManager::settings() const
{
    return d->settings;
}

void VersionManager::setNamingScheme(VersionNamingScheme* scheme)
{
    d->scheme = scheme;
}

VersionNamingScheme* VersionManager::namingScheme() const
{
    if (d->scheme)
    {
        return d->scheme;
    }
    else
    {
        return &d->defaultScheme;
    }
}

VersionFileOperation VersionManager::operation(FileNameType request, const VersionFileInfo& loadedFile,
                                               const DImageHistory& initialResolvedHistory,
                                               const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);

    if      (request == CurrentVersionName)
    {
        name.checkNeedNewVersion();
    }
    else if (request == NewVersionName)
    {
        name.fork();
    }

    name.setSaveDirectory();
    name.setSaveFormat();
    name.setSaveFileName();
    name.initOperation();
    name.checkIntermediates();

    return name.operation;
}

VersionFileOperation VersionManager::operationNewVersionInFormat(const VersionFileInfo& loadedFile,
                                                                 const QString& format,
                                                                 const DImageHistory& initialResolvedHistory,
                                                                 const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);

    name.fork();
    name.setSaveDirectory();
    name.setSaveFormat(format);
    name.setSaveFileName();
    name.initOperation();
    name.checkIntermediates();

    return name.operation;
}

VersionFileOperation VersionManager::operationNewVersionAs(const VersionFileInfo& loadedFile,
                                                           const VersionFileInfo& saveLocation,
                                                           const DImageHistory& initialResolvedHistory,
                                                           const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);

    name.fork();
    name.setSaveDirectory(saveLocation.path);
    name.setSaveFormat(saveLocation.format);
    name.setSaveFileName(saveLocation.fileName);
    name.initOperation();
    name.checkIntermediates();

    return name.operation;
}

QString VersionManager::toplevelDirectory(const QString& path)
{
    Q_UNUSED(path);

    return QLatin1String("/");
}

QStringList VersionManager::workspaceFileFormats() const
{
    QStringList formats;
    formats << QLatin1String("JPG") << QLatin1String("PNG") << QLatin1String("TIFF") << QLatin1String("PGF") << QLatin1String("JP2");
    QString f = d->settings.format.toUpper();

    if (!formats.contains(f))
    {
        formats << f;
    }
/*
    f = d->settings.formatForStoringSubversions.toUpper();

    if (!formats.contains(f))
    {
        formats << f;
    }
*/
    return formats;
}

} // namespace Digikam
