/**
 * @file sscan.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdbool.h>

#include "dmx_devices.h"

inline static bool is_digit(char c) {
	return (c >= (char)'0') && (c <= (char)'9');
}

inline static bool is_xdigit(char c) {
	return (is_digit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

int sscan_uint8_t(const char *buf, const char *name, uint8_t *value) {
	int i;
	int k = 0;

	for (i = 0; (name[i] != (char) 0) && (buf[i] != (char) '=')	&& (buf[i] != (char) 0); i++) {
		if (name[i] != buf[i]) {
			return 0;
		}
	}

	if (name[i] != (char)0) {
		return 0;
	}

	i++;

	while ((buf[i] != (char) 0) && (buf[i] != '\n')) {
		if (!is_digit(buf[i])) {
			return 1;
		}
		k = k * 10 + (int)buf[i] - (int)'0';
		i++;
	}

	if (k > (int)((uint8_t)~0)) {
		return 1;
	}

	*value = (uint8_t)k;

	return 2;
}

int sscan_char_p(const char *buf, const char *name, char *value, uint8_t *len) {
	int i;
	int k;

	for (i = 0; (name[i] != (char) 0) && (buf[i] != (char) '=')	&& (buf[i] != (char) 0); i++) {
		if (name[i] != buf[i]) {
			return 0;
		}
	}

	if (name[i] != (char)0) {
		return 0;
	}

	i++;

	if ((buf[i] == (char) 0) || (buf[i] == '\n')) {
		return 1;
	}

	k = 0;

	while ((buf[i] != (char) 0) && (buf[i] != (char) '\n') && (k < (int)*len)) {
		value[k++] = buf[i++];
	}

	*len = (uint8_t)k;

	return 2;
}

int sscan_spi(const char *buf, char *spi, char *name, uint8_t *len, uint8_t *address, uint16_t *dmx) {
	int i;
	int j;
	int k;
	char c;
	uint16_t uint16;
	char tmp[3];
	const char SPI[] = "SPI";
	uint8_t nibble_high;
	uint8_t nibble_low;

	for (i = 0; (buf[i] != (char) 0) && (buf[i] != (char) ',') ; i++) {
		if ((i < 3) && (buf[i] != SPI[i]))  {
			return DMX_DEVICE_CONFIG_INVALID_PROTOCOL;
		}
	}

	if (i > 4) {
		return DMX_DEVICE_CONFIG_INVALID_PROTOCOL;
	}

	c = buf[3];

	if (!is_digit(c) && (buf[4] != (char) ',')) {
		return DMX_DEVICE_CONFIG_INVALID_PROTOCOL;
	}

	*spi = c -  (char) '0';

	k = 0;
	i++;

	while ((buf[i] != (char) 0) && (k < (int)*len) && (buf[i] != (char) ',')) {
			name[k++] = buf[i++];
	}

	name[k] = (char) 0;

	*len = k;

	if (buf[i] != (char) ',') {
		return DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS;
	}

	k = 0;
	i++;

	while ((buf[i] != (char) 0) && (buf[i] != (char) ',') && (k < 2)) {
		if (!is_xdigit(buf[i])) {
			return DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS;
		}
		tmp[k++] = buf[i++];
	}

	if ((k == 0) || (buf[i] != (char) ',')) {
		return DMX_DEVICE_CONFIG_INVALID_SLAVE_ADDRESS;
	}

	if (k == 2) {
		nibble_low = tmp[1] > '9' ? (tmp[1] | 0x20) - 'a' + 10 : tmp[1] - '0';
		nibble_high = (tmp[0] > '9' ? (tmp[0] | 0x20) - 'a' + 10 : tmp[0] - '0') << 4;
		*address = nibble_high | nibble_low;
	} else {
		nibble_low = tmp[0] > '9' ? (tmp[0] | 0x20) - 'a' + 10 : tmp[0] - '0';
		*address = nibble_low;
	}

	k = 0;
	i++;
	uint16 = 0;

	if (!is_digit(buf[i])) {
		return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
	}

	while ((buf[i] != (char) 0) && (buf[i] != (char) '\n') && (k < 3) && (buf[i] != (char) ' ')) {
		tmp[k++] = buf[i++];
	}

	if (k == 0)  {
		return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
	}

	j = 0;

	while (k--) {
		if (!is_digit(tmp[j])) {
			return DMX_DEVICE_CONFIG_INVALID_START_ADDRESS;
		}
		uint16 = uint16 * 10 + tmp[j++] - (char) '0';
	}

	*dmx = uint16;

	return 4;
}