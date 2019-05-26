# Copyright 2017 - , Dr.-Ing. Patrick Siegl
# SPDX-License-Identifier: BSD-2-Clause

#!/usr/bin/python

import subprocess
import sys
import operator
import multiprocessing
import re

kernel = "seismic.x86_64.elf"

def bench( kernel, iterations, variants ):
  res = {}
  g_cmd = "./%s --timesteps=4000 --width=2000 --height=644 --pulseX=600 --pulseY=70 --threads=%d" % (kernel, multiprocessing.cpu_count())
  for t in range(iterations):
    for i in range(len(variants)):
      var = variants[ (i + t) % len(variants) ] # due to throttling and turbo boost
      res.setdefault(var, 0.0)

      cmd = g_cmd.split(' ')
      cmd.append( "--kernel=%s" % var )
      out = subprocess.check_output( cmd, shell=False )
      obj = re.match( r'(.*\n)*.*INNER.*GFLOPS: ([0-9]*\.[0-9]*).*', out.decode("utf-8"), re.MULTILINE )
      res[ var ] += float( obj.group(2) )

    print("done with iteration %d" % (t+1) )

  for var in variants:
    res[ var ] /= float(iterations)

  return res


def get_all_supported():
  out = subprocess.check_output( [ "./%s" % kernel, "--help" ], shell=False )
  lines = out.decode("utf-8").splitlines()
  variants = []
  it = enumerate(lines)
  for tpl in it:
    if "--kernel" in tpl[1]:
      break
  for tpl in it:
    if "--" not in tpl[1]:
      variants.append(tpl[1].strip())
    else:
      break
  return variants


res = {}
variants = get_all_supported()
if len(sys.argv) <= 1:
  res = bench( kernel, 5, variants )
else:
  print( sys.argv[1:] )
  for v in sys.argv[1:]:
    if v not in variants:
      print("%s is not supported on this machine!" % v )
      sys.exit(1)
  res = bench( kernel, 5, sys.argv[1:] )

print("")
print("------------------")
print("Results:")
print("------------------")
sorted_res = sorted(res.items(), key=operator.itemgetter(1))
for t in sorted_res:
  print("%2.2f Gflops   (%3.2fx): %s" % (t[1], t[1]/sorted_res[0][1], t[0]))
