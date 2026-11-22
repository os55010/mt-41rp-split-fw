if(CONFIG_MT41RP_USB_POWER_LED)
  zephyr_library()
  zephyr_library_sources(src/usb_power_led.c)
endif()
