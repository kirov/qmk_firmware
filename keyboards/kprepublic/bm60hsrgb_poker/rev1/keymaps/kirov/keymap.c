/* Copyright 2020 Evgeniy Kirov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#define BACKLIGHT_TIMEOUT 15  // in minutes
static uint16_t idle_timer = 0;
static uint8_t halfmin_counter = 0;
static uint8_t old_rgb_value = -1;
static bool rgb_on = true;

#define KC_SPACEFN LT(L_FN, KC_SPC)

enum layers {
    L_DEFAULT,
    L_FN,
	L_RGB,
    L_HYPER,
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	[L_DEFAULT] = LAYOUT_60_ansi(
		KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
		KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
		LT(L_HYPER, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT,
		LSFT_T(KC_F13), KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, RSFT_T(KC_F13),
		KC_LCTL, KC_LALT, KC_LGUI, KC_SPACEFN, KC_RGUI, KC_RALT, MO(1), KC_RCTL
	),

	[L_FN] = LAYOUT_60_ansi(
		KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_DEL,
		KC_TRNS, LALT(KC_LEFT), KC_UP, LALT(KC_RGHT), LGUI(KC_LEFT), KC_PGUP, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RCS(KC_LBRC), RCS(KC_RBRC), KC_NO,
		KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, LGUI(KC_RGHT), KC_PGDN, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_NO, KC_NO, TO(2),
		KC_TRNS, KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU, RGB_VAD, RGB_VAI, KC_MPRV, KC_MNXT, KC_MPLY, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
	),

	[L_RGB] = LAYOUT_60_ansi(
		KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RESET,
		KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_VAI, KC_NO,
		TO(0), RGB_HUD, RGB_HUI, RGB_SAD, RGB_SAI, RGB_SPD, RGB_SPI, RGB_RMOD, RGB_MOD, KC_NO, RGB_TOG, RGB_VAD, TO(0),
		KC_NO, RGB_M_P, RGB_M_B, RGB_M_R, RGB_M_R, RGB_M_SW, RGB_M_SN, RGB_M_K, RGB_M_X, RGB_M_G, RGB_M_T, KC_NO,
		KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, TO(0), KC_NO
	),

	[L_HYPER] = LAYOUT_60_ansi(
		HYPR(KC_GRV), HYPR(KC_1), HYPR(KC_2), HYPR(KC_3), HYPR(KC_4), HYPR(KC_5), HYPR(KC_6), HYPR(KC_7), HYPR(KC_8), HYPR(KC_9), HYPR(KC_0), HYPR(KC_MINS), HYPR(KC_EQL), HYPR(KC_BSPC),
		KC_TRNS, HYPR(KC_Q), HYPR(KC_W), HYPR(KC_E), HYPR(KC_R), HYPR(KC_T), HYPR(KC_Y), HYPR(KC_U), HYPR(KC_I), HYPR(KC_O), HYPR(KC_P), HYPR(KC_LBRC), HYPR(KC_RBRC), HYPR(KC_BSLS),
		KC_TRNS, HYPR(KC_A), HYPR(KC_S), HYPR(KC_D), HYPR(KC_F), HYPR(KC_G), HYPR(KC_H), HYPR(KC_J), HYPR(KC_K), HYPR(KC_L), HYPR(KC_SCLN), HYPR(KC_QUOT), HYPR(KC_ENT),
		KC_TRNS, HYPR(KC_Z), HYPR(KC_X), HYPR(KC_C), HYPR(KC_V), HYPR(KC_B), HYPR(KC_N), HYPR(KC_M), HYPR(KC_COMM), HYPR(KC_DOT), HYPR(KC_SLSH), KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
	)
};

void matrix_scan_user(void) {
    // idle_timer needs to be set one time
    if (idle_timer == 0) idle_timer = timer_read();

    if (rgb_on && (timer_elapsed(idle_timer) > 30000)) {
        halfmin_counter++;
        idle_timer = timer_read();
    }

    if (rgb_on && (halfmin_counter >= BACKLIGHT_TIMEOUT * 2)) {
        rgblight_disable_noeeprom();
        rgb_on = false;
        halfmin_counter = 0;
    }
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (rgb_on == false || old_rgb_value == -1) {
            if (old_rgb_value == -1) old_rgb_value = 1;
            old_rgb_value = 1;
            rgblight_enable_noeeprom();
            rgb_on = true;
        }
        idle_timer = timer_read();
        halfmin_counter = 0;
    }

    return true;
}

bool get_retro_tapping(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_SPACEFN:
            return true;
        default:
            return false;
    }
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_SPACEFN:
            return false;
        default:
            return true;
    }
}
