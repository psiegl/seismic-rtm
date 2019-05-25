
# Seismic Reverse Time Migration (RTM)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/660e70a7763a48ef9c322289a6d3cb70)](https://app.codacy.com/app/psiegl/seismic-rtm?utm_source=github.com&utm_medium=referral&utm_content=psiegl/seismic-rtm&utm_campaign=Badge_Grade_Dashboard)
[![Build Status](https://circleci.com/gh/psiegl/seismic-rtm.svg?style=svg)](https://circleci.com/gh/psiegl/seismic-rtm)
[![GitHub license](https://img.shields.io/github/license/psiegl/seismic-rtm.svg)](https://raw.githubusercontent.com/psiegl/seismic-rtm/master/LICENSE.MIT)

Code models a 2-dimensional, forward-propagated Reverse Time Migration (RTM) by utilising an acoustic, homogeneous wave equation.
The 2-dimensional nine point stencil builds upon a fourth order FDM with second derivative, whereas the synthethic initial wavelet leverages the seismic Ricker wavelet.
FDM fourth derivative: https://en.wikipedia.org/wiki/Five-point_stencil#Higher_derivatives

## ORIGIN

  CWP/SU:Seismic Un*x (SeisUnix)
   https://pubs.usgs.gov/of/2001/of01-326/HTML/SEISUNIX.HTM
   https://github.com/JohnWStockwellJr/SeisUnix

  MADAGASCAR project
   http://www.ahay.org/

## PLAIN, FPGA

  Victor Medeiros
   Thesis: "fastRTM: An IDE to Support Development of the RTM Algorithm in High Performance FPGA platforms", 2013

## SSE

  Alexandros Gremm
   Thesis: "Acceleration, Clustering and Performance Evaluation of Seismic Applications", June 2011
   https://github.com/agremm/Seismic

  Thomas Grosser, Alexandros Gremm, Sebastian Veith, Gerald Heim, Wolfgang Rosenstiel, Victor Medeiros, Manoel Eusebio de Lima
   "Exploiting Heterogeneous Computing Platforms By Cataloging Best Solutions For Resource Intensive Seismic Applications", Mai 2011

## CURRENT

  Patrick Siegl
   Thesis: "Hybride Beschleunigung einer seismischen Applikation durch Kombination von traditionellen Methoden und OpenCL", April 2012
           ("Hybrid acceleration of a seismic application by combining traditional methods with opencl")

