/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "editorcore_p.h"

// C++ includes

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// Qt includes

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QColor>
#include <QFile>
#include <QVariant>
#include <QImageReader>
#include <QPainter>
#include <QScopedPointer>

namespace Digikam
{

EditorCore* EditorCore::m_defaultInstance = nullptr;

EditorCore* EditorCore::defaultInstance()
{
    return m_defaultInstance;
}

void EditorCore::setDefaultInstance(EditorCore* const instance)
{
    m_defaultInstance = instance;
}

EditorCore::EditorCore()
    : QObject(),
      d      (new Private)
{
    d->undoMan = new UndoManager(this);
    d->thread  = new SharedLoadSaveThread;

    connect(d->thread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotImageLoaded(LoadingDescription,DImg)));

    connect(d->thread, SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    connect(d->thread, SIGNAL(signalLoadingProgress(LoadingDescription,float)),
            this, SLOT(slotLoadingProgress(LoadingDescription,float)));

    connect(d->thread, SIGNAL(signalSavingProgress(QString,float)),
            this, SLOT(slotSavingProgress(QString,float)));
}

EditorCore::~EditorCore()
{
    delete d->undoMan;
    delete d->thread;
    delete d;

    if (m_defaultInstance == this)
    {
        m_defaultInstance = nullptr;
    }
}

void EditorCore::setDisplayingWidget(QWidget* const widget)
{
    d->displayingWidget = widget;
}

void EditorCore::load(const QString& filePath, IOFileSettings* const iofileSettings)
{
    LoadingDescription description(filePath, LoadingDescription::ConvertForEditor);

    if (DImg::fileFormat(filePath) == DImg::RAW)
    {
        if (EditorToolIface::editorToolIface() && iofileSettings->useRAWImport)
        {
            if (d->rawPlugin && (d->rawPlugin->iid() != iofileSettings->rawImportToolIid))
            {
                disconnect(d->rawPlugin, SIGNAL(signalDecodedImage(Digikam::LoadingDescription,Digikam::DImg)),
                           this, SLOT(slotLoadRawFromTool(Digikam::LoadingDescription,Digikam::DImg)));

                disconnect(d->rawPlugin, SIGNAL(signalLoadRaw(Digikam::LoadingDescription)),
                           this, SLOT(slotLoadRaw(Digikam::LoadingDescription)));

                d->rawPlugin = nullptr;
            }

            if (!d->rawPlugin)
            {
                const auto plugs = DPluginLoader::instance()->allPlugins();

                for (DPlugin* const p : plugs)
                {
                    DPluginRawImport* const raw = dynamic_cast<DPluginRawImport*>(p);

                    if (raw && (raw->iid() == iofileSettings->rawImportToolIid))
                    {
                        d->rawPlugin = raw;

                        connect(d->rawPlugin, SIGNAL(signalDecodedImage(Digikam::LoadingDescription,Digikam::DImg)),
                                this, SLOT(slotLoadRawFromTool(Digikam::LoadingDescription,Digikam::DImg)));

                        connect(d->rawPlugin, SIGNAL(signalLoadRaw(Digikam::LoadingDescription)),
                                this, SLOT(slotLoadRaw(Digikam::LoadingDescription)));

                        break;
                    }
                }
            }

            if (d->rawPlugin)
            {
                if (d->rawPlugin->run(filePath, iofileSettings->rawDecodingSettings))
                {
                    d->thread->stopLoading();

                    return;
                }
            }

            qCCritical(DIGIKAM_GENERAL_LOG) << "Cannot found Raw Import tool! This probably due to a wrong "
                                               "install of plugins. Load Raw file with default settings...";
        }

        description = LoadingDescription(filePath, iofileSettings->rawDecodingSettings,
                                         LoadingDescription::RawDecodingGlobalSettings,
                                         LoadingDescription::ConvertForEditor);
    }

    d->load(description);
}

void EditorCore::slotLoadRawFromTool(const LoadingDescription& props, const DImg& img)
{
    d->resetValues();
    d->currentDescription = props;

    Q_EMIT signalLoadingStarted(d->currentDescription.filePath);

    slotImageLoaded(d->currentDescription, img);
    EditorToolIface::editorToolIface()->unLoadTool();
/*
    Q_EMIT signalImageLoaded(d->currentDescription.filePath, true);
*/
}

void EditorCore::slotLoadRaw(const LoadingDescription& props)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << d->nextRawDescription.rawDecodingSettings;
*/
    d->load(props);
}

