#!/usr/bin/env python3
"""
ADF4382 and no-OS Chip Control GUI
A scalable Python GUI application for controlling Analog Devices chips via shared libraries.
"""

import sys
import os
import json
import ctypes
import threading
from typing import Dict, Any, Optional, List
from pathlib import Path

# Try to import PyQt5, fallback to Tkinter if not available
try:
    from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, 
                                QWidget, QComboBox, QLabel, QLineEdit, QPushButton, 
                                QTextEdit, QGroupBox, QGridLayout, QMessageBox, 
                                QSpinBox, QDoubleSpinBox, QCheckBox, QFormLayout)
    from PyQt5.QtCore import Qt, QThread, pyqtSignal
    from PyQt5.QtGui import QFont, QTextCursor
    USE_PYQT5 = True
except ImportError:
    try:
        import tkinter as tk
        from tkinter import ttk, messagebox, scrolledtext
        USE_PYQT5 = False
    except ImportError:
        print("Error: Neither PyQt5 nor tkinter is available.")
        print("Please install PyQt5: pip install PyQt5")
        print("Or use a Python installation with tkinter support.")
        sys.exit(1)

class ChipConfig:
    """Configuration class for chip parameters and functions."""
    
    def __init__(self, config_file: str):
        """Initialize chip configuration from JSON file."""
        with open(config_file, 'r') as f:
            self.config = json.load(f)
        
        self.chip_name = self.config['chip_name']
        self.lib_name = self.config['lib_name']
        self.init_func = self.config['init_func']
        self.remove_func = self.config['remove_func']
        self.params = self.config['params']
        self.setters = self.config.get('setters', [])
        self.spi_defaults = self.config.get('spi_defaults', {})
        
        # Load shared library
        self.lib = None
        self.device = None
        self.load_library()
    
    def load_library(self):
        """Load the shared library and resolve function symbols."""
        try:
            lib_path = Path(self.lib_name)
            if not lib_path.is_absolute():
                lib_path = Path.cwd() / self.lib_name
            
            self.lib = ctypes.CDLL(str(lib_path))
            print(f"Loaded library: {lib_path}")
        except Exception as e:
            print(f"Error loading library {self.lib_name}: {e}")
            self.lib = None
    
    def get_param_default(self, param_name: str) -> Any:
        """Get default value for a parameter."""
        for param in self.params:
            if param['name'] == param_name:
                return param['default']
        return None
    
    def get_param_range(self, param_name: str) -> List[Any]:
        """Get range for a parameter."""
        for param in self.params:
            if param['name'] == param_name:
                return param.get('range', [])
        return []

