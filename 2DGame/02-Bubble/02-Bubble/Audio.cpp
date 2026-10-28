#include <iostream>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "Audio.h"


using namespace std;


// Indexed by Audio::Sound. Any format miniaudio decodes works (wav, mp3, flac).
static const char *soundFiles[Audio::SOUND_COUNT] =
{
	"sounds/menu_move.wav",
	"sounds/menu_select.wav",
	"sounds/bomb_place.wav",
	"sounds/explosion.wav",
	"sounds/item_pickup.wav",
	"sounds/player_hurt.wav"
};


struct Audio::Impl
{
	ma_engine engine;
	ma_sound music;
	bool bMusicLoaded;
	float musicVolume;
};


Audio::Audio()
{
	impl = NULL;
}

Audio::~Audio()
{
	if(impl == NULL)
		return;
	stopMusic();
	ma_engine_uninit(&impl->engine);
	delete impl;
}

bool Audio::init()
{
	if(impl != NULL)
		return true;
	impl = new Impl();
	impl->bMusicLoaded = false;
	impl->musicVolume = 1.0f;
	if(ma_engine_init(NULL, &impl->engine) != MA_SUCCESS)
	{
		cout << "Could not initialize audio, the game will run without sound" << endl;
		delete impl;
		impl = NULL;
		return false;
	}

	// Decode every effect now so the first play() does not stall the frame
	for(int i = 0; i < SOUND_COUNT; i++)
	{
		if(ma_resource_manager_register_file(ma_engine_get_resource_manager(&impl->engine), soundFiles[i],
		                                     MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE) != MA_SUCCESS)
			cout << "Could not load sound " << soundFiles[i] << endl;
	}

	return true;
}

void Audio::play(Sound sound)
{
	if(impl == NULL || sound < 0 || sound >= SOUND_COUNT)
		return;
	ma_engine_play_sound(&impl->engine, soundFiles[sound], NULL);
}

void Audio::playMusic(const string &file)
{
	if(impl == NULL)
		return;
	stopMusic();
	// Streamed instead of decoded up front because music files are long
	if(ma_sound_init_from_file(&impl->engine, file.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &impl->music) != MA_SUCCESS)
	{
		cout << "Could not load music " << file << endl;
		return;
	}
	impl->bMusicLoaded = true;
	ma_sound_set_looping(&impl->music, MA_TRUE);
	ma_sound_set_volume(&impl->music, impl->musicVolume);
	ma_sound_start(&impl->music);
}

void Audio::stopMusic()
{
	if(impl == NULL || !impl->bMusicLoaded)
		return;
	ma_sound_uninit(&impl->music);
	impl->bMusicLoaded = false;
}

void Audio::setVolume(float volume)
{
	if(impl != NULL)
		ma_engine_set_volume(&impl->engine, volume);
}

void Audio::setMusicVolume(float volume)
{
	if(impl == NULL)
		return;
	impl->musicVolume = volume;
	if(impl->bMusicLoaded)
		ma_sound_set_volume(&impl->music, volume);
}