void EditorCore::applyTransform(const IccTransform& transform)
{
    if (!d->valid)
    {
        return;
    }

    d->currentDescription.postProcessingParameters.colorManagement = LoadingDescription::ApplyTransform;
    d->currentDescription.postProcessingParameters.setTransform(transform);
    d->loadCurrent();

    if (EditorToolIface::editorToolIface())
    {
        EditorToolIface::editorToolIface()->unLoadTool();
    }
}

void EditorCore::restore()
{
    LoadingDescription description = d->currentDescription;
    d->resetValues();
    d->load(description);
}

void EditorCore::resetImage()
{
    if (EditorToolIface::editorToolIface())
    {
        EditorToolIface::editorToolIface()->unLoadTool();
    }

    d->resetValues();
    d->image.reset();
}

void EditorCore::setICCSettings(const ICCSettingsContainer& cmSettings)
{
    d->cmSettings = cmSettings;
}

ICCSettingsContainer EditorCore::getICCSettings() const
{
    return d->cmSettings;
}

void EditorCore::setExposureSettings(ExposureSettingsContainer* const expoSettings)
{
    d->expoSettings = expoSettings;
}

ExposureSettingsContainer* EditorCore::getExposureSettings() const
{
    return d->expoSettings;
}

void EditorCore::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    if (loadingDescription != d->currentDescription)
    {
        return;
    }

    // RAW tool active? Discard previous loaded image
    // Special case for the Gitlab-CI, no Editor window also no EditorToolIface
/*
    if (EditorToolIface::editorToolIface())
    {
        EditorTool* const tool = EditorToolIface::editorToolIface()->currentTool();

        if (tool && tool->property("DPluginIId").toString().contains(QLatin1String("rawimport")))
        {
            return;
        }
    }
*/
    bool valRet = false;
    d->image    = img;

    if (!d->image.isNull())
    {
        d->valid      = true;
        valRet        = true;
        d->resolvedInitialHistory = d->image.getOriginalImageHistory();
        d->resolvedInitialHistory.clearReferredImages(); // default empty, real values set by higher level

        // Raw files are already rotated properly by Raw engine. Only perform auto-rotation with non-RAW files.
        // We don't have a feedback from Raw engine about auto-rotated RAW file during decoding.
        // Setting rotatedOrFlipped to true will reset the exif flag on save (the data is then already rotated)

        if      (d->image.detectedFormat() == DImg::RAW)
        {
            d->rotatedOrFlipped = true;
        }
        else if (d->exifOrient)
        {
            // Do not rotate twice if already rotated, e.g. for full size preview.

            if (!d->image.wasExifRotated())
            {
                d->rotatedOrFlipped = d->image.rotateAndFlip(d->image.exifOrientation(loadingDescription.filePath));
            }
        }

        // set after rotation

        d->origWidth  = d->image.width();
        d->origHeight = d->image.height();
        d->width      = d->origWidth;
        d->height     = d->origHeight;
        d->loadedSize = d->image.size();
        d->image.setAttribute(QLatin1String("originalSize"), d->image.size());
    }
    else
    {
        valRet = false;
    }

    Q_EMIT signalImageLoaded(d->currentDescription.filePath, valRet);

    setModified();
}

void EditorCore::setSoftProofingEnabled(bool enabled)
{
    d->doSoftProofing = enabled;
}

bool EditorCore::softProofingEnabled() const
{
    return d->doSoftProofing;
}

void EditorCore::slotLoadingProgress(const LoadingDescription& loadingDescription, float progress)
{
    if (loadingDescription == d->currentDescription)
    {
        Q_EMIT signalLoadingProgress(loadingDescription.filePath, progress);
    }
}

bool EditorCore::exifRotated() const
{
    return d->rotatedOrFlipped;
}

void EditorCore::setExifOrient(bool exifOrient)
{
    d->exifOrient = exifOrient;
}

void EditorCore::undo()
{
    if (!d->undoMan->anyMoreUndo())
    {
        Q_EMIT signalUndoStateChanged();

        return;
    }

    d->undoMan->undo();

    Q_EMIT signalUndoStateChanged();
}

void EditorCore::redo()
{
    if (!d->undoMan->anyMoreRedo())
    {
        Q_EMIT signalUndoStateChanged();

        return;
    }

    d->undoMan->redo();

    Q_EMIT signalUndoStateChanged();
}

void EditorCore::rollbackToOrigin()
{
    d->undoMan->rollbackToOrigin();

    Q_EMIT signalUndoStateChanged();
}

void EditorCore::saveAs(const QString& filePath, IOFileSettings* const iofileSettings,
                        bool setExifOrientationTag, const QString& givenMimeType,
                        const QString& intendedFilePath)
{
    d->saveAs(filePath, iofileSettings, setExifOrientationTag, givenMimeType,
              VersionFileOperation(), intendedFilePath);
}

