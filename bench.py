#!/usr/bin/python

#  This file is part of seismic-rtm.
#
#  seismic-rtm is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  seismic-rtm is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with seismic.  If not, see <http://www.gnu.org/licenses/>.

import os
import subprocess
import sys
import operator
import multiprocessing

kernel = "seismic.elf"

def bench( kernel, iterations, variants ):
  res = {}
  for var in variants:
    res[ var ] = 0.0

  for t in range(0,iterations):
    for i in range(0,len(variants)):
      var = variants[ (i + t) % len(variants) ] # due to throttling and turbo boost
      s_var = "--kernel=%s" % var
      if var == "openmp":
        s_var = "-m"
      cmd = "./%s --timesteps=4000 --width=2000 --height=516 --pulseX=600 --pulseY=70 --threads=%d %s" % (kernel, multiprocessing.cpu_count(), s_var)
      print( cmd )
      out2, err2 = subprocess.Popen( cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE ).communicate()
      line = ""
      for s in out2.decode("utf-8").splitlines():
        if "OUTER" in s:
          line = s
          break
      print( line )
      res[ var ] += float( line.split(" ")[-1].replace(")","") )

    print("done with iteration %d" % (t+1) )

  for var in variants:
    res[ var ] /= float(iterations)

  return res


def get_all_supported():
  out, err = subprocess.Popen( ("./%s --help" % kernel).split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE ).communicate()
  kern_opt = 0
  variants = []
  for var in out.decode("utf-8").splitlines():
    var = var.strip()
    if kern_opt:
      if var == "":
        break
    else:
      if "--kernel" in var:
        kern_opt = 1
      continue
    variants.append( var )
  return variants



res = {}
if len(sys.argv) <= 1:
  res = bench( kernel, 5, get_all_supported() )
else:
  variants = get_all_supported()
  for v in sys.argv[1:]:
    if v not in variants:
      print("%s is not supported on this machine!" % v )
      sys.exit(1)
  res = bench( kernel, 5, sys.argv[1:] )

print("")
print("------------------")
print("Results in GFLOPS:")
print("------------------")
sorted_res = sorted(res.items(), key=operator.itemgetter(1))
for t in sorted_res:
  print("%.2f: %s" % (t[1], t[0]))
