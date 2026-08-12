#! /usr/bin/env ruby

# ============================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date        : 2012-07-14
# Description : a helper script for finding source code with no moc includes
#
# SPDX-FileCopyrightText: 2012 by Andi Clemens <andi dot clemens at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================ */

# get all header files containing the line "Q_OBJECT"
candidates = Dir.glob("**/*.h").select do |file_name|
  File.read(file_name) =~ /^\s*Q_OBJECT\s*$/ rescue false
end

# get all source files with missing MOC includes
missingMocIncludes = candidates.select do |file_name|
  source_file = file_name.sub(/\.h$/, ".cpp")
  moc_file = File.basename(file_name, '.h') + '.moc'
  pattern = /#include\s+[<"]#{moc_file}[>"]/

  (File.read(source_file) =~ pattern) == nil rescue false
end

# display missing MOC includes
puts "missing MOC include:"
width = missingMocIncludes.length.to_s.length

missingMocIncludes.each_with_index do |file_name, idx|
  puts "%#{width}s: #{file_name}" % (idx + 1)
end
