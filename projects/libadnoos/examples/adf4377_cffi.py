#!/usr/bin/env python3
"""
CFFI wrapper for ADF4377 driver
Uses your existing libadf4377.so and libadnoos.so
"""
from cffi import FFI
from pathlib import Path
import os

# Set library path
SCRIPT_DIR = Path(__file__).parent.absolute()
PROJECT_ROOT = SCRIPT_DIR.parent
os.environ['LD_LIBRARY_PATH'] = f"{PROJECT_ROOT}/drivers_so:{PROJECT_ROOT}/core/build"

# Initialize CFFI
ffi = FFI()

# Define all C types and structures
ffi.cdef("""
    // Basic types
    typedef uint8_t bool;
    
    // Enums
    enum adf4377_dev_id {
        ADF4377 = 0x05,
        ADF4378 = 0x06
    };
    
    enum no_os_spi_mode {
        NO_OS_SPI_MODE_0 = 0,
        NO_OS_SPI_MODE_1 = 1,
        NO_OS_SPI_MODE_2 = 2,
        NO_OS_SPI_MODE_3 = 3
    };
    
    enum no_os_spi_bit_order {
        NO_OS_SPI_BIT_ORDER_MSB_FIRST = 0,
        NO_OS_SPI_BIT_ORDER_LSB_FIRST = 1
    };
    
    enum no_os_spi_lanes {
        NO_OS_SPI_SINGLE_LANE = 0,
        NO_OS_SPI_DUAL_LANE = 1,
        NO_OS_SPI_QUAD_LANE = 2,
        NO_OS_SPI_OCTO_LANE = 3
    };
    
    // Opaque types (we only pass pointers)
    typedef struct adf4377_dev adf4377_dev;
    typedef struct no_os_spi_desc no_os_spi_desc;
    typedef struct no_os_gpio_desc no_os_gpio_desc;
    typedef struct no_os_spi_platform_ops no_os_spi_platform_ops;
    typedef struct no_os_platform_spi_delays no_os_platform_spi_delays;
    
    // Linux SPI extra structure
    struct linux_spi_init_param {
        uint32_t device_id;
        uint8_t chip_select;
        uint32_t max_speed_hz;
        uint8_t mode;
    };
    
    // no-OS SPI init param structure
    struct no_os_spi_init_param {
        uint32_t device_id;
        uint32_t max_speed_hz;
        uint8_t chip_select;
        enum no_os_spi_mode mode;
        enum no_os_spi_bit_order bit_order;
        enum no_os_spi_lanes lanes;
        const struct no_os_spi_platform_ops *platform_ops;
        struct no_os_platform_spi_delays *platform_delays;
        void *extra;
        struct no_os_spi_desc *parent;
    };
    
    // GPIO init param (simplified - we'll set to NULL)
    struct no_os_gpio_init_param {
        uint32_t number;
        const void *platform_ops;
        void *extra;
    };
    
    // ADF4377 init param structure
    struct adf4377_init_param {
        struct no_os_spi_init_param *spi_init;
        struct no_os_gpio_init_param *gpio_ce_param;
        struct no_os_gpio_init_param *gpio_enclk1_param;
        struct no_os_gpio_init_param *gpio_enclk2_param;
        enum adf4377_dev_id dev_id;
        bool spi4wire;
        uint64_t clkin_freq;
        uint64_t f_clk;
        uint8_t cp_i;
        uint32_t muxout_select;
        uint8_t ref_doubler_en;
        uint8_t clkout_op;
        uint32_t ref_div_factor;
    };
    
    // Function declarations
    int32_t adf4377_init(adf4377_dev **device, struct adf4377_init_param *init_param);
    int32_t adf4377_remove(adf4377_dev *dev);
    
    int adf4377_spi_read(adf4377_dev *dev, uint8_t reg_addr, uint8_t *data);
    int adf4377_spi_write(adf4377_dev *dev, uint8_t reg_addr, uint8_t data);
    int adf4377_spi_update_bit(adf4377_dev *dev, uint16_t reg_addr, uint8_t mask, uint8_t data);
    
    int adf4377_set_rfout(adf4377_dev *dev, uint64_t val);
    int adf4377_get_rfout(adf4377_dev *dev, uint64_t *val);
    int adf4377_set_freq(adf4377_dev *dev);
    
    int adf4377_set_ref_clk(adf4377_dev *dev, uint64_t val);
    int adf4377_get_ref_clk(adf4377_dev *dev, uint64_t *val);
    int adf4377_set_en_ref_doubler(adf4377_dev *dev, bool en);
    int adf4377_get_en_ref_doubler(adf4377_dev *dev, bool *en);
    int adf4377_set_ref_div(adf4377_dev *dev, int32_t div);
    int adf4377_get_ref_div(adf4377_dev *dev, int32_t *div);
    int adf4377_set_cp_i(adf4377_dev *dev, int32_t reg_val);
    int adf4377_get_cp_i(adf4377_dev *dev, int32_t *reg_val);
    
    // Get linux_spi_ops from core library
    extern const struct no_os_spi_platform_ops linux_spi_ops;
""")

