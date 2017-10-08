#!/usr/bin/python

#  This file is part of seismic-rtm.
#
# Copyright (c) 2017, Dipl.-Inf. Patrick Siegl
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import subprocess
import sys
import operator
import multiprocessing
import pprint

kernel = "seismic.x86_64.elf"

def bench( kernel, iterations, variants ):
  res = {}
  g_cmd = "./%s --timesteps=4000 --width=2000 --height=644 --pulseX=600 --pulseY=70 --threads=%d" % (kernel, multiprocessing.cpu_count())
  for t in range(iterations):
    for i in range(len(variants)):
      var = variants[ (i + t) % len(variants) ] # due to throttling and turbo boost
      cmd = "%s --kernel=%s" % (g_cmd, var)
      out2, err2 = subprocess.Popen( cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE ).communicate()
      res.setdefault(var, 0.0)
      res[ var ] += float( ("%s" % out2.decode("utf-8").splitlines()[20]).split(" ")[-1].replace(")","") )

    print("done with iteration %d" % (t+1) )

  for var in variants:
    res[ var ] /= float(iterations)

  return res


def get_all_supported():
  out, err = subprocess.Popen( ("./%s --help" % kernel).split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE ).communicate()
  variants = []
  lines = out.splitlines()
  for i in range(len(lines)):
    if "--kernel" in str(lines[i]):
      break
  while 1:
    i += 1
    var = str(lines[i].decode("utf-8") ).strip()
    if "--" not in var:
      variants.append(var)
    else:
      break
  return variants



res = {}
if len(sys.argv) <= 1:
  res = bench( kernel, 5, get_all_supported() )
else:
  print( sys.argv[1:] )
  variants = get_all_supported()
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
