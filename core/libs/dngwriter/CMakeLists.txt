#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# NOTE: see the common definitions set on top level by RulesDNGSdk.cmake

# DNG SDK and XMP SDK use C++ exceptions

kde_enable_exceptions()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

    ${EXPAT_INCLUDE_DIR}
    ${JXL_INCLUDE_DIRS}

    ${CMAKE_CURRENT_SOURCE_DIR}/extra/md5
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/public/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source
)

# Don't process automoc on DNG extra headers

file(GLOB_RECURSE dng_extra_headers ${CMAKE_CURRENT_SOURCE_DIR}/extra/*.h*)

foreach(_file ${dng_extra_headers})
    set_property(SOURCE ${_file} PROPERTY SKIP_AUTOMOC ON)
endforeach()

#------------------------------------------------------------------------------------

set(libmd5_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/extra/md5/XMP_MD5.cpp)

# Used by digikamcore

add_library(core_libmd5_obj OBJECT ${libmd5_SRCS})

target_compile_definitions(core_libmd5_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# Disable warnings: we will never touch this code.
# Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler (bug: #269903)

if(MSVC)

    target_compile_options(core_libmd5_obj PRIVATE /W0)

else()

    target_compile_options(core_libmd5_obj PRIVATE -w)

endif()

#------------------------------------------------------------------------------------

# NOTE: minimum XMP SDK code to compile for DNG SDK. See extra/README for details.
set(libxmp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/PerfUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/SafeStringAPIs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/UnicodeConversions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/UnicodeInlines.incl_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/XML_Node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/XMP_LibUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/source/XMP_ProgressTracker.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/ExpatAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/ParseRDF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/WXMPDocOps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/WXMPIterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/WXMPMeta.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/WXMPUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPCore_Impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPIterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPIterator2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPMeta-GetSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPMeta-Parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPMeta-Serialize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPMeta.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPMeta2-GetSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPUtils-FileInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPUtils2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPDocOps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/xmp_sdk/XMPCore/source/XMPDocOps-Utils.cpp
)

# Used by digikamcore

add_library(core_libxmp_obj OBJECT ${libxmp_SRCS})

target_compile_definitions(core_libxmp_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# Disable warnings: we will never touch this code.
# Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler (bug: #269903)

if(MSVC)

    target_compile_options(core_libxmp_obj PRIVATE /W0)

else()

    target_compile_options(core_libxmp_obj PRIVATE -w)

endif()

#------------------------------------------------------------------------------------

set(libdng_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_1d_function.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_1d_table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_abort_sniffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_area_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_bad_pixels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_big_table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_bottlenecks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_camera_profile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_color_space.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_color_spec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_date_time.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_exif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_file_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_filter_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_fingerprint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_gain_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_globals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_host.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_hue_sat_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_ifd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_image_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_iptc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_jpeg_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_lens_correction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_linearization_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_local_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_lossless_jpeg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_memory_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_misc_opcodes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_mosaic_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_mutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_negative.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_opcode_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_opcodes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_orientation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_parse_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_pixel_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_point.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_preview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_pthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_rational.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_read_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_rect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_ref_counted_block.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_reference.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_resample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_safe_arithmetic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_shared.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_simple_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_spline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_string_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_tag_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_temperature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_tile_iterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_tone_curve.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_validate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_xmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_xmp_sdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_xy_coord.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_bmff.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_jpeg_memory_source.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_jxl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_lossless_jpeg_shared.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/dng_sdk/dng_update_meta.cpp
)

# Used by digikamcore

add_library(core_libdng_obj OBJECT ${libdng_SRCS})

target_compile_definitions(core_libdng_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

set_target_properties(core_libdng_obj
                      PROPERTIES
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO
)

# Disable warnings: we will never touch this code.
# Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler (bug: #269903)

if(MSVC)

    target_compile_options(core_libdng_obj PRIVATE /W0)

else()

    target_compile_options(core_libdng_obj PRIVATE -w)

endif()

# For unit tests.

add_library(libdng STATIC
            $<TARGET_OBJECTS:core_libdng_obj>
            $<TARGET_OBJECTS:core_libxmp_obj>
            $<TARGET_OBJECTS:core_libmd5_obj>
)

target_link_libraries(libdng

                      PRIVATE

                      ${EXPAT_LIBRARY}
                      ${JXL_LIBRARIES}
)

#------------------------------------------------------------------------------------

set(libdngwriter_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dngsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriterhost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_convert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_import.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_mosaic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_negative.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_exif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_makernote.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_xmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_backupraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_export.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dngwriter_postprocess.cpp
)

# Used by digikamcore

add_library(core_dngwriter_obj OBJECT ${libdngwriter_SRCS})

target_compile_definitions(core_dngwriter_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
