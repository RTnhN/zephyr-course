#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#if defined(CONFIG_LED_SUBSYSTEM)

#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

#if defined(CONFIG_BLINK_SLEEP_250MS)
static constexpr int blink_interval_ms = 250;
#elif defined(CONFIG_BLINK_SLEEP_1000MS)
static constexpr int blink_interval_ms = 1000;
#else
static constexpr int blink_interval_ms = 2000;
#endif

static constexpr int pwm_period_ms = 10;

// GPIO-based PWM: a logical 1 turns on the LED regardless of GPIO polarity.
static int show_brightness(int brightness, int duration_ms)
{
    for (int elapsed_ms = 0; elapsed_ms < duration_ms; elapsed_ms += pwm_period_ms) {
        const int period_ms = (duration_ms - elapsed_ms < pwm_period_ms)
                                  ? duration_ms - elapsed_ms : pwm_period_ms;
        const int on_us = period_ms * 1000 * brightness / 100;
        const int off_us = period_ms * 1000 - on_us;

        if (on_us > 0) {
            const int ret = gpio_pin_set_dt(&led, 1);
            if (ret < 0) return ret;
            k_usleep(on_us);
        }
        if (off_us > 0) {
            const int ret = gpio_pin_set_dt(&led, 0);
            if (ret < 0) return ret;
            k_usleep(off_us);
        }
    }
    return 0;
}

static int show_phase(int start_brightness, int end_brightness)
{
    const int fade_ms = (CONFIG_LED_FADE_DURATION < blink_interval_ms)
                            ? CONFIG_LED_FADE_DURATION : blink_interval_ms;

    for (int elapsed_ms = 0; elapsed_ms < blink_interval_ms; elapsed_ms += pwm_period_ms) {
        const int period_ms = (blink_interval_ms - elapsed_ms < pwm_period_ms)
                                  ? blink_interval_ms - elapsed_ms : pwm_period_ms;
        const int progress_ms = (elapsed_ms + period_ms < fade_ms)
                                    ? elapsed_ms + period_ms : fade_ms;
        const int brightness = fade_ms == 0 ? end_brightness
                             : start_brightness + (end_brightness - start_brightness)
                                   * progress_ms / fade_ms;
        const int ret = show_brightness(brightness, period_ms);
        if (ret < 0) return ret;
    }
    return 0;
}

#endif

int main(void)
{
#if defined(CONFIG_LED_SUBSYSTEM)
    if (!gpio_is_ready_dt(&led)) return 0;
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0) return 0;

    while (1) {
        LOG_INF("LED state: ON");
        if (show_phase(0, CONFIG_LED_BRIGHTNESS) < 0) return 0;

        LOG_INF("LED state: OFF");
        if (show_phase(CONFIG_LED_BRIGHTNESS, 0) < 0) return 0;
    }
#endif
    return 0;
}
