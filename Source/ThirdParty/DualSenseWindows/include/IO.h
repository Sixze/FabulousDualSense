/*
	DualSenseWindows API
	https://github.com/mattdevv/DualSense-Windows

	Licensed under the MIT License (To be found in repository root directory)
*/
#pragma once

#include <DSW_Api.h>
#include <Device.h>
#include <DS5State.h>
#include <DeviceSpecs.h>

namespace DS5W {
	/// <summary>
	/// Enumerate all ds5 deviced connected to the computer
	/// </summary>
	/// <param name="ptrBuffer">Pointer to begin of array of DeviceEnumInfo objects / DeviceEnumInfo pointers</param>
	/// <param name="inArrLength">Length of imput array</param>
	/// <param name="requiredLength"> pointer to uint witch recives the required total length</param>
	/// <param name="pointerToArray"> DeviceEnumInfo pointer is the pointer to an array of DeviceEnumInfo objects. false: DeviceEnumInfo pointer is a pointer to DeviceEnumInfo pointers to DeviceEnumInfo objects</param>
	/// <returns>DS5W Return value</returns>
	extern "C" DS5W_API DS5W_ReturnValue enumDevices(void* ptrBuffer, unsigned int inArrLength, unsigned int* requiredLength, bool pointerToArray = true);

	/// <summary>
	/// Enumerate all ds5 deviced that are not in the list of known devices
	/// Devices are 'known' if their unique ID is in the array passed to this function
	/// </summary>
	/// <param name="ptrBuffer">Pointer to begin of array of unused DeviceEnumInfo objects / DeviceEnumInfo pointers</param>
	/// <param name="inArrLength">Length of input array</param>
	/// <param name="knownDeviceIDs"> pointer to array of known device IDs</param>
	/// <param name="numKnownDevices"> length of knownDeviceIDs array</param>
	/// <param name="requiredLength"> pointer to uint witch recives the required total length</param>
	/// <param name="pointerToArray"> DeviceEnumInfo pointer is the pointer to an array of DeviceEnumInfo objects. false: DeviceEnumInfo pointer is a pointer to DeviceEnumInfo pointers to DeviceEnumInfo objects</param>
	/// <returns>DS5W Return value</returns>
	extern "C" DS5W_API DS5W_ReturnValue enumUnknownDevices(void* ptrBuffer, unsigned int inArrLength, unsigned int* knownDeviceIDs, unsigned int numKnownDevices, unsigned int* requiredLength, bool pointerToArray = true);

	/// <summary>
	/// Initializes a DeviceContext from its enum infos
	/// </summary>
	/// <param name="ptrEnumInfo">Pointer to enum object to create device from</param>
	/// <param name="ptrContext">Pointer to context to create to</param>
	/// <returns>If creation was successfull</returns>
	extern "C" DS5W_API DS5W_ReturnValue initDeviceContext(DS5W::DeviceEnumInfo* ptrEnumInfo, DS5W::DeviceContext* ptrContext);

	/// <summary>
	/// Stop device functions and free all links in Windows
	/// This context will not be able to be reconnected
	/// </summary>
	/// <param name="ptrContext">Pointer to context</param>
	extern "C" DS5W_API void freeDeviceContext(DS5W::DeviceContext* ptrContext);

	/// <summary>
	/// Stop device functions and disconnect device from windows
	/// This context is able to be reconnected
	/// </summary>
	/// <param name="ptrContext">Context to shutdown</param>
	extern "C" DS5W_API void shutdownDevice(DS5W::DeviceContext * ptrContext);

	/// <summary>
	/// Try to reconnect a disconnected device
	/// </summary>
	/// <param name="ptrContext">Context to reconnect on</param>
	/// <returns>Result</returns>
	extern "C" DS5W_API DS5W_ReturnValue reconnectDevice(DS5W::DeviceContext* ptrContext);

	/// <summary>
	/// Get device input state
	/// Blocks thread until state is read or an error occurs
	/// </summary>
	/// <param name="ptrContext">Pointer to context</param>
	/// <param name="ptrInputState">Pointer to input state</param>
	/// <returns>Result of call</returns>
	extern "C" DS5W_API DS5W_ReturnValue getDeviceInputState(DS5W::DeviceContext* ptrContext, DS5W::DS5InputState* ptrInputState);

	/// <summary>
	/// Set the device output state
	/// Blocks thread until state is read or an error occurs
	/// </summary>
	/// <param name="ptrContext">Pointer to context</param>
	/// <param name="ptrOutputState">Pointer to output state to be set</param>
	/// <returns>Result of call</returns>
	extern "C" DS5W_API DS5W_ReturnValue setDeviceOutputState(DS5W::DeviceContext* ptrContext, DS5W::DS5OutputState* ptrOutputState);

	/// <summary>
	/// Starts an overlapped IO call to get device input report
	/// </summary>
	extern "C" DS5W_API DS5W_ReturnValue startInputRequest(DS5W::DeviceContext* ptrContext);

	/// <summary>
	/// Waits until overlapped call finishes
	/// Only call this if startInputRequest() returned DS5W_E_IO_PENDING
	/// </summary>
	extern "C" DS5W_API DS5W_ReturnValue awaitInputRequest(DS5W::DeviceContext* ptrContext);

	/// <summary>
	/// Parses and copies the last input report read into an InputState struct
	/// Intended to be used with startInputRequest() after the request is completed
	/// </summary>
	extern "C" DS5W_API void getHeldInputState(DS5W::DeviceContext * ptrContext, DS5W::DS5InputState * ptrInputState);
}
