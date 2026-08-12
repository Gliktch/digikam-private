# <img src="core/data/pics/digikam/logo-digikam.webp" width="48"/> DigiKam - Professional Photo Management with the Power of Open Source

| CI Job        | Status                                                                                                                                                                                                  |
|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Gitlab Builds | [![Gitlab Builds Status](https://invent.kde.org/graphics/digikam/badges/master/pipeline.svg)](https://invent.kde.org/graphics/digikam/-/pipelines)                                                      |
| Coverity Scan | [![Coverity Scan Status](https://scan.coverity.com/projects/285/badge.svg)](https://scan.coverity.com/projects/digikam)                                                                                 |

If you are reading this on Github, be aware that this is an out-dated mirror. Our real code repository [is located here](https://invent.kde.org/graphics/digikam)

Developers, if you want to contribute, see the project [Contribute Page](https://www.digikam.org/contribute/)

NOTE: this C++ project compile with **CMake** and support **Qt5** and **Qt6** frameworks.

# About

digiKam is an advanced open-source digital photo management application that runs on Linux, Windows, and MacOS.
The application provides a comprehensive set of tools for importing, managing, editing, and sharing photos and RAW files.

![Albums View and Image Editor](/project/img/README/35354951072.webp)

You can use digiKam's import capabilities to easily transfer photos, RAW files, and videos directly from your camera
and external storage devices (SD cards, USB disks, etc.). The application allows you to configure import settings
and rules that process and organize imported items on-the-fly.

![Map View displaying rated items and Batch Queue Manager in action](/project/img/README/32558229094.webp)

digiKam organizes photos, RAW files, and videos into albums. But the application also features powerful tagging
tools that allow you to assign tags, ratings, and labels to photos and raw files. You can then use filtering
functionality to quickly find items that match specific criteria.

![Search items by date rage and Geolocation editor](/project/img/README/32557269024.webp)

In addition to filtering functionality, digiKam features powerful searching capabilities that let you search
the photo library by a wide range of criteria. You can search photos by tags, labels, rating, data, location,
and even specific EXIF, IPTC, or XMP metadata.

![Search by Tags with preview mode and Metadata Editor in action](/project/img/README/32217007615.webp)

You can also combine several criteria for more advanced searches. digiKam rely on Exiv2 library to handle metadata
tag contents from files to populate the photo library.

![Advanced search tool and video file result played as preview](/project/img/README/40743725771.webp)

digiKam can handle RAW files, and the application uses the excellent LibRaw library for decoding raw files.
The library is actively maintained and regularly updated to include support for the latest camera models.

![Find by items similarity and Light Table in action to compare side by side](/project/img/README/31407487553.webp)

The application provides a comprehensive set of editing tools. This includes basic tools for adjusting colors,
cropping, and sharpening as well as advanced tools for, curves adjustment, panorama stitching, and much more.
A special tool based on Lensfun library permit to apply lens corrections automatically on images.

![Panorama tool stiching photo](/project/img/README/40474079731.webp)

Extended functionality in digiKam is implemented via a set of tools, dedicated especially to import and export
contents to remote web-services.

digiKam is based in part on the work of the Independent JPEG Group.

# Authors

See [AUTHORS](AUTHORS) file for details.

# Related URLs

* [digiKam project web site](https://www.digikam.org)
* [digiKam handbook git repository](https://invent.kde.org/documentation/digikam-doc)
* [digiKam web site git repository](https://invent.kde.org/websites/digikam-org)
* [digiKam unit-test data git repository](https://invent.kde.org/graphics/digikam-test-data)

# Contact

If you have questions, comments, and suggestions see the project [Support Page](https://www.digikam.org/support/).

# Bug reports

IMPORTANT: the bug reports and wishlist entries are hosted by the Bugzilla
system which can be reached from the standard Help menu of digiKam.
A mail will automatically be sent to the digiKam development mailing list.
There is no need to contact directly the digiKam mailing list for a bug report
or a devel wish.

The instructions to report bugs or wishes to the digiKam project can be seen at [this url](https://www.digikam.org/contribute/#reporting-bugs-and-submitting-feature-requests):

See also the extra issue entries for the shared libraries used by digiKam:

* Raw file formats support: [LibRaw library](https://github.com/LibRaw/LibRaw/issues)
* Lens database support: [Lensfun library](https://github.com/lensfun/lensfun/issues)
* Camera drivers: [GPhoto2 library](http://gphoto.org/bugs)
* Files metadata backend: [Exiv2 library](https://github.com/Exiv2/exiv2/issues)
* Heif file format support: [LibHeif library](https://github.com/strukturag/libheif/issues)

# Compilation and Installation

See the instructions [available at this url](https://www.digikam.org/download/git).

# Donate Money

If you love digiKam, you can help developers to buy new photo devices to test
and implement new features. Thanks in advance for your generous donations.

For more information, look [at this url](https://www.digikam.org/donate/)
