<!--
SPDX-License-Identifier: BSD-2-Clause

SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>
-->

# Seismic Reverse Time Migration (RTM)

[![Build Status](https://circleci.com/gh/psiegl/seismic-rtm.svg?style=svg)](https://circleci.com/gh/psiegl/seismic-rtm)
[![Build Status](https://dev.azure.com/psiegl/seismic-rtm/_apis/build/status/psiegl.seismic-rtm?branchName=master)](https://dev.azure.com/psiegl/seismic-rtm/_build/latest?definitionId=1&branchName=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/2db350f5b41b44d19403ab0a51350ef6)](https://www.codacy.com/app/psiegl/seismic-rtm?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=psiegl/seismic-rtm&amp;utm_campaign=Badge_Grade)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/18474.svg)](https://scan.coverity.com/projects/psiegl-seismic-rtm)
[![CodeFactor](https://www.codefactor.io/repository/github/psiegl/seismic-rtm/badge/master)](https://www.codefactor.io/repository/github/psiegl/seismic-rtm/overview/master)
[![GitHub license](https://img.shields.io/github/license/psiegl/seismic-rtm.svg)](https://raw.githubusercontent.com/psiegl/seismic-rtm/master/LICENSES/BSD-2-Clause.txt)

Code models a 2-dimensional, forward-propagated Reverse Time Migration (RTM) by utilising an acoustic, homogeneous wave equation.
The 2-dimensional nine point stencil builds upon a fourth order [FDM with second derivative](https://en.wikipedia.org/wiki/Five-point_stencil#Higher_derivatives), whereas the synthethic initial wavelet leverages the seismic Ricker wavelet.

| variants    | authors |
| ----------- | ------- |
| origin      | CWP/SU:[Seismic Un*x](https://pubs.usgs.gov/of/2001/of01-326/HTML/SEISUNIX.HTM) / [SeisUnix](https://github.com/JohnWStockwellJr/SeisUnix) and [MADAGASCAR project](http://www.ahay.org/) |
| plain, FPGA | [Victor Medeiros, "fastRTM: An IDE to Support Development of the RTM Algorithm in High Performance FPGA platforms", 2013](https://repositorio.ufpe.br/handle/123456789/12299) |
| SSE         | [Alexandros Gremm, "Acceleration, Clustering and Performance Evaluation of Seismic Applications", June 2011](https://github.com/agremm/Seismic) and [Thomas Grosser, Alexandros Gremm, Sebastian Veith, Gerald Heim, Wolfgang Rosenstiel, Victor Medeiros, Manoel Eusebio de Lima, "Exploiting Heterogeneous Computing Platforms By Cataloging Best Solutions For Resource Intensive Seismic Applications", Mai 2011](https://www.thinkmind.org/index.php?view=article&articleid=intensive_2011_2_20_30034) |
| current     | Patrick Siegl, "Hybride Beschleunigung einer seismischen Applikation durch Kombination von traditionellen Methoden und OpenCL" ("Hybrid acceleration of a seismic application by combining traditional methods with opencl"), April 2012 |
