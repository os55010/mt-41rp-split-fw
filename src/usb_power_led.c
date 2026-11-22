/*
 * USB（VBUS）から給電されている間だけ LED を点灯する。
 *
 * ペリフェラル（左手）では ZMK の USB イベントが使えないので、
 * nRF52840 の POWER ペリフェラルの VBUS 検出を一定間隔で確認する。
 * 点灯する LED は devicetree の alias `usb-power-led` で選ぶ。
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <hal/nrf_power.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(usb_power_led), gpios);

static void usb_power_led_update(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(usb_power_led_work, usb_power_led_update);

static void usb_power_led_update(struct k_work *work) {
    bool vbus = nrf_power_usbregstatus_vbusdet_get(NRF_POWER);

    gpio_pin_set_dt(&led, vbus);
    k_work_schedule(&usb_power_led_work, K_MSEC(CONFIG_MT41RP_USB_POWER_LED_POLL_MS));
}

static int usb_power_led_init(void) {
    if (!gpio_is_ready_dt(&led)) {
        return -ENODEV;
    }

    int err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (err) {
        return err;
    }

    k_work_schedule(&usb_power_led_work, K_NO_WAIT);
    return 0;
}

SYS_INIT(usb_power_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
