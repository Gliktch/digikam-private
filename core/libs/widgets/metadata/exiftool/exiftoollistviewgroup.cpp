/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata list view group.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoollistviewgroup.h"

// Qt includes

#include <QFont>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

ExifToolListViewGroup::ExifToolListViewGroup(QTreeWidget* const parent,
                                             const QString& group)
    : QObject        (parent),
      QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsEnabled);
    setExpanded(true);
    setDisabled(false);
    QFont fn0(font(0));
    fn0.setBold(true);
    fn0.setItalic(false);
    setFont(0, fn0);
    setText(0, group);

    if      (group == QLatin1String("AFCP"))
    {
        setToolTip(0, i18n("AXS File Concatenation Protocol"));
    }
    else if (group == QLatin1String("AIFF"))
    {
        setToolTip(0, i18n("Audio Interchange File Format"));
    }
    else if (group == QLatin1String("APE"))
    {
        setToolTip(0, i18n("Monkey's Audio"));
    }
    else if (group == QLatin1String("APP0"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 0"));
    }
    else if (group == QLatin1String("APP1"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 1"));
    }
    else if (group == QLatin1String("APP11"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 11"));
    }
    else if (group == QLatin1String("APP12"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 12"));
    }
    else if (group == QLatin1String("APP13"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 13"));
    }
    else if (group == QLatin1String("APP14"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 14"));
    }
    else if (group == QLatin1String("APP15"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 15"));
    }
    else if (group == QLatin1String("APP4"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 4"));
    }
    else if (group == QLatin1String("APP5"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 5"));
    }
    else if (group == QLatin1String("APP6"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 6"));
    }
    else if (group == QLatin1String("APP8"))
    {
        setToolTip(0, i18n("JPEG Meta Information Level 8"));
    }
    else if (group == QLatin1String("ASF"))
    {
        setToolTip(0, i18n("Microsoft Advanced Systems Format"));
    }
    else if (group == QLatin1String("Audible"))
    {
        setToolTip(0, i18n("Audible Audiobook"));
    }
    else if (group == QLatin1String("Canon"))
    {
        setToolTip(0, i18n("Canon Camera Information"));
    }
    else if (group == QLatin1String("CanonVRD"))
    {
        setToolTip(0, i18n("Canon Recipe Data Information"));
    }
    else if (group == QLatin1String("Composite"))
    {
        setToolTip(0, i18n("Derived Convenience Tags"));
    }
    else if (group == QLatin1String("DICOM"))
    {
        setToolTip(0, i18n("Digital Imaging and Communications in Medicine"));
    }
    else if (group == QLatin1String("DNG"))
    {
        setToolTip(0, i18n("Digital Negative"));
    }
    else if (group == QLatin1String("DV"))
    {
        setToolTip(0, i18n("Digital Video"));
    }
    else if (group == QLatin1String("DjVu"))
    {
        setToolTip(0, i18n("DjVu image"));
    }
    else if (group == QLatin1String("Ducky"))
    {
        setToolTip(0, i18n("Photoshop Save for Web"));
    }
    else if (group == QLatin1String("EXE"))
    {
        setToolTip(0, i18n("DOS/Windows executable"));
    }
    else if (group == QLatin1String("EXIF"))
    {
        setToolTip(0, i18n("Exchangeable Image File Format metadata"));
    }
    else if (group == QLatin1String("FITS"))
    {
        setToolTip(0, i18n("Flexible Image Transport System"));
    }
    else if (group == QLatin1String("FLAC"))
    {
        setToolTip(0, i18n("Free Lossless Audio Codec"));
    }
    else if (group == QLatin1String("FLIR"))
    {
        setToolTip(0, i18n("FLIR Thermal Imaging Camera Information"));
    }
    else if (group == QLatin1String("File"))
    {
        setToolTip(0, i18n("File System Information"));
    }
    else if (group == QLatin1String("Flash"))
    {
        setToolTip(0, i18n("Macromedia/Adobe Flash Information"));
    }
    else if (group == QLatin1String("FlashPix"))
    {
        setToolTip(0, i18n("FlashPix Information"));
    }
    else if (group == QLatin1String("Font"))
    {
        setToolTip(0, i18n("Font Name Tags"));
    }
    else if (group == QLatin1String("FotoStation"))
    {
        setToolTip(0, i18n("FotoWare FotoStation Information"));
    }
    else if (group == QLatin1String("GIF"))
    {
        setToolTip(0, i18n("Compuserve Graphics Interchange Format"));
    }
    else if (group == QLatin1String("GIMP"))
    {
        setToolTip(0, i18n("GNU Image Manipulation Program"));
    }
    else if (group == QLatin1String("GeoTiff"))
    {
        setToolTip(0, i18n("Geolocated Tagged Image File Format"));
    }
    else if (group == QLatin1String("GoPro"))
    {
        setToolTip(0, i18n("GoPro Tags"));
    }
    else if (group == QLatin1String("H264"))
    {
        setToolTip(0, i18n("H.264 Video Streams Information"));
    }
    else if (group == QLatin1String("HTML"))
    {
        setToolTip(0, i18n("HyperText Markup Language"));
    }
    else if (group == QLatin1String("ICC Profile"))
    {
        setToolTip(0, i18n("International Color Consortium Color Profile"));
    }
    else if (group == QLatin1String("ID3"))
    {
        setToolTip(0, i18n("ID3 And Lyrics3 information"));
    }
    else if (group == QLatin1String("IPTC"))
    {
        setToolTip(0, i18n("International Press Telecommunications Council"));
    }
    else if (group == QLatin1String("ISO"))
    {
        setToolTip(0, i18n("ISO 9660 Disk Image"));
    }
    else if (group == QLatin1String("ITC"))
    {
        setToolTip(0, i18n("iTunes Cover Flow Artwork"));
    }
    else if (group == QLatin1String("JFIF"))
    {
        setToolTip(0, i18n("JPEG File Interchange Format"));
    }
    else if (group == QLatin1String("JPEG"))
    {
        setToolTip(0, i18n("Joint Photographic Experts Group"));
    }
    else if (group == QLatin1String("JSON"))
    {
        setToolTip(0, i18n("JavaScript Object Notation"));
    }
    else if (group == QLatin1String("Jpeg2000"))
    {
        setToolTip(0, i18n("Joint Photographic Experts Group 2000"));
    }
    else if (group == QLatin1String("LNK"))
    {
        setToolTip(0, i18n("Microsoft Shell Link"));
    }
    else if (group == QLatin1String("Leaf"))
    {
        setToolTip(0, i18n("Creo Leaf Capture Information"));
    }
    else if (group == QLatin1String("Lytro"))
    {
        setToolTip(0, i18n("Lytro Light Field Picture Info"));
    }
    else if (group == QLatin1String("M2TS"))
    {
        setToolTip(0, i18n("MPEG-2 Transport Stream"));
    }
    else if (group == QLatin1String("MIE"))
    {
        setToolTip(0, i18n("Meta Information Encapsulation"));
    }
    else if (group == QLatin1String("MIFF"))
    {
        setToolTip(0, i18n("Magick Image File Format"));
    }
    else if (group == QLatin1String("MNG"))
    {
        setToolTip(0, i18n("Multiple-image Network Graphics"));
    }
    else if (group == QLatin1String("MOI"))
    {
        setToolTip(0, i18n("JVC Canon and Panasonic Camcorders Information"));
    }
    else if (group == QLatin1String("MPC"))
    {
        setToolTip(0, i18n("Musepack Audio Information"));
    }
    else if (group == QLatin1String("MPEG"))
    {
        setToolTip(0, i18n("Motion Picture Experts Group"));
    }
    else if (group == QLatin1String("MPF"))
    {
        setToolTip(0, i18n("CIPA Multi-Picture Format Specification"));
    }
    else if (group == QLatin1String("MXF"))
    {
        setToolTip(0, i18n("Material Exchange Format"));
    }
    else if (group == QLatin1String("MakerNotes"))
    {
        setToolTip(0, i18n("Maker Notes Information"));
    }
    else if (group == QLatin1String("Matroska"))
    {
        setToolTip(0, i18n("Matroska Audio/Video/Subtitle Information"));
    }
    else if (group == QLatin1String("Meta"))
    {
        setToolTip(0, i18n("Meta Information"));
    }
    else if (group == QLatin1String("Ogg"))
    {
        setToolTip(0, i18n("Ogg Bitstream Container"));
    }
    else if (group == QLatin1String("OpenEXR"))
    {
        setToolTip(0, i18n("Open Extended Range"));
    }
    else if (group == QLatin1String("Opus"))
    {
        setToolTip(0, i18n("Ogg Opus Audio"));
    }
    else if (group == QLatin1String("PDF"))
    {
        setToolTip(0, i18n("Adobe Portable Document Format"));
    }
    else if (group == QLatin1String("PICT"))
    {
        setToolTip(0, i18n("Apple Picture Information"));
    }
    else if (group == QLatin1String("PLIST"))
    {
        setToolTip(0, i18n("Apple Property List"));
    }
    else if (group == QLatin1String("PNG"))
    {
        setToolTip(0, i18n("Portable Network Graphics"));
    }
    else if (group == QLatin1String("PSP"))
    {
        setToolTip(0, i18n("Paint Shop Pro Information"));
    }
    else if (group == QLatin1String("Palm"))
    {
        setToolTip(0, i18n("Palm database, Mobipocket electronic books, and Amazon Kindle Information"));
    }
    else if (group == QLatin1String("PanasonicRaw"))
    {
        setToolTip(0, i18n("Panasonic RAW 2 Information"));
    }
    else if (group == QLatin1String("Parrot"))
    {
        setToolTip(0, i18n("Streaming Parrot Drone Video Information"));
    }
    else if (group == QLatin1String("PhotoCD"))
    {
        setToolTip(0, i18n("Kodak Photo CD Image Pac"));
    }
    else if (group == QLatin1String("PhotoMechanic"))
    {
        setToolTip(0, i18n("Photo Mechanic Trailer Information"));
    }
    else if (group == QLatin1String("Photoshop"))
    {
        setToolTip(0, i18n("Adobe Photoshop Information"));
    }
    else if (group == QLatin1String("PostScript"))
    {
        setToolTip(0, i18n("PostScript Information"));
    }
    else if (group == QLatin1String("PrintIM"))
    {
        setToolTip(0, i18n("PrintIM Information"));
    }
    else if (group == QLatin1String("QuickTime"))
    {
        setToolTip(0, i18n("Apple QuickTime Movie Information"));
    }
    else if (group == QLatin1String("RAF"))
    {
        setToolTip(0, i18n("FujiFilm RAW Format Information"));
    }
    else if (group == QLatin1String("RIFF"))
    {
        setToolTip(0, i18n("Resource Interchange File Format Information"));
    }
    else if (group == QLatin1String("RSRC"))
    {
        setToolTip(0, i18n("Mac OS Resource"));
    }
    else if (group == QLatin1String("RTF"))
    {
        setToolTip(0, i18n("Rich Text Format"));
    }
    else if (group == QLatin1String("Radiance"))
    {
        setToolTip(0, i18n("RGBE High Dynamic-Range"));
    }
    else if (group == QLatin1String("Rawzor"))
    {
        setToolTip(0, i18n("Rawzor Image Information"));
    }
    else if (group == QLatin1String("Real"))
    {
        setToolTip(0, i18n("Real Media/Video Information"));
    }
    else if (group == QLatin1String("Red"))
    {
        setToolTip(0, i18n("Redcode RAW Video Information"));
    }
    else if (group == QLatin1String("SVG"))
    {
        setToolTip(0, i18n("Scalable Vector Graphics"));
    }
    else if (group == QLatin1String("SigmaRaw"))
    {
        setToolTip(0, i18n("Sigma/Foveon RAW Information"));
    }
    else if (group == QLatin1String("Sony"))
    {
        setToolTip(0, i18n("Sony Cameras Information"));
    }
    else if (group == QLatin1String("Stim"))
    {
        setToolTip(0, i18n("Stereo Still Image Information"));
    }
    else if (group == QLatin1String("Theora"))
    {
        setToolTip(0, i18n("Ogg Theora Video Specification"));
    }
    else if (group == QLatin1String("Torrent"))
    {
        setToolTip(0, i18n("BitTorrent Description Information"));
    }
    else if (group == QLatin1String("Trailer"))
    {
        setToolTip(0, i18n("Trailer Information"));
    }
    else if (group == QLatin1String("VCard"))
    {
        setToolTip(0, i18n("Virtual Card Information"));
    }
    else if (group == QLatin1String("Vorbis"))
    {
        setToolTip(0, i18n("Ogg Vorbis Specification"));
    }
    else if (group == QLatin1String("WTV"))
    {
        setToolTip(0, i18n("Windows Recorded TV Show Information"));
    }
    else if (group == QLatin1String("XML"))
    {
        setToolTip(0, i18n("Extensible Markup Language"));
    }
    else if (group == QLatin1String("XMP"))
    {
        setToolTip(0, i18n("Extensible Metadata Platform"));
    }
    else if (group == QLatin1String("ZIP"))
    {
        setToolTip(0, i18n("ZIP Archive Information"));
    }
}

} // namespace Digikam
