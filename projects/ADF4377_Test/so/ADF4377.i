/* adf4377.i - minimal SWIG interface file for adf4377.h */
%module adf4377

/* Tell SWIG where to find the real header */
%{
#include "../../../drivers/frequency/adf4377/adf4377.h"
%}

/* Include common typemaps (very helpful for pointers, ints, etc.) */
%include <typemaps.i>

/* Pull in the entire header — this is the automation part */
%include "../../../drivers/frequency/adf4377/adf4377.h"