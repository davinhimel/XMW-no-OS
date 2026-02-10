import adf4377

spi_extra = adf4377.linux_spi_init_param()
spi_extra.device_id = 0
spi_extra.chip_select = 0
spi_extra.max_speed_hz = 1000000
spi_extra.mode = 0

spi = adf4377.no_os_spi_init_param()
spi.device_id = 0
spi.max_speed_hz = 1000000
spi.chip_select = 0
spi.mode = adf4377.NO_OS_SPI_MODE_0
spi.bit_order = adf4377.NO_OS_SPI_BIT_ORDER_MSB_FIRST
spi.platform_ops = adf4377.get_linux_spi_ops()
spi.extra = spi_extra

ip = adf4377.adf4377_init_param()
ip.spi_init = spi
ip.spi4wire = True
ip.clkin_freq = 125000000
ip.f_clk = 10000000000
ip.ref_doubler_en = 1
ip.ref_div_factor = 1
ip.cp_i = adf4377.ADF4377_CP_10MA1
ip.clkout_op = adf4377.ADF4377_CLKOUT_640MV
ip.muxout_select = adf4377.ADF4377_MUXOUT_HIGH_Z
ip.dev_id = adf4377.ADF4377

import adf4377, os

try:
    dev = adf4377.adf4377_dev.init(ip)
    print("dev:", dev)
except Exception as e:
    print("init exception:", e)

print("last error:", adf4377.adf4377_get_last_error())
ret = adf4377.adf4377_get_last_error()
if ret < 0:
    print("errno:", -ret, os.strerror(-ret))