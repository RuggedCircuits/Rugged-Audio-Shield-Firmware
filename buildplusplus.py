#!/usr/bin/env python
"""Automatically increment the build number in a version file. We expect that the
file comprises arbitrary data that we echo line by line, except for two special
markers:
   * The ----- marker indicates that the next line contains the current application
     information in 4 fields:
          APPNAME MAJOR MINOR BUILD
     For example:
         PULSES 1 3 12

     This line (including the markers) is echoed out but the build number is incremented
     by 1. All other fields are copied verbatim.

   * The ///// marker indicates the end of verbatim copying. This script then ignores
     all subsequent lines in the input file and write two new lines of the form:
        #define APPID "PULSES 01030012"
        #define APPVERSION  "1.3.0012"

The input file is overwritten with the new build-incremented version.

  Rugged Audio Shield Firmware for ATxmega

  Copyright (c) 2012 Rugged Circuits LLC.  All rights reserved.
  http://ruggedcircuits.com

  This file is part of the Rugged Circuits Rugged Audio Shield firmware distribution.

  This is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 3 of the License, or (at your option) any later
  version.

  This software is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  A copy of the GNU General Public License can be viewed at
  <http://www.gnu.org/licenses>

"""

import os
import sys
import tempfile

if len(sys.argv) != 2:
  print "Usage: %s VersionFile.c" % sys.argv[0]
  sys.exit(1)

try:
  infid = file(sys.argv[1], "rt")
except:
  print "Cannot open input file"
  sys.exit(2)

fileno, path = tempfile.mkstemp(".tmp", "b_p_p", ".")
outfid = os.fdopen(fileno, "w")

appname = major = minor = build = None
nextIsAppInfo = False

for line in infid:
  if nextIsAppInfo:
    nextIsAppInfo=False

    try:
      appname, major, minor, build = line.split()
    except:
      print 'Unable to interpret marker line:\n  %s' % line
      sys.exit(3)

    try:
      major, minor, build = [int(x) for x in (major,minor,build)]
    except:
      print 'Major/minor/build not valid integers:\n  %s' % line
      sys.exit(4)

    appname = appname.upper()
    build += 1
    print >> outfid, "%s %d %d %d" % (appname, major, minor, build)
  else:
    print >> outfid, line,

    if len(line) >= 5:
      if line[:5]=='-----' and appname is None:
        nextIsAppInfo = True
      elif line[:5] == '/////' and appname is not None:
        break # Done with input, now create our output
else:
  if appname is None:
    print "Did not find application info marker '-----'"
  else:
    print "Did not find '/////' marker"
  sys.exit(5)

print >> outfid, '#define APPVERSION_MAJOR %u' % major
print >> outfid, '#define APPVERSION_MINOR %u' % minor
print >> outfid, '#define APPVERSION_BUILD %u' % build
print >> outfid, '#define APPID "%s %02d%02d%04d"' % (appname, major, minor, build)
print >> outfid, '#define APPVERSION "%d.%d.%04d"' % (major, minor, build)

outfid.close()
infid.close()

os.rename(path, sys.argv[1])

sys.exit(0)
