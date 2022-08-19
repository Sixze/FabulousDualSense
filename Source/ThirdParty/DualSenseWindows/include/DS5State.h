/*
	DS5State.h is part of DualSenseWindows
	https://github.com/mattdevv/DualSense-Windows

	Contributors of this file:
	11.2021 mattdevv
	11.2020 Ludwig Füchsl

	Licensed under the MIT License (To be found in repository root directory)
*/
#pragma once

// DPAD buttons
#define DS5W_ISTATE_BTN_DPAD_LEFT 0x01
#define DS5W_ISTATE_BTN_DPAD_DOWN 0x02
#define DS5W_ISTATE_BTN_DPAD_RIGHT 0x04
#define DS5W_ISTATE_BTN_DPAD_UP 0x08

// Face buttons
#define DS5W_ISTATE_BTN_SQUARE 0x10
#define DS5W_ISTATE_BTN_CROSS 0x20
#define DS5W_ISTATE_BTN_CIRCLE 0x40
#define DS5W_ISTATE_BTN_TRIANGLE 0x80

// Shoulder buttons
#define DS5W_ISTATE_BTN_BUMPER_LEFT 0x0100
#define DS5W_ISTATE_BTN_BUMPER_RIGHT 0x0200
#define DS5W_ISTATE_BTN_TRIGGER_LEFT 0x0400
#define DS5W_ISTATE_BTN_TRIGGER_RIGHT 0x0800

// Menu buttons
#define DS5W_ISTATE_BTN_SELECT 0x1000
#define DS5W_ISTATE_BTN_MENU 0x2000

// Stick buttons
#define DS5W_ISTATE_BTN_STICK_LEFT 0x4000
#define DS5W_ISTATE_BTN_STICK_RIGHT 0x8000

// Extra buttons
#define DS5W_ISTATE_BTN_PLAYSTATION_LOGO 0x010000
#define DS5W_ISTATE_BTN_PAD_BUTTON 0x020000
#define DS5W_ISTATE_BTN_MIC_BUTTON 0x040000

#define DS5W_OSTATE_PLAYER_LED_LEFT 0x01
#define DS5W_OSTATE_PLAYER_LED_MIDDLE_LEFT 0x02
#define DS5W_OSTATE_PLAYER_LED_MIDDLE 0x04
#define DS5W_OSTATE_PLAYER_LED_MIDDLE_RIGHT 0x08
#define DS5W_OSTATE_PLAYER_LED_RIGHT 0x10

namespace DS5W {

	/// <summary>
	/// Analog stick
	/// </summary>
	typedef struct _AnalogStick {
		/// <summary>
		/// X Position of stick (0 = Center)
		/// </summary>
		char x;

		/// <summary>
		/// Y Posistion of stick (0 = Center)
		/// </summary>
		char y;
	} AnalogStick;

	/// <summary>
	/// 3 Component vector
	/// </summary>
	typedef struct _Vec3 {
		int x;
		int y;
		int z;
	} Vector3, Vec3;

	/// <summary>
	/// RGB Color
	/// </summary>
	typedef struct _Color {
		unsigned char r;
		unsigned char g;
		unsigned char b;
	} Color;

	/// <summary>
	/// Touchpad state
	/// </summary>
	typedef struct _Touch {
		/// <summary>
		/// X positon of finger (0 - 1920)
		/// </summary>
		unsigned int x;

		/// <summary>
		/// Y position of finger (0 - 1080)
		/// </summary>
		unsigned int y;

		/// <summary>
		/// Touch is down
		/// </summary>
		bool down;

		/// <summary>
		/// 7-bit ID for touch
		/// </summary>
		unsigned char id;
	} Touch;

	typedef struct _Battery {
		/// <summary>
		/// Charching state of the battery
		/// </summary>
		bool charging;

		/// <summary>
		/// Indicates that the battery is fully charged
		/// </summary>
		bool fullyCharged;

		/// <summary>
		/// Battery charge level 0x0 to 
		/// </summary>
		unsigned char level;
	} Battery;

	/// <summary>
	/// State of the mic led
	/// </summary>
	typedef enum class _MicLed : unsigned char{
		/// <summary>
		/// Lef is off
		/// </summary>
		OFF = 0x00,

		/// <summary>
		/// Led is on
		/// </summary>
		ON = 0x01,

		/// <summary>
		/// Led is pulsing
		/// </summary>
		PULSE = 0x02,
	} MicLed;

