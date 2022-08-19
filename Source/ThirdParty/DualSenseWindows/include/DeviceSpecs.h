/*
	DS5Specs.h is part of DualSenseWindows
	https://github.com/mattdevv/DualSense-Windows

	Contributors of this file:
	11.2021 Matthew Hall

	Licensed under the MIT License (To be found in repository root directory)
*/
#pragma once

#define SONY_CORP_VENDOR_ID						0x054C
#define DUALSENSE_CONTROLLER_PROD_ID			0x0CE6

#define DS_INPUT_REPORT_USB						0x01
#define DS_INPUT_REPORT_USB_SIZE				64
#define DS_INPUT_REPORT_BT						0x31
#define DS_INPUT_REPORT_BT_SIZE					78

#define DS_OUTPUT_REPORT_USB					0x02
#define DS_OUTPUT_REPORT_USB_SIZE				63
#define DS_OUTPUT_REPORT_BT						0x31
#define DS_OUTPUT_REPORT_BT_SIZE				78

#define DS_FEATURE_REPORT_CALIBRATION			0x05
#define DS_FEATURE_REPORT_CALIBRATION_SIZE		41
#define DS_FEATURE_REPORT_PAIRING_INFO			0x09
#define DS_FEATURE_REPORT_PAIRING_INFO_SIZE		20
#define DS_FEATURE_REPORT_FIRMWARE_INFO			0x20
#define DS_FEATURE_REPORT_FIRMWARE_INFO_SIZE	64

#define DS_MAX_INPUT_REPORT_SIZE				78 /* DS_INPUT_REPORT_BT_SIZE = 78 */
#define DS_MAX_OUTPUT_REPORT_SIZE				78 /* DS_OUTPUT_REPORT_BT_SIZE = 78 */

#define DS_ACC_RES_PER_G						8192
#define DS_ACC_RANGE							(4*DS_ACC_RES_PER_G)
#define DS_GYRO_RES_PER_DEG_S					1024
#define DS_GYRO_RANGE							(2048*DS_GYRO_RES_PER_DEG_S)
#define DS_TOUCHPAD_WIDTH						1920
#define DS_TOUCHPAD_HEIGHT						1080

/*	
	// body of input report
	// starts at byte index 1 (USB), 2 (BT)

	0x00 uint8_t left_stick_x
	0x01 uint8_t left_stick_y
	0x02 uint8_t right_stick_x
	0x03 uint8_t right_stick_y
	0x04 uint8_t left_trigger
	0x05 uint8_t right_trigger

	0x06 uint8_t seq_number; // unknown use

	0x07 uint8_t buttons[4];
	0x0B uint8_t reserved[4];

	0x0F uint16_t gyro[3]; // needs calibration
	0x15 uint16_t accel[3]; // needs calibration
	0x1B uint32_t sensor_timestamp; // in units of 0.333_ microseconds
	0x1F uint8_t reserved2;

	0x20 struct touch_point points[2]; // 4 bytes each

	0x28 uint8_t reserved3[12];
	0x34 uint8_t status;
	0x35 uint8_t reserved4[10];
*/

/*
	// from https://gist.github.com/stealth-alex/10a8e7cc6027b78fa18a7f48a0d3d1e4
	// body of output report
	// starts at byte index 1 (USB), 2 (BT)

	uint8_t feature_flags_1;
	uint8_t feature_flags_2;
	uint8_t motor_strength_right;
	uint8_t motor_strength_left;

	// audio settings requiring volume control flags
	outputReport[5]  = 0xff; // audio volume of connected headphones (maxes out at about 0x7f)
	outputReport[6]  = 0xff; // volume of internal speaker (0-255) (ties in with index 38?!? PS5 appears to only use the range 0x3d-0x64)
	outputReport[7]  = 0xff; // internal microphone volume (not at all linear; 0-255, maxes out at 0x40, all values above are treated like 0x40; 0 is not fully muted, use audio mute flag instead!)
	outputReport[8]  = 0x0c; // audio flags (switching between mic settings causes up to 1s of silence)
		// 0x01 = force use of internal controller mic (if neither 0x01 and 0x02 are set, an attached headset will take precedence)
		// 0x02 = force use of mic attached to the controller (headset)
			// 0x04 = pads left channel of external mic (~1/3rd of the volume? maybe the amount can be controlled?)
		// 0x08 = pads left channel of internal mic (~1/3rd of the volume? maybe the amount can be controlled?)
		// 0x10 = disable attached headphones (only if 0x20 to enable internal speakers is provided as well)
		// 0x20 = enable audio on internal speaker (in addition to a connected headset; headset will use a stereo upmix of the left channel, internal speaker will play the right channel)

	// audio related LEDs requiring according LED toggle flags
	outputReport[9]  = 0x01; // microphone LED (1 = on, 2 = pulsating / neither does affect the mic)

	// audio settings requiring mute toggling flags
	outputReport[10] = 0x00; // 0x10 microphone mute, 0x40 audio mute

	// trigger motors (see below for details)
	outputReport[11..21] right trigger effect (mode byte + up to 10 parameters)
	outputReport[22..32] left trigger effect (mode byte + up to 10 parameters)

	outputReport[33] = 0x00; // value is copied to input report at offset 43
	outputReport[34] = 0x00; // value is copied to input report at offset 44
	outputReport[35] = 0x00; // value is copied to input report at offset 45
	outputReport[36] = 0x00; // value is copied to input report at offset 46

	outputReport[37] = 0x00; // (lower nibble: main motor; upper nibble trigger effects) 0x00 to 0x07 - reduce overall power of the respective motors/effects by 12.5% per increment (this does not affect the regular trigger motor settings, just the automatically repeating trigger effects)
	outputReport[38] = 0x07; // volume of internal speaker (0-7; ties in with index 6 - the PS5 default for this appears to be 4)

	// LED section (requires LED setting flag)
	outputReport[39] = 2; // flags 0x01 = set player led brightness (value in index 43), 0x02 = uninterruptable blue LED pulse (action in index 42)
	outputReport[42] = 2; // pulse option
			1 = slowly (2s) fade to blue (scheduled to when the regular LED settings are active)
			2 = slowly (2s) fade out (scheduled after fade-in completion) with eventual switch back to configured LED color; only a fade-out can cancel the pulse (neither index 2, 0x08, nor turning this off will cancel it!)
	outputReport[43] = 0x02; // 0x00 = high brightness, 0x01 = medium brightness, 0x02 = low brightness (requires flag from index 39)
	outputReport[44] = 0x04; // 5 white player indicator LEDs below the touchpad (bitmask 00-1f from left to right with 0x04 being the center LED; bit 0x20 to set the brightness immediately and not fade in;)
	outputReport[45] = 0x1f; // Red value of light bars left and right from touchpad
	outputReport[46] = 0xff; // Green value of light bars left and right from touchpad
	outputReport[47] = 0x1f; // Blue value of light bars left and right from touchpad
*/