# Load the libraries
# First load core library to get linux_spi_ops
core_lib = ffi.dlopen(f"{PROJECT_ROOT}/core/build/libadnoos.so")
# Then load driver library
lib = ffi.dlopen(f"{PROJECT_ROOT}/drivers_so/libadf4377.so")

# Get linux_spi_ops pointer
try:
    linux_spi_ops = ffi.addressof(core_lib, 'linux_spi_ops')
except:
    # Fallback: try to get it via dlsym
    import ctypes
    libdl = ctypes.CDLL("libdl.so.2")
    libdl.dlsym.restype = ctypes.c_void_p
    handle = core_lib._handle if hasattr(core_lib, '_handle') else None
    if handle:
        linux_spi_ops_addr = libdl.dlsym(handle, b"linux_spi_ops")
        linux_spi_ops = ffi.cast("const struct no_os_spi_platform_ops *", linux_spi_ops_addr)
    else:
        linux_spi_ops = ffi.NULL


class ADF4377:
    """Python-friendly wrapper for ADF4377 driver"""
    
    # Constants
    ADF4377_REG = lambda x: x
    ADF4377_MUXOUT_MSK = 0xF0
    ADF4377_MUXOUT_HIGH = 0x8
    ADF4377_MUXOUT = lambda x: ((x & 0x0F) << 4) & 0xF0
    
    def __init__(self, device_id=0, chip_select=0, max_speed_hz=10000, 
                 clkin_freq=125000000, f_clk=10000000000, 
                 ref_doubler_en=1, ref_div_factor=1, cp_i=0x0F, 
                 clkout_op=3, muxout_select=0):
        """
        Initialize ADF4377 driver
        
        Args:
            device_id: SPI device ID (0 = /dev/spidev0.X)
            chip_select: SPI chip select (0 or 1)
            max_speed_hz: SPI speed in Hz (default 10000 = 10 kHz)
            clkin_freq: Input reference clock frequency (default 125 MHz)
            f_clk: Desired output frequency (default 10 GHz)
            ref_doubler_en: Enable reference doubler (default 1)
            ref_div_factor: Reference divider factor (default 1)
            cp_i: Charge pump current (default 0x0F = ADF4377_CP_10MA1)
            clkout_op: Clock output amplitude (default 3 = ADF4377_CLKOUT_640MV)
            muxout_select: MUXOUT selection (default 0 = ADF4377_MUXOUT_HIGH_Z)
        """
        self.dev = None
        
        # Create linux_spi_extra structure
        self.linux_spi_extra = ffi.new("struct linux_spi_init_param *", {
            'device_id': device_id,
            'chip_select': chip_select,
            'max_speed_hz': max_speed_hz,
            'mode': 0  # SPI_MODE_0
        })
        
        # Create no_os_spi_init_param structure
        self.spi_init_param = ffi.new("struct no_os_spi_init_param *", {
            'device_id': device_id,
            'max_speed_hz': max_speed_hz,
            'chip_select': chip_select,
            'mode': 0,  # NO_OS_SPI_MODE_0
            'bit_order': 0,  # NO_OS_SPI_BIT_ORDER_MSB_FIRST
            'lanes': 0,  # NO_OS_SPI_SINGLE_LANE
            'platform_ops': linux_spi_ops,
            'platform_delays': ffi.NULL,
            'extra': self.linux_spi_extra,
            'parent': ffi.NULL
        })
        
        # Create adf4377_init_param structure
        self.init_param = ffi.new("struct adf4377_init_param *", {
            'spi_init': self.spi_init_param,
            'gpio_ce_param': ffi.NULL,
            'gpio_enclk1_param': ffi.NULL,
            'gpio_enclk2_param': ffi.NULL,
            'dev_id': 0x05,  # ADF4377
            'spi4wire': True,
            'clkin_freq': clkin_freq,
            'f_clk': f_clk,
            'cp_i': cp_i,
            'muxout_select': muxout_select,
            'ref_doubler_en': ref_doubler_en,
            'clkout_op': clkout_op,
            'ref_div_factor': ref_div_factor
        })
    
    def init(self):
        """Initialize the ADF4377 device"""
        dev_ptr = ffi.new("adf4377_dev **")
        ret = lib.adf4377_init(dev_ptr, self.init_param)
        if ret != 0:
            raise RuntimeError(f"adf4377_init failed with error {ret}")
        self.dev = dev_ptr[0]
        return ret
    
    def remove(self):
        """Clean up and remove the device"""
        if self.dev:
            ret = lib.adf4377_remove(self.dev)
            self.dev = None
            return ret
        return 0
    
    def read_register(self, reg_addr):
        """Read a register value"""
        if not self.dev:
            raise RuntimeError("Device not initialized. Call init() first.")
        data = ffi.new("uint8_t *")
        ret = lib.adf4377_spi_read(self.dev, reg_addr, data)
        if ret != 0:
            raise RuntimeError(f"adf4377_spi_read failed with error {ret}")
        return data[0]
    
    def write_register(self, reg_addr, data):
        """Write a register value"""
        if not self.dev:
            raise RuntimeError("Device not initialized. Call init() first.")
        ret = lib.adf4377_spi_write(self.dev, reg_addr, data)
        if ret != 0:
            raise RuntimeError(f"adf4377_spi_write failed with error {ret}")
        return ret
    
    def update_register_bits(self, reg_addr, mask, data):
        """Update specific bits in a register"""
        if not self.dev:
            raise RuntimeError("Device not initialized. Call init() first.")
        ret = lib.adf4377_spi_update_bit(self.dev, reg_addr, mask, data)
        if ret != 0:
            raise RuntimeError(f"adf4377_spi_update_bit failed with error {ret}")
        return ret
    
    def set_muxout_high(self):
        """Set MUXOUT to HIGH output"""
        muxout_value = self.ADF4377_MUXOUT(self.ADF4377_MUXOUT_HIGH)
        return self.update_register_bits(
            self.ADF4377_REG(0x1D),
            self.ADF4377_MUXOUT_MSK,
            muxout_value
        )
    
    def set_rfout(self, frequency_hz):
        """Set RF output frequency"""
        if not self.dev:
            raise RuntimeError("Device not initialized. Call init() first.")
        ret = lib.adf4377_set_rfout(self.dev, frequency_hz)
        if ret != 0:
            raise RuntimeError(f"adf4377_set_rfout failed with error {ret}")
        return ret
    
    def get_rfout(self):
        """Get current RF output frequency"""
        if not self.dev:
            raise RuntimeError("Device not initialized. Call init() first.")
        val = ffi.new("uint64_t *")
        ret = lib.adf4377_get_rfout(self.dev, val)
        if ret != 0:
            raise RuntimeError(f"adf4377_get_rfout failed with error {ret}")
        return val[0]
    
    def __enter__(self):
        """Context manager entry"""
        self.init()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.remove()