class ChipController:
    """Controller class for chip operations."""
    
    def __init__(self, chip_config: ChipConfig):
        """Initialize chip controller."""
        self.config = chip_config
        self.device = None
        self.initialized = False
    
    def initialize_chip(self, spi_device_id: int = 0) -> bool:
        """Initialize the chip."""
        if not self.config.lib:
            return False
        
        try:
            # Define C structures for initialization
            class SPIInitParam(ctypes.Structure):
                _fields_ = [
                    ("device_id", ctypes.c_uint32),
                    ("max_speed_hz", ctypes.c_uint32),
                    ("bit_order", ctypes.c_uint32),
                    ("mode", ctypes.c_uint32),
                    ("platform_ops", ctypes.c_void_p),
                    ("chip_select", ctypes.c_uint32),
                    ("extra", ctypes.c_void_p)
                ]
            
            class ChipInitParam(ctypes.Structure):
                _fields_ = [
                    ("spi_init", ctypes.POINTER(SPIInitParam)),
                    ("spi_3wire_en", ctypes.c_bool),
                    ("cmos_3v3", ctypes.c_bool),
                    ("ref_freq_hz", ctypes.c_uint64),
                    ("freq", ctypes.c_uint64),
                    ("ref_doubler_en", ctypes.c_bool),
                    ("ref_div", ctypes.c_uint8),
                    ("cp_i", ctypes.c_uint8),
                    ("bleed_word", ctypes.c_uint16),
                    ("ld_count", ctypes.c_uint8),
                    ("en_lut_gen", ctypes.c_uint8),
                    ("en_lut_cal", ctypes.c_uint8),
                    ("max_lpf_cap_value_uf", ctypes.c_uint8),
                    ("id", ctypes.c_uint8)
                ]
            
            # Set up SPI parameters
            spi_param = SPIInitParam(
                device_id=spi_device_id,
                max_speed_hz=self.config.spi_defaults.get('max_speed_hz', 1500000),
                bit_order=0,  # MSB first
                mode=self.config.spi_defaults.get('mode', 0),
                platform_ops=None,
                chip_select=self.config.spi_defaults.get('chip_select', 0),
                extra=None
            )
            
            # Set up chip parameters
            chip_param = ChipInitParam(
                spi_init=ctypes.byref(spi_param),
                spi_3wire_en=False,
                cmos_3v3=False,
                ref_freq_hz=self.config.get_param_default('ref_freq_hz'),
                freq=self.config.get_param_default('freq'),
                ref_doubler_en=self.config.get_param_default('ref_doubler_en'),
                ref_div=self.config.get_param_default('ref_div'),
                cp_i=self.config.get_param_default('cp_i'),
                bleed_word=self.config.get_param_default('bleed_word'),
                ld_count=self.config.get_param_default('ld_count'),
                en_lut_gen=0,
                en_lut_cal=0,
                max_lpf_cap_value_uf=10,
                id=0  # ADF4382
            )
            
            # Set function signatures
            self.config.lib.adf4382_init.argtypes = [ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ChipInitParam)]
            self.config.lib.adf4382_init.restype = ctypes.c_int
            
            # Initialize chip
            device_ptr = ctypes.c_void_p()
            result = self.config.lib.adf4382_init(ctypes.byref(device_ptr), ctypes.byref(chip_param))
            
            if result == 0:
                self.device = device_ptr.value
                self.initialized = True
                return True
            else:
                return False
                
        except Exception as e:
            print(f"Error initializing chip: {e}")
            return False
    
    def set_parameter(self, param_name: str, value: Any) -> bool:
        """Set a chip parameter."""
        if not self.initialized or not self.device:
            return False
        
        try:
            # Find the setter function for this parameter
            setter_func = None
            for setter in self.config.setters:
                if setter['param'] == param_name:
                    setter_func = setter['func']
                    param_type = setter['type']
                    break
            
            if not setter_func:
                return False
            
            # Get function from library
            func = getattr(self.config.lib, setter_func)
            
            # Set function signature based on parameter type
            if param_type == 'uint64':
                func.argtypes = [ctypes.c_void_p, ctypes.c_uint64]
                func.restype = ctypes.c_int
                result = func(self.device, ctypes.c_uint64(value))
            elif param_type == 'int32':
                func.argtypes = [ctypes.c_void_p, ctypes.c_int32]
                func.restype = ctypes.c_int
                result = func(self.device, ctypes.c_int32(value))
            elif param_type == 'uint32':
                func.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
                func.restype = ctypes.c_int
                result = func(self.device, ctypes.c_uint32(value))
            else:
                return False
            
            return result == 0
            
        except Exception as e:
            print(f"Error setting parameter {param_name}: {e}")
            return False
    
    def cleanup(self):
        """Clean up chip resources."""
        if self.initialized and self.device and self.config.lib:
            try:
                self.config.lib.adf4382_remove.argtypes = [ctypes.c_void_p]
                self.config.lib.adf4382_remove.restype = ctypes.c_int
                self.config.lib.adf4382_remove(self.device)
            except Exception as e:
                print(f"Error during cleanup: {e}")
            finally:
                self.device = None
                self.initialized = False

