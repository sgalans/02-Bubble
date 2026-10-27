#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <string>
#include "ShaderProgram.h"
#include "Sprite.h"
#include "Texture.h"


#define MENU_NUM_LEVELS 5
#define MENU_MAX_TEXT_SCALE 5


// Menu is the start screen: it lists the levels and lets the player pick one.
// Text is drawn with a small bitmap font built in code, so it needs no image files.
// Levels whose file does not exist yet are shown greyed out and cannot be picked.


class Menu
{

public:
	Menu();
	~Menu();

	void init();
	void update(int deltaTime);
	void render();

	void keyPressed(int key);

	// Returns the chosen level (1..MENU_NUM_LEVELS) once, or 0 if none was chosen
	int takeChosenLevel();

	static std::string levelFile(int level);

private:
	void initShaders();
	void initFont();
	void moveSelection(int step);
	void renderText(const std::string &text, float x, float y, int scale, const glm::vec4 &color);
	void renderTextCentered(const std::string &text, float y, int scale, const glm::vec4 &color);

private:
	ShaderProgram program;
	Texture fontTexture;
	Sprite *glyph[MENU_MAX_TEXT_SCALE]; // One quad size per text scale (1x..5x)
	bool available[MENU_NUM_LEVELS];
	int selected, chosen;
	float currentTime;

};


#endif // _MENU_INCLUDE
