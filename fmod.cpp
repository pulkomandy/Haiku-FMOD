/* HaLFMOD - Haiku Lightweight FMOD
 * Copyright 2012, Adrien Destugues
 *
 * Distributed under the terms of the MIT Licence.
 *
 * The FMOD API is Copyright (c), Firelight Technologies Pty, Ltd. 2004-2007.*/

#include <fmod.h>

#include <stdio.h>
#include <string.h>

#include <MediaFile.h>
#include <MediaTrack.h>
#include <ObjectList.h>
#include <SoundPlayer.h>

extern "C" {

FMOD_RESULT FMOD_System_Create(FMOD_SYSTEM** system)
{
	return FMOD_OK;
}

FMOD_RESULT FMOD_System_Release(FMOD_SYSTEM* system)
{
	return FMOD_OK;
}

FMOD_RESULT FMOD_System_Init(FMOD_SYSTEM* system, int maxchannels, FMOD_INITFLAGS flags, void* extradriverdata)
{
	return FMOD_ERR_UNIMPLEMENTED;
}

FMOD_RESULT FMOD_System_Close(FMOD_SYSTEM* system)
{
	return FMOD_ERR_UNIMPLEMENTED;
}

FMOD_RESULT FMOD_System_Update(FMOD_SYSTEM* system)
{
	return FMOD_OK;
}

FMOD_RESULT FMOD_System_CreateStream(FMOD_SYSTEM* system, const char* name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO* exinfo, FMOD_SOUND** sound)
{
	if (mode & FMOD_OPENMEMORY)
	{
		BMediaFile* file = new BMediaFile(new BMemoryIO(name_or_data, 4 * 1024 * 1024));
		// Let's hope this is enough, we have no way of knowing the actual size.
		// TODO actually we can first alloc a small part of the buffer, sniff
		// it, then allocate actual size (computed from duration bitrate etc)
		*sound = (FMOD_SOUND*)file;
		return FMOD_OK;
	} else {
		puts("Sorry ! only FMOD_OPENMEMORY mode is supported so far !");
		sound = NULL;
		return FMOD_ERR_UNIMPLEMENTED;
	}
}

void playChannel(void* cookie, void* buffer, size_t size, const media_raw_audio_format& format)
{
	BMediaTrack* file = (BMediaTrack*) cookie;

	int64 size64 = size;
	switch(format.format)
	{
		case format.B_AUDIO_INT:
			size64 /= sizeof(int);
			break;
		case format.B_AUDIO_SHORT:
			size64 /= sizeof(short);
			break;
		case format.B_AUDIO_FLOAT:
			size64 /= sizeof(float);
			break;
		case format.B_AUDIO_CHAR:
		case format.B_AUDIO_UCHAR:
			size64 /= sizeof(char);
			break;
	}
	file->ReadFrames(buffer, &size64);
}

FMOD_RESULT FMOD_System_PlaySound(FMOD_SYSTEM* system, FMOD_CHANNELINDEX channelid, FMOD_SOUND* sound, FMOD_BOOL paused, FMOD_CHANNEL** channel)
{
	BMediaFile* file  = (BMediaFile*)sound;
	BMediaTrack* theMusic = file->TrackAt(0);

	if (theMusic == NULL)
	{
		*channel = NULL;
		return FMOD_ERR_FORMAT;
	}

	media_format format;
	theMusic->DecodedFormat(&format);

	BSoundPlayer* player = new BSoundPlayer(&format.u.raw_audio, "HaLFMOD",
		playChannel, NULL, theMusic);

	*channel = (FMOD_CHANNEL*)player;

	return FMOD_OK;
}

#pragma mark -

FMOD_RESULT FMOD_Sound_Release(FMOD_SOUND* sound)
{
	BMediaFile* file = (BMediaFile*) sound;
	file->ReleaseAllTracks();
	delete file;
	return FMOD_OK;
}

#pragma mark -

FMOD_RESULT FMOD_Channel_SetVolume(FMOD_CHANNEL* channel, float volume)
{
	BSoundPlayer * player = (BSoundPlayer*)channel;
	if (player == NULL)
		return FMOD_ERR_FORMAT;
	player->SetVolume(volume);
	return FMOD_OK;
}

FMOD_RESULT FMOD_Channel_SetPaused(FMOD_CHANNEL* channel, FMOD_BOOL paused)
{
	BSoundPlayer * player = (BSoundPlayer*)channel;
	if (player == NULL)
		return FMOD_ERR_FORMAT;
	if (paused)
		player->Stop();
	else
		player->Start();
	return FMOD_OK;
}

FMOD_RESULT FMOD_Channel_Stop(FMOD_CHANNEL* channel)
{
	BSoundPlayer* player = (BSoundPlayer*) channel;
	if (player == NULL)
		return FMOD_ERR_FORMAT;
	player->Stop(true);
	delete player;
	return FMOD_OK;
}
}
