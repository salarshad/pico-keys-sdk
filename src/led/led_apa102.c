/*
 * This file is part of the Pico Keys SDK distribution (https://github.com/polhenarejos/pico-keys-sdk).
 * Copyright (c) 2022 Pol Henarejos.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pico_keys.h"

#ifdef ESP_PLATFORM

#include "driver/gpio.h"

#define APA102_DEFAULT_DATA_GPIO GPIO_NUM_40
#define APA102_DEFAULT_CLK_GPIO  GPIO_NUM_39

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} apa102_color_t;

static const apa102_color_t color_map[] = {
    {0x00, 0x00, 0x00},
    {0xFF, 0x00, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0x00, 0xFF},
    {0xFF, 0xFF, 0x00},
    {0xFF, 0x00, 0xFF},
    {0x00, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF},
};

static gpio_num_t apa102_data_gpio = APA102_DEFAULT_DATA_GPIO;
static gpio_num_t apa102_clk_gpio = APA102_DEFAULT_CLK_GPIO;

static void apa102_write_bit(bool bit_value) {
    gpio_set_level(apa102_data_gpio, bit_value ? 1 : 0);
    gpio_set_level(apa102_clk_gpio, 1);
    gpio_set_level(apa102_clk_gpio, 0);
}

static void apa102_write_byte(uint8_t value) {
    for (int bit = 7; bit >= 0; bit--) {
        apa102_write_bit((value >> bit) & 0x1);
    }
}

static void apa102_write_frame(uint8_t global_brightness, uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < 4; i++) {
        apa102_write_byte(0x00);
    }

    apa102_write_byte(0xE0 | (global_brightness & 0x1F));
    apa102_write_byte(blue);
    apa102_write_byte(green);
    apa102_write_byte(red);

    for (int i = 0; i < 4; i++) {
        apa102_write_byte(0xFF);
    }
}

void led_driver_init_apa102() {
    if (phy_data.led_gpio_present) {
        apa102_data_gpio = (gpio_num_t)phy_data.led_gpio;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << apa102_data_gpio) | (1ULL << apa102_clk_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&io_conf);
    gpio_set_level(apa102_clk_gpio, 0);
    gpio_set_level(apa102_data_gpio, 0);
    apa102_write_frame(0, 0, 0, 0);
}

void led_driver_color_apa102(uint8_t color, uint32_t led_brightness, float progress) {
    if (!(phy_data.opts & PHY_OPT_DIMM)) {
        progress = progress >= 0.5f ? 1.0f : 0.0f;
    }

#ifdef PICO_BOARD_TDONGLE_S3
    // Map the standard "wait for touch" yellow state to blue on the T-Dongle S3.
    if (color == LED_COLOR_YELLOW) {
        color = LED_COLOR_BLUE;
    }
#endif

    uint32_t led_phy_brightness = phy_data.led_brightness_present ? phy_data.led_brightness : MAX_BTNESS;
    float brightness = ((float)led_brightness / MAX_BTNESS) * ((float)led_phy_brightness / MAX_BTNESS) * progress;
    const apa102_color_t rgb = color_map[color];

    uint8_t red = (uint8_t)(rgb.red * brightness);
    uint8_t green = (uint8_t)(rgb.green * brightness);
    uint8_t blue = (uint8_t)(rgb.blue * brightness);
    uint8_t global_brightness = (red || green || blue) ? (uint8_t)(1 + (30.0f * brightness)) : 0;

    apa102_write_frame(global_brightness, red, green, blue);
}

led_driver_t led_driver_apa102 = {
    .init = led_driver_init_apa102,
    .set_color = led_driver_color_apa102,
};

#endif
