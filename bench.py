#!/usr/bin/python

import os
import subprocess
import sys
import operator
import multiprocessing

kernel = "seismic.elf"

out, err = subprocess.Popen( ("./%s --help" % kernel).split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE ).communicate()
kern_opt = 0
res = {}
for var in out.decode("utf-8").splitlines():
  var = var.strip()
  if kern_opt:
    if var == "":
      break
  else:
    if "--kernel" in var:
      kern_opt = 1
    continue

  nums = 10
  sum_ms = 0.0
  for t in range(0,nums):
    cmd = "./%s --timesteps=1000 --width=2000 --height=516 --pulseX=600 --pulseY=70 --threads=%d --kernel=%s" % (kernel, multiprocessing.cpu_count(), var)
    out2, err2 = subprocess.Popen( cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE ).communicate()
    sum_ms += float( ("%s" % out2.decode("utf-8").splitlines()[20]).split(" ")[-1].replace(")","") )
  res[ var ] = sum_ms/float(nums)
  print("%s: done" % var )

print("")
print("results:")
sorted_res = sorted(res.items(), key=operator.itemgetter(1))
for t in sorted_res:
  print("%.2f: %s" % (t[1], t[0]))
