if(CONFIG_MT41RP_STATUS_LEDS)
  # ZMK のヘッダ（zmk/ble.h など）を使うので app ターゲットに追加する
  target_sources(app PRIVATE src/status_leds.c)
endif()
