// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "VoiceDataCommon.h"
#include "OnlineSubsystemSteamPackage.h"

/** Defines the data involved in a Steam voice packet */
class FVoicePacketSteam : public FVoicePacket
{

PACKAGE_SCOPE:

	/** The unique net id of the talker sending the data */
	TSharedPtr<class FUniqueNetId> Sender;
	/** The data that is to be sent/processed */
	TArray<uint8> Buffer;
	/** The current amount of space used in the buffer for this packet */
	uint16 Length;

public:
	/** Zeros members and validates the assumptions */
	FVoicePacketSteam() :
		Sender(NULL),
		Length(0)
	{
		Buffer.Empty(MAX_VOICE_DATA_SIZE);
	}

	/** Should only be used by TSharedPtr and FVoiceData */
	virtual ~FVoicePacketSteam()
	{
	}

	/**
	 * Copies another packet and inits the ref count
	 *
	 * @param Other packet to copy
	 * @param InRefCount the starting ref count to use
	 */
	FVoicePacketSteam(const FVoicePacketSteam& Other);

	/** Returns the amount of space this packet will consume in a buffer */
	virtual uint16 GetTotalPacketSize() OVERRIDE;

	/** @return the amount of space used by the internal voice buffer */
	virtual uint16 GetBufferSize() OVERRIDE;

	/** @return the sender of this voice packet */
	virtual TSharedPtr<class FUniqueNetId> GetSender() OVERRIDE;

	/** 
	 * Serialize the voice packet data to a buffer 
	 *
	 * @param Ar buffer to write into
	 */
	virtual void Serialize(class FArchive& Ar) OVERRIDE;
};

/** Holds the current voice packet data state */
struct FVoiceDataSteam
{
	/** Data used by the local talkers before sent */
	FVoicePacketSteam LocalPackets[MAX_SPLITSCREEN_TALKERS];
	/** Holds the set of received packets that need to be processed */
	FVoicePacketList RemotePackets;

	FVoiceDataSteam() {}
	~FVoiceDataSteam() {}
};
