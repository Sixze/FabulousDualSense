/*
	Device.h is part of DualSenseWindows
	https://github.com/mattdevv/DualSense-Windows

	Contributors of this file:
	11.2021 Matthew Hall
	11.2020 Ludwig Füchsl

	Licensed under the MIT License (To be found in repository root directory)
*/
#pragma once

#include <Windows.h>
#include <DeviceSpecs.h>

// more accurate integer multiplication by a fraction
constexpr int mult_frac(int x, int numer, int denom)
{
	int quot = x / denom;
	int rem = x % denom;
	return quot * numer + (rem * numer) / denom;
}

namespace DS5W {
	/// <summary>
	/// Storage for calibration values used to parse raw motion data
	/// </summary>
	typedef struct _AxisCalibrationData {
		short bias;
		int sens_numer;
		int sens_denom;

		int calibrate(int rawValue)
		{
			return mult_frac(sens_numer, rawValue - bias, sens_denom);
		}
	} AxisCalibrationData;

	typedef struct _DeviceCalibrationData {
		/// <summary>
			/// Values to calibrate controller's accelerometer and gyroscope
			/// </summary>
		AxisCalibrationData accelerometer[3];

		/// <summary>
		/// Values to calibrate controller's gyroscope
		/// </summary>
		AxisCalibrationData gyroscope[3];
	} DeviceCalibrationData;

	/// <summary>
	/// Enum for device connection type
	/// </summary>
	typedef enum class _DeviceConnection : unsigned char {
		/// <summary>
		/// Controler is connected via USB
		/// </summary>
		USB = 0,

		/// <summary>
		/// Controler is connected via bluetooth
		/// </summary>
		BT = 1,
	} DeviceConnection;

	/// <summary>
	/// Struckt for storing device enum info while device discovery
	/// </summary>
	typedef struct _DeviceEnumInfo {
		/// <summary>
		/// Encapsulate data in struct to (at least try) prevent user from modifing the context
		/// </summary>
		struct {
			/// <summary>
			/// Path to the discovered device
			/// </summary>
			wchar_t path[260];

			/// <summary>
			/// Connection type of the discoverd device
			/// </summary>
			DeviceConnection connection;

			/// <summary>
			/// Unique device identifier
			/// 32-bit hash of device interface's path
			/// </summary>
			UINT32 uniqueID;
		} _internal;
	} DeviceEnumInfo;

	/// <summary>
	/// Device context
	/// </summary>
	typedef struct _DeviceContext {
		/// <summary>
		/// Encapsulate data in struct to (at least try) prevent user from modifing the context
		/// </summary>
		struct {
			/// <summary>
			/// Path to the device
			/// </summary>
			wchar_t devicePath[260];

			/// <summary>
			/// Unique device identifier
			/// 32-bit hash of device interface's path
			/// </summary>
			UINT32 uniqueID;

			/// <summary>
			/// Handle to the open device
			/// </summary>
			HANDLE deviceHandle;

			/// <summary>
			/// Synchronization struct for async input
			/// </summary>
			OVERLAPPED olRead;

			/// <summary>
			/// Synchronization struct for async output
			/// </summary>
			OVERLAPPED olWrite;

			/// <summary>
			/// Connection of the device
			/// </summary>
			DeviceConnection connectionType;

			/// <summary>
			/// Collection of values required to parse controller's motion data
			/// </summary>
			DeviceCalibrationData calibrationData;

			/// <summary>
			/// Time when last input report was received, measured in 0.33 microseconds
			/// </summary>
			unsigned int timestamp;

			/// <summary>
			/// Current state of connection
			/// </summary>
			bool connected;

			/// <summary>
			/// HID Input buffer
			/// </summary>
			unsigned char hidInBuffer[DS_MAX_INPUT_REPORT_SIZE];

			/// <summary>
			/// HID Output buffer
			/// </summary>
			unsigned char hidOutBuffer[DS_MAX_OUTPUT_REPORT_SIZE];
		}_internal;
	} DeviceContext;
}