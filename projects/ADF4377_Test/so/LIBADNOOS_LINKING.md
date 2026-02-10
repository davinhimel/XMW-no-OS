# How libadnoos SPI Linking Works

## Chain of libraries

```
Python:  import adf4377  →  loads _adf4377.so
_adf4377.so  →  depends on libadf4377.so and libadnoos.so (from your link line)
libadf4377.so  →  depends on libadnoos.so (built that way by make_driver_so.sh)
libadnoos.so   →  provides Linux SPI (spidev), no_os_spi_*, linux_spi_ops
```

- **libadnoos.so** (core): built from `projects/libadnoos/core` (CMake). Contains:
  - `linux_spi.c` → `linux_spi_ops`, `linux_spi_init()`, etc.
  - `no_os_spi.c` → `no_os_spi_init()`, `no_os_spi_write_and_read()`, etc.
  - Util: `no_os_alloc`, `no_os_mutex`, etc.

- **libadf4377.so** (driver): built by `projects/libadnoos/tools/make_driver_so.sh`. Contains:
  - Only `adf4377.c`. It calls `no_os_spi_init()` and uses `init_param->spi_init->platform_ops` (expected to be `&linux_spi_ops`).
  - Linked with `-ladnoos` and `-Wl,-rpath,core/build`, so at runtime it loads **libadnoos.so** to resolve `no_os_spi_init`, `no_os_alloc`, etc.

So: **libadnoos is what provides SPI**. The driver .so does not contain SPI code; it gets it from libadnoos.

## Correct way to build _adf4377.so (SWIG Python module)

1. **Use libadnoos-built libadf4377.so**  
   Build it with:
   ```bash
   cd /home/rpidev1/no-OS/no-OS/projects/libadnoos/tools
   ./make_driver_so.sh ../../../drivers/frequency/adf4377/adf4377.c
   ```
   Output: `projects/libadnoos/drivers_so/libadf4377.so`.

2. **Link order**  
   Your SWIG wrapper (`_adf4377.so`) must link:
   - **libadf4377** first (wrapper calls `adf4377_init`, etc. from it).
   - **libadnoos** second (libadf4377 needs `no_os_*` and `linux_spi_ops` from it).
   So: `-ladf4377 -ladnoos` (order matters for the linker).

3. **Runtime search path (rpath)**  
   When Python loads `_adf4377.so`, the loader must find:
   - `libadf4377.so`
   - `libadnoos.so`
   So embed both dirs in the SWIG-built .so, e.g.:
   ```bash
   -Wl,-rpath,'/path/to/libadnoos/drivers_so:/path/to/libadnoos/core/build'
   ```
   Or set `LD_LIBRARY_PATH` when running Python to include both directories.

4. **Include paths for compiling the wrap**  
   Your `adf4377.h` includes `no_os_spi.h`, `no_os_gpio.h`, etc. So when compiling `ADF4377_wrap.c` you need:
   - `-I no-OS/include`
   - `-I no-OS/drivers/platform/linux`
   - `-I no-OS/drivers/api` (if needed)
   - `-I no-OS/drivers/frequency/adf4377` (or equivalent so `adf4377.h` is found)

## Exact build (from ADF4377_Test/so)

Use the Makefile target (recommended):

```bash
cd /home/rpidev1/no-OS/no-OS/projects/ADF4377_Test/so
make swig-adf4377
```

Or by hand (after `swig -python -I... ADF4377.i` to generate `ADF4377_wrap.c`):

```bash
NO_OS=/home/rpidev1/no-OS/no-OS
LIBADNOOS=$NO_OS/projects/libadnoos
CORE=$LIBADNOOS/core/build
DRV=$LIBADNOOS/drivers_so

gcc -shared -fPIC -O2 \
  -I$NO_OS -I$NO_OS/include -I$NO_OS/drivers/api \
  -I$NO_OS/drivers/platform/linux -I$NO_OS/drivers/frequency/adf4377 -I$NO_OS/util \
  $(python3-config --includes) \
  ADF4377_wrap.c \
  -L$DRV -L$CORE -ladf4377 -ladnoos \
  -Wl,-rpath,$DRV:$CORE \
  $(python3-config --ldflags) -lpthread -lm \
  -o _adf4377.so
```

Run Python with rpath or LD_LIBRARY_PATH so both libs are found:

```bash
LD_LIBRARY_PATH=$DRV:$CORE python3 -c "import adf4377; print('OK')"
```

## Summary

- **SPI linking is done by libadnoos**: the driver .so is built against libadnoos and gets SPI from it at runtime.
- Your SWIG build must link **both** `libadf4377` and `libadnoos` and set **rpath** (or `LD_LIBRARY_PATH`) so both .so files are found when Python imports the module.
