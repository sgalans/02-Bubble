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

	static Audio &instance()
	{
		static Audio A;

		return A;
	}

	~Audio();

	bool init();
	void play(Sound sound);

	// Only one music track plays at a time; it loops until stopped
	void playMusic(const std::string &file);
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