	/// <summary>
	/// Type of trigger effect
	/// </summary>
	typedef enum class _TriggerEffectType : unsigned char {
		/// <summary>
		/// Disable all effects (after trigger is released)
		/// </summary>
		NoResitance = 0x00,

		/// <summary>
		/// Continuous Resitance is applied
		/// </summary>
		ContinuousResitance = 0x01,

		/// <summary>
		/// Seciton resistance is applied
		/// </summary>
		SectionResitance = 0x02,

		/// <summary>
		/// Disable all effects and release any active tension
		/// </summary>
		ReleaseAll = 0x05,

		/// <summary>
		/// Extended trigger effect
		/// </summary>
		EffectEx = 0x26,

		/// <summary>
		/// Calibrate triggers
		/// </summary>
		Calibrate = 0xFC,
	} TriggerEffectType;

	/// <summary>
	/// Trigger effect
	/// </summary>
	typedef struct _TriggerEffect {
		/// <summary>
		/// Trigger effect type
		/// </summary>
		TriggerEffectType effectType;

		/// <summary>
		/// Union for effect parameters
		/// </summary>
		union {
			/// <summary>
			/// Union one raw data
			/// </summary>
			unsigned char _u1_raw[10];

			/// <summary>
			/// For type == ContinuousResitance
			/// </summary>
			struct {
				/// <summary>
				/// Start position of resistance
				/// </summary>
				unsigned char startPosition;

				/// <summary>
				/// Force of resistance
				/// </summary>
				unsigned char force;

				/// <summary>
				/// PAD / UNUSED
				/// </summary>
				unsigned char _pad[8];
			} Continuous;

			/// <summary>
			/// For type == SectionResitance
			/// </summary>
			struct {
				/// <summary>
				/// Start position of resistance
				/// </summary>
				unsigned char startPosition;

				/// <summary>
				/// End position of resistance (>= start)
				/// </summary>
				unsigned char endPosition;
				
				/// <summary>
				/// PAD / UNUSED
				/// </summary>
				unsigned char _pad[8];
			} Section;

			/// <summary>
			/// For type == EffectEx
			/// </summary>
			struct {
				/// <summary>
				/// Position at witch the effect starts
				/// </summary>
				unsigned char startPosition;

				/// <summary>
				/// Wher the effect should keep playing when trigger goes beyond 255
				/// </summary>
				bool keepEffect;

				/// <summary>
				/// Force applied when trigger >= (255 / 2)
				/// </summary>
				unsigned char beginForce;

				/// <summary>
				/// Force applied when trigger <= (255 / 2)
				/// </summary>
				unsigned char middleForce;

				/// <summary>
				/// Force applied when trigger is beyond 255
				/// </summary>
				unsigned char endForce;

				/// <summary>
				/// Vibration frequency of the trigger
				/// </summary>
				unsigned char frequency;

				/// <summary>
				/// PAD / UNUSED
				/// </summary>
				unsigned char _pad[4];
			} EffectEx;
		};
	} TriggerEffect;

	/// <summary>
	/// Led brightness
	/// </summary>
	typedef enum _LedBrightness : unsigned char {
		/// <summary>
		/// Low led brightness
		/// </summary>
		LOW = 0x02,

		/// <summary>
		/// Medium led brightness
		/// </summary>
		MEDIUM = 0x01,

		/// <summary>
		/// High led brightness
		/// </summary>
		HIGH = 0x00,
	} LedBrightness;

	/// <summary>
	/// Player leds values
	/// </summary>
	typedef struct _PlayerLeds {
		/// <summary>
		/// Player indication leds bitflag (You may used them for other features) DS5W_OSTATE_PLAYER_LED_???
		/// </summary>
		unsigned char bitmask;

		/// <summary>
		/// Indicates weather the player leds should fade in
		/// </summary>
		bool playerLedFade;

		/// <summary>
		/// Brightness of the player leds
		/// </summary>
		LedBrightness brightness;
	} PlayerLeds;

	/// <summary>
	/// Flags used by DualSense controller to identify changes output report will perform
	/// </summary>
	typedef enum class _OutputFlags : unsigned short
	{
		SetMainMotorsA =		1 << 0,		// Allow changing controller haptics. Also requires SetMainMotorsB flag
		SetMainMotorsB =		1 << 1,		// Allow changing controller haptics. Also requires SetMainMotorsA flag
		SetTriggerMotorsA =		1 << 2,		// Allow changing trigger haptics. Also requires SetTriggerMotorsB flag
		SetTriggerMotorsB =		1 << 3,		// Allow changing trigger haptics. Also requires SetTriggerMotorsA flag
		SetAudioVolume =		1 << 4,		// Enable modification of audio volume
		EnableAudio =			1 << 5,		// Enable internal speaker (even while headset is connected)
		SetMicrophoneVolume =	1 << 6,		// Enable modification of microphone volume
		EnableMicrophone =		1 << 7,		// Enable internal mic (even while headset is connected)

		SetMicrophoneLED =		1 << 8,		// Allow changing microphone LED state
		SetAudioMicMute =		1 << 9,		// Set microphone to mute when flag is on?
		SetColorLED =			1 << 10,	// Allow changing lightbar RGB value
		DisableAllLED =			1 << 11,	// Turn off all lights while flag is set
		SetPlayerLED =			1 << 12,	// Allow changing which player LEDs are enabled
		UnknownFlag1 =			1 << 13,	// ?
		SetMotorStrength =		1 << 14,	// Allow changing rumble strength
		UnknownFlag2 =			1 << 15,	// ?

	} OutputFlags;

