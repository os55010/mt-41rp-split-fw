/*
 * mt-41rp split の状態 LED
 *
 * - バッテリーに充電中: charging-led（緑）を点灯
 *   XIAO nRF52840 の充電 IC（BQ25101）の CHG 出力を読む。USB をつないでいても、
 *   バッテリーがない・満充電のときは消える。
 * - ペアリング中: pairing-led（青）を点滅
 *   右手（セントラル）: 今の BT プロファイルにホストが登録されていない（ペアリング待ち）
 *   左手（ペリフェラル）: 右手とまだボンディングされておらず、つながっていない
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) || !IS_ENABLED(CONFIG_ZMK_SPLIT)
#define STATUS_LEDS_HOST_SIDE 1
#include <zmk/ble.h>
#else
#define STATUS_LEDS_HOST_SIDE 0
#include <zmk/split/bluetooth/peripheral.h>
#endif

#define NODE DT_INST(0, zmk_mt41rp_status_leds)

static const struct gpio_dt_spec chg = GPIO_DT_SPEC_GET(NODE, chg_gpios);
static const struct gpio_dt_spec charging_led =
    GPIO_DT_SPEC_GET(DT_PHANDLE(NODE, charging_led), gpios);
static const struct gpio_dt_spec pairing_led =
    GPIO_DT_SPEC_GET(DT_PHANDLE(NODE, pairing_led), gpios);

static bool is_pairing(void) {
#if STATUS_LEDS_HOST_SIDE
    return zmk_ble_active_profile_is_open();
#else
    return !zmk_split_bt_peripheral_is_bonded() && !zmk_split_bt_peripheral_is_connected();
#endif
}

static void status_leds_update(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(status_leds_work, status_leds_update);
static bool blink_on;

static void status_leds_update(struct k_work *work) {
    gpio_pin_set_dt(&charging_led, gpio_pin_get_dt(&chg) > 0);

    if (is_pairing()) {
        blink_on = !blink_on;
        gpio_pin_set_dt(&pairing_led, blink_on);
        k_work_schedule(&status_leds_work, K_MSEC(CONFIG_MT41RP_STATUS_LEDS_BLINK_MS));
    } else {
        blink_on = false;
        gpio_pin_set_dt(&pairing_led, 0);
        k_work_schedule(&status_leds_work, K_MSEC(CONFIG_MT41RP_STATUS_LEDS_POLL_MS));
    }
}

static int status_leds_init(void) {
    if (!gpio_is_ready_dt(&chg) || !gpio_is_ready_dt(&charging_led) ||
        !gpio_is_ready_dt(&pairing_led)) {
        return -ENODEV;
    }

    int err = gpio_pin_configure_dt(&chg, GPIO_INPUT);
    if (!err) {
        err = gpio_pin_configure_dt(&charging_led, GPIO_OUTPUT_INACTIVE);
    }
    if (!err) {
        err = gpio_pin_configure_dt(&pairing_led, GPIO_OUTPUT_INACTIVE);
    }
    if (err) {
        return err;
    }

    k_work_schedule(&status_leds_work, K_NO_WAIT);
    return 0;
}

SYS_INIT(status_leds_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