class PyQt5GUI(QMainWindow):
    """PyQt5-based GUI for chip control."""
    
    def __init__(self):
        super().__init__()
        self.chip_configs = {}
        self.current_chip = None
        self.chip_controller = None
        self.param_widgets = {}
        self.init_ui()
        self.load_chip_configs()
    
    def init_ui(self):
        """Initialize the user interface."""
        self.setWindowTitle("ADF4382 and no-OS Chip Control")
        self.setGeometry(100, 100, 800, 600)
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        layout = QVBoxLayout(central_widget)
        
        # Chip selector
        chip_group = QGroupBox("Chip Selection")
        chip_layout = QHBoxLayout(chip_group)
        
        chip_layout.addWidget(QLabel("Select Chip:"))
        self.chip_combo = QComboBox()
        self.chip_combo.currentTextChanged.connect(self.on_chip_selected)
        chip_layout.addWidget(self.chip_combo)
        
        layout.addWidget(chip_group)
        
        # SPI Configuration
        spi_group = QGroupBox("SPI Configuration")
        spi_layout = QFormLayout(spi_group)
        
        self.spi_device_spin = QSpinBox()
        self.spi_device_spin.setRange(0, 10)
        self.spi_device_spin.setValue(0)
        spi_layout.addRow("SPI Device ID:", self.spi_device_spin)
        
        self.spi_cs_spin = QSpinBox()
        self.spi_cs_spin.setRange(0, 10)
        self.spi_cs_spin.setValue(0)
        spi_layout.addRow("Chip Select:", self.spi_cs_spin)
        
        layout.addWidget(spi_group)
        
        # Parameters
        self.params_group = QGroupBox("Chip Parameters")
        self.params_layout = QFormLayout(self.params_group)
        layout.addWidget(self.params_group)
        
        # Control buttons
        button_layout = QHBoxLayout()
        
        self.init_button = QPushButton("Initialize Chip")
        self.init_button.clicked.connect(self.initialize_chip)
        button_layout.addWidget(self.init_button)
        
        self.update_button = QPushButton("Update All Parameters")
        self.update_button.clicked.connect(self.update_all_params)
        self.update_button.setEnabled(False)
        button_layout.addWidget(self.update_button)
        
        self.read_button = QPushButton("Read Status")
        self.read_button.clicked.connect(self.read_status)
        self.read_button.setEnabled(False)
        button_layout.addWidget(self.read_button)
        
        layout.addLayout(button_layout)
        
        # Log console
        log_group = QGroupBox("Log Console")
        log_layout = QVBoxLayout(log_group)
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setFont(QFont("Courier", 9))
        log_layout.addWidget(self.log_text)
        
        layout.addWidget(log_group)
        
        self.log("GUI initialized. Select a chip to begin.")
    
    def load_chip_configs(self):
        """Load chip configurations from JSON files."""
        config_dir = Path("configs")
        if not config_dir.exists():
            self.log("Configs directory not found. Creating sample config...")
            self.create_sample_config()
        
        for config_file in config_dir.glob("*.json"):
            try:
                chip_config = ChipConfig(str(config_file))
                self.chip_configs[chip_config.chip_name] = chip_config
                self.chip_combo.addItem(chip_config.chip_name)
                self.log(f"Loaded config: {chip_config.chip_name}")
            except Exception as e:
                self.log(f"Error loading config {config_file}: {e}")
    
    def create_sample_config(self):
        """Create a sample ADF4382 configuration file."""
        config_dir = Path("configs")
        config_dir.mkdir(exist_ok=True)
        
        sample_config = {
            "chip_name": "ADF4382",
            "lib_name": "libadf4382.so",
            "init_func": "adf4382_init",
            "remove_func": "adf4382_remove",
            "params": [
                {"name": "ref_freq_hz", "type": "uint64", "default": 125000000, "range": [10000000, 5000000000]},
                {"name": "freq", "type": "uint64", "default": 20000000000, "range": [687500000, 22000000000]},
                {"name": "cp_i", "type": "uint8", "default": 15, "range": [0, 15]},
                {"name": "bleed_word", "type": "uint16", "default": 4903, "range": [0, 8191]},
                {"name": "ref_doubler_en", "type": "bool", "default": True},
                {"name": "ref_div", "type": "uint8", "default": 1, "range": [1, 63]},
                {"name": "ld_count", "type": "uint8", "default": 10, "range": [0, 255]}
            ],
            "setters": [
                {"func": "adf4382_set_rfout", "param": "freq", "type": "uint64"},
                {"func": "adf4382_set_cp_i", "param": "cp_i", "type": "int32"},
                {"func": "adf4382_set_bleed_word", "param": "bleed_word", "type": "int32"},
                {"func": "adf4382_set_phase_adjust", "param": "phase_ps", "type": "uint32"}
            ],
            "spi_defaults": {"device_id": 0, "chip_select": 0, "max_speed_hz": 1500000, "mode": 0}
        }
        
        with open(config_dir / "adf4382.json", 'w') as f:
            json.dump(sample_config, f, indent=2)
        
        self.log("Created sample ADF4382 configuration file.")
    
    def on_chip_selected(self, chip_name: str):
        """Handle chip selection."""
        if not chip_name or chip_name not in self.chip_configs:
            return
        
        self.current_chip = self.chip_configs[chip_name]
        self.create_parameter_widgets()
        self.log(f"Selected chip: {chip_name}")
    
    def create_parameter_widgets(self):
        """Create parameter input widgets based on selected chip."""
        # Clear existing widgets
        for i in reversed(range(self.params_layout.rowCount())):
            self.params_layout.removeRow(i)
        
        self.param_widgets.clear()
        
        if not self.current_chip:
            return
        
        for param in self.current_chip.params:
            param_name = param['name']
            param_type = param['type']
            default_value = param['default']
            param_range = param.get('range', [])
            
            # Create appropriate widget based on parameter type
            if param_type == 'uint64':
                widget = QSpinBox()
                widget.setRange(0, 2**63 - 1)
                if param_range:
                    widget.setRange(param_range[0], param_range[1])
                widget.setValue(default_value)
            elif param_type == 'uint32':
                widget = QSpinBox()
                widget.setRange(0, 2**32 - 1)
                if param_range:
                    widget.setRange(param_range[0], param_range[1])
                widget.setValue(default_value)
            elif param_type == 'uint16':
                widget = QSpinBox()
                widget.setRange(0, 65535)
                if param_range:
                    widget.setRange(param_range[0], param_range[1])
                widget.setValue(default_value)
            elif param_type == 'uint8':
                widget = QSpinBox()
                widget.setRange(0, 255)
                if param_range:
                    widget.setRange(param_range[0], param_range[1])
                widget.setValue(default_value)
            elif param_type == 'bool':
                widget = QCheckBox()
                widget.setChecked(default_value)
            else:
                widget = QLineEdit()
                widget.setText(str(default_value))
            
            self.param_widgets[param_name] = widget
            self.params_layout.addRow(f"{param_name}:", widget)
    
    def initialize_chip(self):
        """Initialize the selected chip."""
        if not self.current_chip:
            self.log("No chip selected.")
            return
        
        try:
            self.chip_controller = ChipController(self.current_chip)
            spi_device_id = self.spi_device_spin.value()
            
            if self.chip_controller.initialize_chip(spi_device_id):
                self.log(f"Successfully initialized {self.current_chip.chip_name}")
                self.init_button.setEnabled(False)
                self.update_button.setEnabled(True)
                self.read_button.setEnabled(True)
            else:
                self.log(f"Failed to initialize {self.current_chip.chip_name}")
                
        except Exception as e:
            self.log(f"Error during initialization: {e}")
    
    def update_all_params(self):
        """Update all parameters for the current chip."""
        if not self.chip_controller or not self.chip_controller.initialized:
            self.log("Chip not initialized.")
            return
        
        try:
            for param_name, widget in self.param_widgets.items():
                if isinstance(widget, QCheckBox):
                    value = widget.isChecked()
                elif isinstance(widget, (QSpinBox, QDoubleSpinBox)):
                    value = widget.value()
                else:
                    value = widget.text()
                
                if self.chip_controller.set_parameter(param_name, value):
                    self.log(f"Updated {param_name}: {value}")
                else:
                    self.log(f"Failed to update {param_name}")
            
            self.log("Parameter update completed.")
            
        except Exception as e:
            self.log(f"Error updating parameters: {e}")
    
    def read_status(self):
        """Read chip status."""
        if not self.chip_controller or not self.chip_controller.initialized:
            self.log("Chip not initialized.")
            return
        
        self.log("Reading chip status...")
        # This would typically call getter functions from the library
        self.log("Status read completed.")
    
    def log(self, message: str):
        """Add message to log console."""
        self.log_text.append(f"[{QApplication.instance().applicationName()}] {message}")
        self.log_text.moveCursor(QTextCursor.End)
    
    def closeEvent(self, event):
        """Handle application close event."""
        if self.chip_controller:
            self.chip_controller.cleanup()
        event.accept()

