# Seismic Reverse Time Migration (RTM)

[![Build Status](https://circleci.com/gh/psiegl/seismic-rtm.svg?style=svg)](https://circleci.com/gh/psiegl/seismic-rtm)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/2db350f5b41b44d19403ab0a51350ef6)](https://www.codacy.com/app/psiegl/seismic-rtm?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=psiegl/seismic-rtm&amp;utm_campaign=Badge_Grade)
[![GitHub license](https://img.shields.io/github/license/psiegl/seismic-rtm.svg)](https://raw.githubusercontent.com/psiegl/seismic-rtm/master/LICENSE.MIT)

Code models a 2-dimensional, forward-propagated Reverse Time Migration (RTM) by utilising an acoustic, homogeneous wave equation.
The 2-dimensional nine point stencil builds upon a fourth order [FDM with second derivative](https://en.wikipedia.org/wiki/Five-point_stencil#Higher_derivatives), whereas the synthethic initial wavelet leverages the seismic Ricker wavelet.

## ORIGIN

  CWP/SU:[Seismic Un*x](https://pubs.usgs.gov/of/2001/of01-326/HTML/SEISUNIX.HTM) / [SeisUnix](https://github.com/JohnWStockwellJr/SeisUnix)

  [MADAGASCAR project](http://www.ahay.org/)

## PLAIN, FPGA

  Victor Medeiros, "fastRTM: An IDE to Support Development of the RTM Algorithm in High Performance FPGA platforms", 2013

## SSE

  Alexandros Gremm, "Acceleration, Clustering and Performance Evaluation of Seismic Applications", June 2011, [url](https://github.com/agremm/Seismic)

  Thomas Grosser, Alexandros Gremm, Sebastian Veith, Gerald Heim, Wolfgang Rosenstiel, Victor Medeiros, Manoel Eusebio de Lima, "Exploiting Heterogeneous Computing Platforms By Cataloging Best Solutions For Resource Intensive Seismic Applications", Mai 2011

## CURRENT

  Patrick Siegl, "Hybride Beschleunigung einer seismischen Applikation durch Kombination von traditionellen Methoden und OpenCL" ("Hybrid acceleration of a seismic application by combining traditional methods with opencl"), April 2012
