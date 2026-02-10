/* adf4377.i - SWIG interface for adf4377 driver and no-OS SPI/linux types.
 *
 * Structure:
 *   1. Integer "in" typemaps  - Python int/long/bool -> C stdint/bool (for args and struct setters)
 *   2. OUTPUT typemaps        - C pointer outputs -> Python return tuple
 *   3. SPI/init includes     - no_os_spi, linux_spi, get_linux_spi_ops
 *   4. Main header           - adf4377.h
 *
 * This allows natural Python:  obj.device_id = 0;  ret, val = adf4377_spi_read(dev, reg)
 */
%module adf4377

%{
#include "../../../drivers/frequency/adf4377/adf4377.h"
#include "../../../drivers/platform/linux/linux_spi.h"
%}

%include <typemaps.i>
/* stdint types (uint32_t etc.) come from parsed headers (e.g. linux_spi.h includes stdint.h) */

/* =============================================================================
 * Integer "in" typemaps: accept Python int/long/bool for C stdint and bool.
 * Used for function arguments and struct member setters (e.g. device_id = 0).
 * ============================================================================= */
%typemap(in) uint8_t {
  unsigned long v = PyLong_AsUnsignedLongMask($input);
  if (PyErr_Occurred()) SWIG_fail;
  $1 = (uint8_t)(v & 0xFF);
}
%typemap(in) uint16_t {
  unsigned long v = PyLong_AsUnsignedLongMask($input);
  if (PyErr_Occurred()) SWIG_fail;
  $1 = (uint16_t)(v & 0xFFFF);
}
%typemap(in) uint32_t {
  unsigned long v = PyLong_AsUnsignedLongMask($input);
  if (PyErr_Occurred()) SWIG_fail;
  $1 = (uint32_t)(v & 0xFFFFFFFFUL);
}
%typemap(in) uint64_t {
  $1 = (uint64_t) PyLong_AsUnsignedLongLong($input);
  if (PyErr_Occurred()) SWIG_fail;
}
%typemap(in) int8_t {
  long v = PyLong_AsLong($input);
  if (PyErr_Occurred()) SWIG_fail;
  $1 = (int8_t)(v & 0xFF);
}
%typemap(in) int16_t {
  long v = PyLong_AsLong($input);
  if (PyErr_Occurred()) SWIG_fail;
  $1 = (int16_t)(v & 0xFFFF);
}
%typemap(in) int32_t {
  long v = PyLong_AsLong($input);
  if (PyErr_Occurred()) SWIG_fail;
  $1 = (int32_t) v;
}
%typemap(in) int64_t {
  $1 = (int64_t) PyLong_AsLongLong($input);
  if (PyErr_Occurred()) SWIG_fail;
}
%typemap(in) bool {
  if ($input == Py_None) { $1 = false; }
  else { $1 = (bool) PyObject_IsTrue($input); }
}

/* Return int32_t as Python int (default can be pointer on some setups) */
%typemap(out) int32_t {
  $result = PyLong_FromLong((long)$1);
}

/* =============================================================================
 * OUTPUT typemaps: pointer output parameters -> append to Python return tuple.
 * So:  ret, value = adf4377_spi_read(dev, reg)  works.
 * ============================================================================= */
/* Manual OUTPUT typemaps for stdint/bool (typemaps.i has no OUTPUT for these).
   From Python you call func(dev, ...) and get (ret, value) back. */
%typemap(in, numinputs=0) uint8_t *data (uint8_t temp) {
  $1 = &temp;
}
%typemap(argout) uint8_t *data {
  PyObject *o = PyLong_FromLong((long)(unsigned char)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) uint64_t *val (uint64_t temp) {
  $1 = &temp;
}
%typemap(argout) uint64_t *val {
  PyObject *o = PyLong_FromUnsignedLongLong(*$1);
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) int32_t *div (int32_t temp) {
  $1 = &temp;
}
%typemap(argout) int32_t *div {
  PyObject *o = PyLong_FromLong((long)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) int32_t *reg_val (int32_t temp) {
  $1 = &temp;
}
%typemap(argout) int32_t *reg_val {
  PyObject *o = PyLong_FromLong((long)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) int32_t *word (int32_t temp) {
  $1 = &temp;
}
%typemap(argout) int32_t *word {
  PyObject *o = PyLong_FromLong((long)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) int32_t *val (int32_t temp) {
  $1 = &temp;
}
%typemap(argout) int32_t *val {
  PyObject *o = PyLong_FromLong((long)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) int8_t *pwr (int8_t temp) {
  $1 = &temp;
}
%typemap(argout) int8_t *pwr {
  PyObject *o = PyLong_FromLong((long)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) int8_t *div (int8_t temp) {
  $1 = &temp;
}
%typemap(argout) int8_t *div {
  PyObject *o = PyLong_FromLong((long)(*$1));
  $result = SWIG_Python_AppendOutput($result, o);
}
%typemap(in, numinputs=0) bool *en (bool temp) {
  $1 = &temp;
}
%typemap(argout) bool *en {
  PyObject *o = (*$1) ? Py_True : Py_False;
  Py_INCREF(o);
  $result = SWIG_Python_AppendOutput($result, o);
}

/* adf4377_init(adf4377_dev **device, adf4377_init_param *init_param): return (ret, dev) from Python */
%typemap(in, numinputs=0) struct adf4377_dev **device (struct adf4377_dev *temp = 0) {
  $1 = &temp;
}
%typemap(argout) struct adf4377_dev **device {
  PyObject *o = SWIG_NewPointerObj(SWIG_as_voidptr(*$1), $*1_descriptor, SWIG_POINTER_OWN);
  $result = SWIG_Python_AppendOutput($result, o);
}

/* Explicitly include SPI init types so they are always wrapped (needed for adf4377_init from Python). */
%include "no_os_spi.h"
%include "linux_spi.h"

/* Expose linux_spi_ops pointer for use in no_os_spi_init_param.platform_ops */
%inline %{
const struct no_os_spi_platform_ops* get_linux_spi_ops(void) {
  return &linux_spi_ops;
}
%}

/* Pull in the entire header — this is the automation part */
%include "../../../drivers/frequency/adf4377/adf4377.h"