void EditorCore::saveAs(const QString& filePath, IOFileSettings* const iofileSettings,
                        bool setExifOrientationTag, const QString& givenMimeType,
                        const VersionFileOperation& op)
{
    d->saveAs(filePath, iofileSettings, setExifOrientationTag, givenMimeType, op, op.saveFile.filePath());
}

void EditorCore::slotImageSaved(const QString& filePath, bool success)
{
    if (d->filesToSave.isEmpty() || (d->filesToSave[d->currentFileToSave].filePath != filePath))
    {
        return;
    }

    Private::FileToSave& savedFile = d->filesToSave[d->currentFileToSave];

    if (success)
    {
        if (savedFile.historyStep == -1)
        {
            // Note: We operate on a temp file here, so we cannot
            // add it as referred image yet. Done in addLastSavedToHistory

            LoadingDescription description(filePath, LoadingDescription::ConvertForEditor);
            d->currentDescription = description;
        }
        else
        {
            HistoryImageId id = savedFile.image.addAsReferredImage(filePath);

            // for all images following in history, we need to insert the now saved file at the right place

            for (int i = d->currentFileToSave + 1 ; i < d->filesToSave.size() ; ++i)
            {
                d->filesToSave[i].image.insertAsReferredImage(savedFile.historyStep, id);
            }
        }
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "error saving image '" << QFile::encodeName(filePath).constData();
    }

    d->currentFileToSave++;

    if (d->currentFileToSave == d->filesToSave.size())
    {
        d->filesToSave.clear();

        Q_EMIT signalImageSaved(filePath, success);
    }
    else
    {
        d->saveNext();
    }
}

void EditorCore::slotSavingProgress(const QString& filePath, float progress)
{
    if (!d->filesToSave.isEmpty() && d->filesToSave.at(d->currentFileToSave).filePath == filePath)
    {
        Q_EMIT signalSavingProgress(filePath, progress);
    }
}

void EditorCore::abortSaving()
{
    // failure will be reported by a signal

    if (!d->filesToSave.isEmpty())
    {
        d->thread->stopSaving(d->filesToSave.at(d->currentFileToSave).filePath);
        d->filesToSave.clear();
    }
}

QString EditorCore::ensureHasCurrentUuid() const
{
    /*
     * 1) An image is loaded. The DImgLoader adds the HistoryImageId of the loaded file as "Current" entry.
     * 2) The loaded image has no UUID (created by camera etc.). Higher level calls ensureHasCurrentUuid
     *    before any saving is started
     * 3) We create a new UUID and add it to the image's history. When the new image is saved,
     *    it references the original by UUID. Because we, here, do not touch the original,
     *    it is out of scope to add the UUID to the original file's metadata.
     *    Higher level is responsible for this.
     * 4) When the image is saved, DImg::updateMetadata will create a new UUID for the saved
     *    image, which is then of course written to the newly saved file.
     */
    if (!d->image.getItemHistory().currentReferredImage().hasUuid())
    {
        // if there is no uuid in the image, we create one.

        QString uuid = QString::fromUtf8(d->image.createImageUniqueId());
        d->image.addCurrentUniqueImageId(uuid);
    }

    return d->image.getItemHistory().currentReferredImage().uuid();
}

void EditorCore::provideCurrentUuid(const QString& uuid)
{
    // If the (original) image did not yet have a UUID, one is provided by higher level
    // Higher level decides how this UUID is stored; we don't touch the original here.

    if (!d->image.getItemHistory().currentReferredImage().hasUuid())
    {
        d->image.addCurrentUniqueImageId(uuid);
    }
}

void EditorCore::setLastSaved(const QString& filePath)
{
    if (getImageFilePath() == filePath)
    {
        // if the file was overwritten, a complete undo, to the state of original loading,
        // does not return to a real image anymore - it's overwritten

        d->undoMan->clearPreviousOriginData();
    }

    // We cannot do it in slotImageSaved because we may operate on a temporary filePath.

    d->image.imageSavedAs(filePath);
}

void EditorCore::switchToLastSaved(const DImageHistory& resolvedCurrentHistory)
{
    // Higher level wants to use the current DImg object to represent the file
    // it has previously been saved to.
    // setLastSaved shall have been called before.

    d->image.switchOriginToLastSaved();

    if (resolvedCurrentHistory.isNull())
    {
        d->resolvedInitialHistory = d->image.getOriginalImageHistory();
        d->resolvedInitialHistory.clearReferredImages();
    }
    else
    {
        d->resolvedInitialHistory = resolvedCurrentHistory;
    }

    setUndoManagerOrigin();
}