	/// <summary>
	/// Default output flags to allow changing all settings other than audio/microphone
	/// </summary>
	const unsigned short DefaultOutputFlags =
		(unsigned short)OutputFlags::SetMainMotorsA |
		(unsigned short)OutputFlags::SetMainMotorsB |
		(unsigned short)OutputFlags::SetTriggerMotorsA |
		(unsigned short)OutputFlags::SetTriggerMotorsB |
		(unsigned short)OutputFlags::SetMicrophoneLED |
		(unsigned short)OutputFlags::SetAudioMicMute |
		(unsigned short)OutputFlags::SetColorLED |
		(unsigned short)OutputFlags::SetPlayerLED |
		(unsigned short)OutputFlags::UnknownFlag1 |
		(unsigned short)OutputFlags::SetMotorStrength |
		(unsigned short)OutputFlags::UnknownFlag2;

	/// <summary>
	/// Input state of the controler
	/// </summary>
	typedef struct _DS5InputState {
		/// <summary>
		/// Position of left stick
		/// </summary>
		AnalogStick leftStick;

		/// <summary>
		/// Posisiton of right stick
		/// </summary>
		AnalogStick rightStick;

		/// <summary>
		/// bitflags of buttons, (face | btnsA | btnsB), final 13 bits are empty
		/// </summary>
		unsigned int buttonMap;

		/// <summary>
		/// Left trigger position
		/// </summary>
		unsigned char leftTrigger;

		/// <summary>
		/// Right trigger position
		/// </summary>
		unsigned char rightTrigger;

		/// <summary>
		/// Accelerometer
		/// </summary>
		Vector3 accelerometer;

		/// <summary>
		/// Gyroscope  (Currently only raw values will be dispayed! Probably needs calibration (Will be done within the lib in the future))
		/// </summary>
		Vector3 gyroscope;

		/// <summary>
		/// First touch point
		/// </summary>
		Touch touchPoint1;

		/// <summary>
		/// Second touch point
		/// </summary>
		Touch touchPoint2;

		/// <summary>
		/// Sensor timestamp in 0.33 microseconds
		/// </summary>
		unsigned int currentTime;

		/// <summary>
		/// Time since last input report. Measured in 0.33 microseconds
		/// </summary>
		unsigned int deltaTime;

		/// <summary>
		/// Battery information
		/// </summary>
		Battery battery;

		/// <summary>
		/// Indicates the connection of headphone
		/// </summary>
		bool headPhoneConnected;

		/// <summary>
		/// EXPERIMAENTAL: Feedback of the left adaptive trigger (only when trigger effect is active)
		/// </summary>
		unsigned char leftTriggerFeedback;

		/// <summary>
		/// EXPERIMAENTAL: Feedback of the right adaptive trigger (only when trigger effect is active)
		/// </summary>
		unsigned char rightTriggerFeedback;
	} DS5InputState;

	typedef struct _DS5OutputState {

		/// <summary>
		/// Left / Hard rumbel motor
		/// </summary>
		unsigned char leftRumble;

		/// <summary>
		/// Right / Soft rumbel motor
		/// </summary>
		unsigned char rightRumble;

		/// <summary>
		/// strength of rumble motors in 12.5% steps
		/// lower nibble (bits 0-3) main rumbles
		/// uppper nibble (bits 4-7) trigger rumbles
		/// </summary>
		unsigned char rumbleStrength;

		/// <summary>
		/// State of the microphone led
		/// </summary>
		MicLed microphoneLed;

		/// <summary>
		/// Diables all leds
		/// </summary>
		bool disableLeds;

		/// <summary>
		/// Player leds
		/// </summary>
		PlayerLeds playerLeds;

		/// <summary>
		/// Color of the lightbar
		/// </summary>
		Color lightbar;

		/// <summary>
		/// Effect of left trigger
		/// </summary>
		TriggerEffect leftTriggerEffect;

		/// <summary>
		/// Effect of right trigger
		/// </summary>
		TriggerEffect rightTriggerEffect;

	} DS5OutputState;
}