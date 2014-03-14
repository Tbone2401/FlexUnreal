// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"
#include "IPAddress.h"

/**
 * Represents an internet ip address, using the relatively standard SOCKADDR_IN structure. All data is in network byte order
 */

class FInternetAddrRaw; 

class FInternetAddrX : public FInternetAddr
{
	
public:
	/**
	 * Constructor. Sets address to default state
	 */
	FInternetAddrX();
	/**
	 * Sets the ip address from a host byte order uint32
	 *
	 * @param InAddr the new address to use (must convert to network byte order)
	 */
	virtual void SetIp(uint32 InAddr) OVERRIDE;
	/**
	 * Sets the ip address from a string ("A.B.C.D")
	 *
	 * @param InAddr the string containing the new ip address to use
	 */
	virtual void SetIp(const TCHAR* InAddr, bool& bIsValid) OVERRIDE;

	/**
	 * Copies the network byte order ip address to a host byte order dword
	 *
	 * @param OutAddr the out param receiving the ip address
	 */
	virtual void GetIp(uint32& OutAddr) const OVERRIDE;

	/**
	 * Sets the port number from a host byte order int
	 *
	 * @param InPort the new port to use (must convert to network byte order)
	 */
	virtual void SetPort(int32 InPort) OVERRIDE;

	/**
	 * Copies the port number from this address and places it into a host byte order int
	 *
	 * @param OutPort the host byte order int that receives the port
	 */
	virtual void GetPort(int32& OutPort) const OVERRIDE;

	/**
	 * Returns the port number from this address in host byte order
	 */
	virtual int32 GetPort(void) const OVERRIDE;

	/** Sets the address to be any address */
	virtual void SetAnyAddress(void) OVERRIDE;

	/** Sets the address to broadcast */
	virtual void SetBroadcastAddress() OVERRIDE;

	/**
	 * Converts this internet ip address to string form
	 *
	 * @param bAppendPort whether to append the port information or not
	 */
	virtual FString ToString(bool bAppendPort) const OVERRIDE;
	/**
	 * Is this a well formed internet address
	 *
	 * @return true if a valid IP, false otherwise
	 */
	virtual bool IsValid() const OVERRIDE;

	FInternetAddrRaw* GetPimpl() { return Pimpl; }

private: 

	FInternetAddrRaw* Pimpl;  
};