void EditorCore::setHistoryIsBranch(bool isBranching)
{
    // The first added step (on top of the initial history) will be marked as branch

    d->image.setHistoryBranchAfter(d->resolvedInitialHistory, isBranching);
}

void EditorCore::setModified()
{
    Q_EMIT signalModified();
    Q_EMIT signalUndoStateChanged();
}

void EditorCore::readMetadataFromFile(const QString& file)
{
    QScopedPointer<DMetadata> meta(new DMetadata(file));

    // This can overwrite metadata changes introduced by tools.
    // Currently, this is ProfileConversion and lensfun.
    // ProfileConversion's changes is redone when saving by DImgLoader.
    // Lensfun is not critical.
    // For a clean solution, we'd need to record a sort of metadata changeset in UndoMetadataContainer.

    d->image.setMetadata(meta->data());

    // If we are editing, and someone else at the same time, there's nothing we can do.

    if (!d->undoMan->hasChanges())
    {
        d->image.setItemHistory(DImageHistory::fromXml(meta->getItemHistory()));
    }
}

void EditorCore::clearUndoManager()
{
    d->undoMan->clear();
    d->undoMan->setOrigin();

    Q_EMIT signalUndoStateChanged();
}

void EditorCore::setUndoManagerOrigin()
{
    d->undoMan->setOrigin();

    Q_EMIT signalUndoStateChanged();
    Q_EMIT signalFileOriginChanged(getImageFilePath());
}

bool EditorCore::isValid() const
{
    return d->valid;
}

int EditorCore::width() const
{
    return d->width;
}

int EditorCore::height() const
{
    return d->height;
}

int EditorCore::origWidth() const
{
    return d->origWidth;
}

int EditorCore::origHeight() const
{
    return d->origHeight;
}

int EditorCore::bytesDepth() const
{
    return d->image.bytesDepth();
}

bool EditorCore::sixteenBit() const
{
    return d->image.sixteenBit();
}

bool EditorCore::hasAlpha() const
{
    return d->image.hasAlpha();
}

bool EditorCore::isReadOnly() const
{
    if (d->image.isNull())
    {
        return true;
    }
    else
    {
        return d->image.isReadOnly();
    }
}

QSize EditorCore::loadedSize() const
{
    return d->loadedSize;
}

void EditorCore::setSelectedArea(const QRect& rect)
{
    d->selX = rect.x();
    d->selY = rect.y();
    d->selW = rect.width();
    d->selH = rect.height();
}

QRect EditorCore::getSelectedArea() const
{
    return (QRect(d->selX, d->selY, d->selW, d->selH));
}

void EditorCore::zoom(double val)
{
    d->zoom   = val;
    d->width  = (int)(d->origWidth  * val);
    d->height = (int)(d->origHeight * val);
}

void EditorCore::rotate90()
{
    d->applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Rotate90));
}

void EditorCore::rotate180()
{
    d->applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Rotate180));
}

void EditorCore::rotate270()
{
    d->applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Rotate270));
}

void EditorCore::flipHoriz()
{
    d->applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::FlipHorizontally));
}

void EditorCore::flipVert()
{
    d->applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::FlipVertically));
}

void EditorCore::crop(const QRect& rect)
{
    d->applyBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Crop, rect), new UndoActionIrreversible(this, QLatin1String("Crop")));
}

void EditorCore::convertDepth(int depth)
{
    d->applyBuiltinFilter(DImgBuiltinFilter(depth == 32 ? DImgBuiltinFilter::ConvertTo8Bit : DImgBuiltinFilter::ConvertTo16Bit),
                          new UndoActionIrreversible(this, QLatin1String("Convert Color Depth")));
}

DImg* EditorCore::getImg() const
{
    if (!d->image.isNull())
    {
        return &d->image;
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "d->image is null";
        return nullptr;
    }
}

DImageHistory EditorCore::getItemHistory() const
{
    return d->image.getItemHistory();
}

DImageHistory EditorCore::getInitialImageHistory() const
{
    return d->image.getOriginalImageHistory();
}

DImageHistory EditorCore::getImageHistoryOfFullRedo() const
{
    return d->undoMan->getImageHistoryOfFullRedo();
}

DImageHistory EditorCore::getResolvedInitialHistory() const
{
    return d->resolvedInitialHistory;
}

void EditorCore::setResolvedInitialHistory(const DImageHistory& history)
{
    d->resolvedInitialHistory = history;
}

