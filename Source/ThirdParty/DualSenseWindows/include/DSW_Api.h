/*
	DSW_Api.h is part of DualSenseWindows
	https://github.com/mattdevv/DualSense-Windows

	Contributors of this file:
	12.2021 Matthew Hall
	11.2020 Ludwig Füchsl

	Licensed under the MIT License (To be found in repository root directory)
*/
#pragma once

#if defined(DS5W_BUILD_DLL)
#define DS5W_API __declspec(dllexport)
#elif defined(DS5W_BUILD_LIB)
#define DS5W_API
#elif defined(DS5W_USE_LIB)
#define DS5W_API
#else
#define DS5W_API __declspec(dllimport)
#endif

#define IO_TIMEOUT_MILLISECONDS	100 /* How long to wait for IO requests before assuming device disconnect */
#define ID_HASH_SEED 0xAABB /* Seed used when hashing device path */

#define DS5W_SUCCESS(expr) ((expr) == _DS5W_ReturnValue::OK)
#define DS5W_FAILED(expr) ((expr) != _DS5W_ReturnValue::OK)

#define DS5W_OK							_DS5W_ReturnValue::OK
#define DS5W_E_UNKNOWN					_DS5W_ReturnValue::E_UNKNOWN
#define DS5W_E_INSUFFICIENT_BUFFER		_DS5W_ReturnValue::E_INSUFFICIENT_BUFFER
#define DS5W_E_EXTERNAL_WINAPI			_DS5W_ReturnValue::E_EXTERNAL_WINAPI
#define DS5W_E_STACK_OVERFLOW			_DS5W_ReturnValue::E_STACK_OVERFLOW
#define DS5W_E_INVALID_ARGS				_DS5W_ReturnValue::E_INVALID_ARGS
#define DS5W_E_CURRENTLY_NOT_SUPPORTED	_DS5W_ReturnValue::E_CURRENTLY_NOT_SUPPORTED
#define DS5W_E_DEVICE_REMOVED			_DS5W_ReturnValue::E_DEVICE_REMOVED
#define DS5W_E_BT_COM					_DS5W_ReturnValue::E_BT_COM
#define DS5W_E_IO_TIMEDOUT				_DS5W_ReturnValue::E_IO_TIMEDOUT
#define DS5W_E_IO_FAILED				_DS5W_ReturnValue::E_IO_FAILED
#define DS5W_E_IO_NOT_FOUND				_DS5W_ReturnValue::E_IO_NOT_FOUND
#define DS5W_E_IO_PENDING				_DS5W_ReturnValue::E_IO_PENDING

/// <summary>
/// Enum for return values
/// </summary>
typedef enum class _DS5W_ReturnValue : unsigned int {
	/// <summary>
	/// Operation completed without an error
	/// </summary>
	OK = 0,

	/// <summary>
	/// Operation encountered an unknown error
	/// </summary>
	E_UNKNOWN = 1,

	/// <summary>
	/// The user supplied buffer is to small
	/// </summary>
	E_INSUFFICIENT_BUFFER = 2,

	/// <summary>
	/// External unexpected winapi error (please report as issue if you get this error!)
	/// </summary>
	E_EXTERNAL_WINAPI = 3,

	/// <summary>
	/// Not enought memory on the stack
	/// </summary>
	E_STACK_OVERFLOW = 4,

	/// <summary>
	/// Invalid arguments
	/// </summary>
	E_INVALID_ARGS = 5,

	/// <summary>
	/// This feature is currently not supported
	/// </summary>
	E_CURRENTLY_NOT_SUPPORTED = 6,

	/// <summary>
	/// Device was disconnected
	/// </summary>
	E_DEVICE_REMOVED = 7,

	/// <summary>
	/// Bluetooth communication error
	/// </summary>
	E_BT_COM = 8,

	/// <summary>
	/// IO timeout
	/// </summary>
	E_IO_TIMEDOUT = 9,

	/// <summary>
	/// IO failed
	/// </summary>
	E_IO_FAILED = 10,

	/// <summary>
	/// Overlapped IO request was not found
	/// </summary>
	E_IO_NOT_FOUND = 11,

	/// <summary>
	/// IO did not complete because it is running in the background
	/// </summary>
	E_IO_PENDING = 12

} DS5W_ReturnValue, DS5W_RV;