class TkinterGUI:
    """Tkinter-based GUI for chip control (fallback)."""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("ADF4382 and no-OS Chip Control")
        self.root.geometry("800x600")
        
        self.chip_configs = {}
        self.current_chip = None
        self.chip_controller = None
        self.param_widgets = {}
        
        self.init_ui()
        self.load_chip_configs()
    
    def init_ui(self):
        """Initialize the user interface."""
        # Chip selector
        chip_frame = ttk.LabelFrame(self.root, text="Chip Selection")
        chip_frame.pack(fill="x", padx=5, pady=5)
        
        ttk.Label(chip_frame, text="Select Chip:").pack(side="left")
        self.chip_combo = ttk.Combobox(chip_frame, state="readonly")
        self.chip_combo.pack(side="left", padx=5)
        self.chip_combo.bind("<<ComboboxSelected>>", self.on_chip_selected)
        
        # SPI Configuration
        spi_frame = ttk.LabelFrame(self.root, text="SPI Configuration")
        spi_frame.pack(fill="x", padx=5, pady=5)
        
        spi_grid = ttk.Frame(spi_frame)
        spi_grid.pack(fill="x", padx=5, pady=5)
        
        ttk.Label(spi_grid, text="SPI Device ID:").grid(row=0, column=0, sticky="w")
        self.spi_device_spin = ttk.Spinbox(spi_grid, from_=0, to=10, width=10)
        self.spi_device_spin.grid(row=0, column=1, padx=5)
        self.spi_device_spin.set(0)
        
        ttk.Label(spi_grid, text="Chip Select:").grid(row=1, column=0, sticky="w")
        self.spi_cs_spin = ttk.Spinbox(spi_grid, from_=0, to=10, width=10)
        self.spi_cs_spin.grid(row=1, column=1, padx=5)
        self.spi_cs_spin.set(0)
        
        # Parameters
        self.params_frame = ttk.LabelFrame(self.root, text="Chip Parameters")
        self.params_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Control buttons
        button_frame = ttk.Frame(self.root)
        button_frame.pack(fill="x", padx=5, pady=5)
        
        self.init_button = ttk.Button(button_frame, text="Initialize Chip", command=self.initialize_chip)
        self.init_button.pack(side="left", padx=5)
        
        self.update_button = ttk.Button(button_frame, text="Update All Parameters", command=self.update_all_params)
        self.update_button.pack(side="left", padx=5)
        self.update_button.state(['disabled'])
        
        self.read_button = ttk.Button(button_frame, text="Read Status", command=self.read_status)
        self.read_button.pack(side="left", padx=5)
        self.read_button.state(['disabled'])
        
        # Log console
        log_frame = ttk.LabelFrame(self.root, text="Log Console")
        log_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10)
        self.log_text.pack(fill="both", expand=True, padx=5, pady=5)
        
        self.log("GUI initialized. Select a chip to begin.")
    
    def load_chip_configs(self):
        """Load chip configurations from JSON files."""
        config_dir = Path("configs")
        if not config_dir.exists():
            self.log("Configs directory not found. Creating sample config...")
            self.create_sample_config()
        
        for config_file in config_dir.glob("*.json"):
            try:
                chip_config = ChipConfig(str(config_file))
                self.chip_configs[chip_config.chip_name] = chip_config
                self.chip_combo['values'] = list(self.chip_configs.keys())
                self.log(f"Loaded config: {chip_config.chip_name}")
            except Exception as e:
                self.log(f"Error loading config {config_file}: {e}")
    
    def create_sample_config(self):
        """Create a sample ADF4382 configuration file."""
        config_dir = Path("configs")
        config_dir.mkdir(exist_ok=True)
        
        sample_config = {
            "chip_name": "ADF4382",
            "lib_name": "libadf4382.so",
            "init_func": "adf4382_init",
            "remove_func": "adf4382_remove",
            "params": [
                {"name": "ref_freq_hz", "type": "uint64", "default": 125000000, "range": [10000000, 5000000000]},
                {"name": "freq", "type": "uint64", "default": 20000000000, "range": [687500000, 22000000000]},
                {"name": "cp_i", "type": "uint8", "default": 15, "range": [0, 15]},
                {"name": "bleed_word", "type": "uint16", "default": 4903, "range": [0, 8191]},
                {"name": "ref_doubler_en", "type": "bool", "default": True},
                {"name": "ref_div", "type": "uint8", "default": 1, "range": [1, 63]},
                {"name": "ld_count", "type": "uint8", "default": 10, "range": [0, 255]}
            ],
            "setters": [
                {"func": "adf4382_set_rfout", "param": "freq", "type": "uint64"},
                {"func": "adf4382_set_cp_i", "param": "cp_i", "type": "int32"},
                {"func": "adf4382_set_bleed_word", "param": "bleed_word", "type": "int32"},
                {"func": "adf4382_set_phase_adjust", "param": "phase_ps", "type": "uint32"}
            ],
            "spi_defaults": {"device_id": 0, "chip_select": 0, "max_speed_hz": 1500000, "mode": 0}
        }
        
        with open(config_dir / "adf4382.json", 'w') as f:
            json.dump(sample_config, f, indent=2)
        
        self.log("Created sample ADF4382 configuration file.")
    
    def on_chip_selected(self, event):
        """Handle chip selection."""
        chip_name = self.chip_combo.get()
        if not chip_name or chip_name not in self.chip_configs:
            return
        
        self.current_chip = self.chip_configs[chip_name]
        self.create_parameter_widgets()
        self.log(f"Selected chip: {chip_name}")
    
    def create_parameter_widgets(self):
        """Create parameter input widgets based on selected chip."""
        # Clear existing widgets
        for widget in self.params_frame.winfo_children():
            widget.destroy()
        
        self.param_widgets.clear()
        
        if not self.current_chip:
            return
        
        # Create new parameter widgets
        for i, param in enumerate(self.current_chip.params):
            param_name = param['name']
            param_type = param['type']
            default_value = param['default']
            param_range = param.get('range', [])
            
            ttk.Label(self.params_frame, text=f"{param_name}:").grid(row=i, column=0, sticky="w", padx=5, pady=2)
            
            # Create appropriate widget based on parameter type
            if param_type in ['uint64', 'uint32', 'uint16', 'uint8']:
                widget = ttk.Spinbox(self.params_frame, width=20)
                if param_range:
                    widget.configure(from_=param_range[0], to=param_range[1])
                widget.set(default_value)
            elif param_type == 'bool':
                widget = ttk.Checkbutton(self.params_frame)
                if default_value:
                    widget.state(['selected'])
            else:
                widget = ttk.Entry(self.params_frame, width=20)
                widget.insert(0, str(default_value))
            
            widget.grid(row=i, column=1, sticky="w", padx=5, pady=2)
            self.param_widgets[param_name] = widget
    
    def initialize_chip(self):
        """Initialize the selected chip."""
        if not self.current_chip:
            self.log("No chip selected.")
            return
        
        try:
            self.chip_controller = ChipController(self.current_chip)
            spi_device_id = int(self.spi_device_spin.get())
            
            if self.chip_controller.initialize_chip(spi_device_id):
                self.log(f"Successfully initialized {self.current_chip.chip_name}")
                self.init_button.state(['disabled'])
                self.update_button.state(['!disabled'])
                self.read_button.state(['!disabled'])
            else:
                self.log(f"Failed to initialize {self.current_chip.chip_name}")
                
        except Exception as e:
            self.log(f"Error during initialization: {e}")
    
    def update_all_params(self):
        """Update all parameters for the current chip."""
        if not self.chip_controller or not self.chip_controller.initialized:
            self.log("Chip not initialized.")
            return
        
        try:
            for param_name, widget in self.param_widgets.items():
                if isinstance(widget, ttk.Checkbutton):
                    value = widget.instate(['selected'])
                elif isinstance(widget, ttk.Spinbox):
                    value = int(widget.get())
                else:
                    value = widget.get()
                
                if self.chip_controller.set_parameter(param_name, value):
                    self.log(f"Updated {param_name}: {value}")
                else:
                    self.log(f"Failed to update {param_name}")
            
            self.log("Parameter update completed.")
            
        except Exception as e:
            self.log(f"Error updating parameters: {e}")
    
    def read_status(self):
        """Read chip status."""
        if not self.chip_controller or not self.chip_controller.initialized:
            self.log("Chip not initialized.")
            return
        
        self.log("Reading chip status...")
        # This would typically call getter functions from the library
        self.log("Status read completed.")
    
    def log(self, message: str):
        """Add message to log console."""
        self.log_text.insert(tk.END, f"[GUI] {message}\n")
        self.log_text.see(tk.END)
    
    def run(self):
        """Run the GUI."""
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.root.mainloop()
    
    def on_closing(self):
        """Handle application closing."""
        if self.chip_controller:
            self.chip_controller.cleanup()
        self.root.destroy()

def main():
    """Main function."""
    print("ADF4382 and no-OS Chip Control GUI")
    print("==================================")
    
    if USE_PYQT5:
        print("Using PyQt5 GUI")
        app = QApplication(sys.argv)
        app.setApplicationName("ADF4382 Control")
        gui = PyQt5GUI()
        gui.show()
        sys.exit(app.exec_())
    else:
        print("Using Tkinter GUI")
        gui = TkinterGUI()
        gui.run()

if __name__ == "__main__":
    main() 