void EditorCore::putImg(const QString& caller, const FilterAction& action, const DImg& img)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, caller));
    d->putImageData(img.bits(), img.width(), img.height(), img.sixteenBit());
    d->image.addFilterAction(action);
    setModified();
}

void EditorCore::setUndoImg(const UndoMetadataContainer& c, const DImg& img)
{
    // called from UndoManager

    d->putImageData(img.bits(), img.width(), img.height(), img.sixteenBit());
    c.toImage(d->image);
}

void EditorCore::imageUndoChanged(const UndoMetadataContainer& c)
{
    // called from UndoManager

    d->origWidth    = d->image.width();
    d->origHeight   = d->image.height();
    c.toImage(d->image);
}

void EditorCore::setFileOriginData(const QVariant& data)
{
    d->image.setFileOriginData(data);

    Q_EMIT signalFileOriginChanged(getImageFilePath());
}

DImg EditorCore::getImgSelection() const
{
    if (!d->selW || !d->selH)
    {
        return DImg();
    }

    if (!d->image.isNull())
    {
        DImg im = d->image.copy(d->selX, d->selY, d->selW, d->selH);
        im.detach();

        return im;
    }

    return DImg();
}

void EditorCore::putImgSelection(const QString& caller, const FilterAction& action, const DImg& img)
{
    if (img.isNull() || d->image.isNull())
    {
        return;
    }

    d->undoMan->addAction(new UndoActionIrreversible(this, caller));

    d->image.bitBltImage(img.bits(), 0, 0, d->selW, d->selH, d->selX, d->selY, d->selW, d->selH, d->image.bytesDepth());

    d->image.addFilterAction(action);
    setModified();
}

void EditorCore::putIccProfile(const IccProfile& profile)
{
    if (d->image.isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "d->image is null";
        return;
    }
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Embedding profile: " << profile;
*/
    d->image.setIccProfile(profile);
    setModified();
}

QStringList EditorCore::getUndoHistory() const
{
    return d->undoMan->getUndoHistory();
}

QStringList EditorCore::getRedoHistory() const
{
    return d->undoMan->getRedoHistory();
}

int EditorCore::availableUndoSteps() const
{
    return d->undoMan->availableUndoSteps();
}

int EditorCore::availableRedoSteps() const
{
    return d->undoMan->availableRedoSteps();
}

IccProfile EditorCore::getEmbeddedICC() const
{
    return d->image.getIccProfile();
}

MetaEngineData EditorCore::getMetadata() const
{
    return d->image.getMetadata();
}

QString EditorCore::getImageFilePath() const
{
    return d->image.originalFilePath();
}

QString EditorCore::getImageFileName() const
{
    return getImageFilePath().section(QLatin1Char('/'), -1);
}

QString EditorCore::getImageFormat() const
{
    if (d->image.isNull())
    {
        return QString();
    }

    QString mimeType = d->image.format();

    // It is a bug in the loader if format attribute is not given

    if (mimeType.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DImg object does not contain attribute \"format\"";
        mimeType = QString::fromUtf8(QImageReader::imageFormat(getImageFilePath()));
    }

    return mimeType;
}

QPixmap EditorCore::convertToPixmap(const DImg& img) const
{
    QPixmap pix;

    if (d->cmSettings.enableCM && (d->cmSettings.useManagedView || d->doSoftProofing))
    {
        // do not use d->monitorICCtrans here, because img may have a different embedded profile

        IccManager manager(img);
        IccTransform transform;

        if (d->doSoftProofing)
        {
            transform = manager.displaySoftProofingTransform(IccProfile(d->cmSettings.defaultProofProfile));
        }
        else
        {
            transform = manager.displayTransform();
        }

        pix = img.convertToPixmap(transform);
    }
    else
    {
        pix = img.convertToPixmap();
    }

    // Show the Over/Under exposure pixels indicators

    if (d->expoSettings->underExposureIndicator || d->expoSettings->overExposureIndicator)
    {
        QPainter painter(&pix);
        QImage pureColorMask = img.pureColorMask(d->expoSettings);
        QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
        painter.drawPixmap(pix.rect(), pixMask);
    }

    return pix;
}

UndoState EditorCore::undoState() const
{
    UndoState state;
    state.hasUndo            = d->undoMan->anyMoreUndo();
    state.hasRedo            = d->undoMan->anyMoreRedo();
    state.hasUndoableChanges = !d->undoMan->isAtOrigin();

    // Includes the edit step performed by RAW import, which is not undoable

    state.hasChanges         = d->undoMan->hasChanges();

    return state;
}

} // namespace Digikam

#include "moc_editorcore.cpp"
