#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

#---------------------------------

# Common DNG SDK and XMP SDK definitions shared for DNGWriter and RawEngine.
#
# NOTE: see bug #195735: for platteforms flags, do not enable Mac flags provided by Adobe.
#       Sounds like all compile fine like under Linux.

# --- For XMP SDK ---------------------------------------------------------------------

add_definitions(-DEnablePluginManager=0)    # Do not use plugin extensions
add_definitions(-DXMP_StaticBuild=1)        # Build statically

add_definitions(-DBUILDING_XMPCOMPAREANDMERGE_LIB=1 -DXMP_COMPONENT_INT_NAMESPACE=AdobeXMPCompareAndMerge_Int -DLINKING_XMPCORE_LIB=1)
add_definitions(-DBUILDING_XMPCOMPAREANDMERGE_AS_STATIC=1)

# Prevent a denial-service-attack related to XML entity expansion
# ("billion laughs attack").
# See https://bugzilla.redhat.com/show_bug.cgi?id=888769

add_definitions(-DBanAllEntityUsage=1)

# Platteform definitions

if(MSVC)

    add_definitions(-DWIN_ENV=1)

else()

    add_definitions(-DUNIX_ENV=1)

endif()

# --- For DNG SDK ----------------------------------------------------------------------

# For details see dng_flags.h

add_definitions(-DqDNGUseXMP=1)             # Enable XMP support
add_definitions(-DqDNGXMPFiles=0)           # Minimum XMP-set
add_definitions(-DqDNGXMPDocOps=0)          # Minimum XMP-set
add_definitions(-DqDNGDebug=0)              # No debug code
add_definitions(-DqDNGUseLibJPEG=1)         # Use libjpeg
add_definitions(-DqDNGUseStdInt=1)          # Use standard definition of integer
add_definitions(-DqDNGThreadSafe=1)         # Use thread safety
add_definitions(-DqDNGValidateTarget=1)     # Compilation of DNG validation CLI tool

# Platteform definitions

if(MSVC)

    add_definitions(-DqWinOS=1
                    -DqMacOS=0
                    -DqLinux=0)

else()

    add_definitions(-DqWinOS=0
                    -DqMacOS=0
                    -DqLinux=1)

endif()

# Check processor endianness

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if(NOT IS_BIG_ENDIAN)

    add_definitions(-DqDNGLittleEndian=1)

endif()
