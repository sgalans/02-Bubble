#ifndef _AUDIO_INCLUDE
#define _AUDIO_INCLUDE


#include <string>


// Audio is a singleton that plays sound effects and background music through miniaudio.
// Effects are fire-and-forget and can overlap. Files are looked up in the sounds folder;
// a missing file only prints a warning, so the game keeps running without it.
// To add a new effect: add an entry to Sound, give it a file in soundFiles (Audio.cpp)
// and call Audio::instance().play(...) where the action happens.


class Audio
{

public:
	enum Sound
	{
		MENU_MOVE, MENU_SELECT, BOMB_PLACE, EXPLOSION, ITEM_PICKUP, PLAYER_HURT, SOUND_COUNT
	};

	// Music tracks, files in musicFiles (Audio.cpp)
	enum Music
	{
		MUSIC_TITLE, MUSIC_FOREST, MUSIC_BOSS, MUSIC_WORLD_CLEAR, MUSIC_GAME_OVER, MUSIC_COUNT
	};

	static Audio &instance()
	{
		static Audio A;

		return A;
	}

	~Audio();

	bool init();
	void play(Sound sound);

	void playMusic(Music music, bool loop = true);
	void stopMusic();

	// Both volumes go from 0 (mute) to 1
	void setVolume(float volume);
	void setMusicVolume(float volume);

private:
	Audio();

private:
	struct Impl; // Hides the miniaudio types so only Audio.cpp includes its big header
	Impl *impl;

};


#endif // _AUDIO_INCLUDE
