#include <iostream>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "Audio.h"


using namespace std;


// Indexed by Audio::Sound. Any format miniaudio decodes works (wav, mp3, flac).
static const char *soundFiles[Audio::SOUND_COUNT] =
{
	"sounds/sound interactions/menu_move.wav",
	"sounds/sound interactions/menu_select.wav",
	"sounds/sound interactions/bomb_place.wav",
	"sounds/sound interactions/explosion.wav",
	"sounds/sound interactions/item_pickup.wav",
	"sounds/sound interactions/player_hurt.wav",
	"sounds/sound interactions/door_open.wav",
	"sounds/sound interactions/jump.wav",
	"sounds/sound interactions/enemy_die.wav"
};

// Indexed by Audio::Music
static const char *musicFiles[Audio::MUSIC_COUNT] =
{
	"sounds/music/02_Title Screen.mp3",
	"sounds/music/06_World of Forest [Area 1].mp3",
	"sounds/music/11._20Boss_20Battle.mp3",
	"sounds/music/17._20World_20Clear.mp3",
	"sounds/music/20. Game Over.mp3"
};


struct Audio::Impl
{
	ma_engine engine;
	ma_sound music;
	bool bMusicLoaded;
	int currentMusic; // -1 when nothing is playing
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
	impl->currentMusic = -1;
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

void Audio::playMusic(Music music, bool loop)
{
	if(impl == NULL || music < 0 || music >= MUSIC_COUNT || impl->currentMusic == music)
		return;
	stopMusic();
	// Streamed instead of decoded up front because music files are long
	if(ma_sound_init_from_file(&impl->engine, musicFiles[music], MA_SOUND_FLAG_STREAM, NULL, NULL, &impl->music) != MA_SUCCESS)
	{
		cout << "Could not load music " << musicFiles[music] << endl;
		return;
	}
	impl->bMusicLoaded = true;
	impl->currentMusic = music;
	ma_sound_set_looping(&impl->music, loop ? MA_TRUE : MA_FALSE);
	ma_sound_set_volume(&impl->music, impl->musicVolume);
	ma_sound_start(&impl->music);
}

void Audio::stopMusic()
{
	if(impl == NULL || !impl->bMusicLoaded)
		return;
	ma_sound_uninit(&impl->music);
	impl->bMusicLoaded = false;
	impl->currentMusic = -1;
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