# Example usage
if __name__ == "__main__":
    print("ADF4377 CFFI Wrapper Test")
    print("=" * 50)
    
    # Create device instance
    chip = ADF4377(
        device_id=0,
        chip_select=0,
        max_speed_hz=10000,  # 10 kHz
        clkin_freq=125000000,  # 125 MHz
        f_clk=10000000000,  # 10 GHz
    )
    
    try:
        # Initialize
        print("Initializing ADF4377...")
        chip.init()
        print("✓ ADF4377 initialized successfully")
        
        # Read current MUXOUT register
        print("\nReading REG001D (MUXOUT register)...")
        reg_value = chip.read_register(0x1D)
        muxout_bits = (reg_value >> 4) & 0x0F
        print(f"  Current REG001D value: 0x{reg_value:02X}")
        print(f"  MUXOUT bits [7:4]: 0x{muxout_bits:01X}")
        
        # Write HIGH to MUXOUT
        print("\nWriting HIGH to MUXOUT bits [7:4]...")
        chip.set_muxout_high()
        print("  ✓ Successfully wrote MUXOUT = HIGH")
        
        # Read back to verify
        print("\nVerifying write...")
        reg_value = chip.read_register(0x1D)
        muxout_bits = (reg_value >> 4) & 0x0F
        print(f"  REG001D value after write: 0x{reg_value:02X}")
        print(f"  MUXOUT bits [7:4]: 0x{muxout_bits:01X}", end="")
        if muxout_bits == chip.ADF4377_MUXOUT_HIGH:
            print(" ✓ (HIGH - correct!)")
        else:
            print(f" ✗ (Expected 0x{chip.ADF4377_MUXOUT_HIGH:X})")
        
        # Set frequencies
        print("\nSetting frequencies...")
        chip.set_rfout(11500000000)  # 11.5 GHz
        print("  ✓ 11.5 GHz")
        chip.set_rfout(8000000000)  # 8.0 GHz
        print("  ✓ 8.0 GHz")
        
    except Exception as e:
        print(f"✗ Error: {e}")
    finally:
        # Cleanup
        print("\nCleaning up...")
        chip.remove()
        print("✓ Done")
    
    print("\n" + "=" * 50)
    print("Test completed!")

