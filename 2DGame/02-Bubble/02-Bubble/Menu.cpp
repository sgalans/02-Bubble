#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>
#include "Menu.h"
#include "Game.h"
#include "Audio.h"


#define FONT_FILE "fonts/PressStart2P-Regular.ttf"
// The font is drawn on an 8 pixel grid, so multiples of 8 keep it crisp
#define TITLE_SIZE 32
#define ENTRY_SIZE 24
#define FOOTER_SIZE 16
#define TITLE_Y 60
#define FIRST_ENTRY_Y 140
#define ENTRY_SPACING 40
#define FOOTER_Y 430
#define BLINK_PERIOD 500
#define MAIN_FIRST_ENTRY_Y 200
#define MAIN_ENTRY_SPACING 50
#define TEXT_FIRST_LINE_Y 120
#define TEXT_LINE_SPACING 24

// Entries of the main screen, in order
enum MainEntry { MAIN_PLAY, MAIN_INSTRUCTIONS, MAIN_CREDITS, MAIN_NUM_ENTRIES };
static const char *MAIN_LABELS[MAIN_NUM_ENTRIES] = { "PLAY", "INSTRUCTIONS", "CREDITS" };

// Text of the info screens, one line per "\n".
// The body uses the 16 px font, so keep each line within 40 characters.
static const char *INSTRUCTIONS_TEXT =
	"ARROWS/WASD: MOVE   UP/W: HIGH JUMP\n"
	"S: SHORT JUMP   SPACE: DROP A BOMB\n"
	"\n"
	"BLOW UP BUSHES TO FIND HIDDEN ITEMS:\n"
	"BOMB UP: CARRY ONE MORE BOMB\n"
	"FIRE UP: BIGGER EXPLOSIONS\n"
	"SPEED UP: MOVE FASTER\n"
	"VEST: INVULNERABILITY FOR 17 SECONDS\n"
	"KEY: OPENS A LOCKED GATE\n"
	"\n"
	"KILL ALL ENEMIES TO OPEN THE DOOR\n"
	"AND REACH IT BEFORE TIME RUNS OUT!";
static const char *CREDITS_TEXT = "MADE BY:\nSergi Galans and Oriol Valencia";


Menu::Menu()
{
	screen = MAIN;
	mainSelected = 0;
	selected = 0;
	chosen = 0;
	currentTime = 0.0f;
	itemIcon = heartIcon = NULL;
}

Menu::~Menu()
{
	delete itemIcon;
	delete heartIcon;
	program.free();
}


void Menu::init()
{
	initShaders();
	titleText.init(FONT_FILE, TITLE_SIZE, program);
	entryText.init(FONT_FILE, ENTRY_SIZE, program);
	footerText.init(FONT_FILE, FOOTER_SIZE, program);

	itemTexture.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	itemTexture.setMinFilter(GL_NEAREST);
	itemTexture.setMagFilter(GL_NEAREST);
	glm::vec2 itemFrame(16.f / itemTexture.width(), 16.f / itemTexture.height());
	itemIcon = Sprite::createSprite(glm::vec2(32.f), itemFrame, &itemTexture, &program);
	itemIcon->setNumberAnimations(3);
	for(int i = 0; i < 3; i++)
	{
		itemIcon->setAnimationSpeed(i, 1);
		itemIcon->addKeyframe(i, glm::vec2(21.f + i * 18.f, 117.f) / glm::vec2(itemTexture.width(), itemTexture.height()));
	}
	itemIcon->changeAnimation(0);
	heartTexture.loadFromFile("images/hearts.png", TEXTURE_PIXEL_FORMAT_RGBA);
	heartTexture.setMinFilter(GL_NEAREST);
	heartTexture.setMagFilter(GL_NEAREST);
	heartIcon = Sprite::createSprite(glm::vec2(32.f), glm::vec2(0.2f, 0.5f), &heartTexture, &program);
	heartIcon->setNumberAnimations(1);
	heartIcon->setAnimationSpeed(0, 1);
	heartIcon->addKeyframe(0, glm::vec2(0.f));
	heartIcon->changeAnimation(0);

	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		ifstream fin(levelFile(i + 1).c_str());
		available[i] = fin.good();
		unlocked[i] = (i == 0);
	}
	// Start on the first level that can be played
	selected = 0;
	while(selected < MENU_NUM_LEVELS - 1 && !isPlayable(selected + 1))
		selected++;
	chosen = 0;
	currentTime = 0.0f;
	screen = MAIN;
	mainSelected = MAIN_PLAY;
}

void Menu::update(int deltaTime)
{
	currentTime += deltaTime;
}

void Menu::render()
{
	program.use();
	program.setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	program.setUniformMatrix4f("modelview", glm::mat4(1.0f));
	program.setUniform2f("texCoordDispl", 0.f, 0.f);

	switch(screen)
	{
	case MAIN:
		renderMain();
		break;
	case LEVELS:
		renderLevels();
		break;
	case INSTRUCTIONS:
		renderTextScreen("INSTRUCTIONS", INSTRUCTIONS_TEXT);
		break;
	case CREDITS:
		renderTextScreen("CREDITS", CREDITS_TEXT);
		break;
	}
}

void Menu::renderMain()
{
	titleText.renderCentered("MAIN MENU", TITLE_Y, SCREEN_WIDTH, glm::vec4(1.f, 0.8f, 0.2f, 1.f));

	for(int i = 0; i < MAIN_NUM_ENTRIES; i++)
	{
		float y = float(MAIN_FIRST_ENTRY_Y + i * MAIN_ENTRY_SPACING);
		glm::vec4 color = (i == mainSelected) ? glm::vec4(1.f, 1.f, 0.3f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
		entryText.renderCentered(MAIN_LABELS[i], y, SCREEN_WIDTH, color);
		if(i == mainSelected)
			renderArrow(MAIN_LABELS[i], y, color);
	}

	footerText.renderCentered("ARROWS: MOVE  ENTER: SELECT  ESC: EXIT", FOOTER_Y, SCREEN_WIDTH,
	                          glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
}

void Menu::renderTextScreen(const string &title, const string &body)
{
	titleText.renderCentered(title, TITLE_Y, SCREEN_WIDTH, glm::vec4(1.f, 0.8f, 0.2f, 1.f));

	stringstream lines(body);
	string line;
	for(int i = 0; getline(lines, line); i++)
		footerText.renderCentered(line, float(TEXT_FIRST_LINE_Y + i * TEXT_LINE_SPACING), SCREEN_WIDTH,
		                          glm::vec4(1.f, 1.f, 1.f, 1.f));

	footerText.renderCentered("ESC: BACK", FOOTER_Y, SCREEN_WIDTH, glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
}

// Blinking arrow to the left of the selected entry
void Menu::renderArrow(const string &label, float y, const glm::vec4 &color)
{
	if(int(currentTime / BLINK_PERIOD) % 2 != 0)
		return;
	float x = (SCREEN_WIDTH - entryText.width(label)) / 2.f - entryText.width("> ");
	entryText.render(">", glm::vec2(x, y), color);
}

void Menu::renderLevels()
{
	titleText.renderCentered("SELECT LEVEL", TITLE_Y, SCREEN_WIDTH, glm::vec4(1.f, 0.8f, 0.2f, 1.f));

	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		stringstream label;
		label << "LEVEL " << (i + 1);
		float y = float(FIRST_ENTRY_Y + i * ENTRY_SPACING);

		glm::vec4 color;
		if(!isPlayable(i + 1))
			color = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
		else if(i == selected)
			color = glm::vec4(1.f, 1.f, 0.3f, 1.f);
		else
			color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		entryText.renderCentered(label.str(), y, SCREEN_WIDTH, color);

		if(i == selected)
			renderArrow(label.str(), y, color);
	}

	renderStartingStats();
	footerText.renderCentered("ARROWS: MOVE  ENTER: PLAY  ESC: BACK", FOOTER_Y, SCREEN_WIDTH,
	                          glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
}

void Menu::renderStartingStats()
{
	const Game &game = Game::instance();
	Player::PowerUps powerUps = game.getStartingPowerUps(selected + 1);
	const int values[] = { game.getStartingLives(selected + 1), powerUps.nBombs,
	                       powerUps.firePower, powerUps.getSpeedUpLevel() + 1 };
	glm::vec4 white(1.f);
	footerText.renderCentered("START WITH", 350.f, SCREEN_WIDTH, glm::vec4(1.f, 0.8f, 0.2f, 1.f));
	for(int i = 0; i < 4; i++)
	{
		glm::vec2 pos(136.f + i * 100.f, 378.f);
		Sprite *icon = i == 0 ? heartIcon : itemIcon;
		if(i > 0)
			itemIcon->changeAnimation(i - 1);
		program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		icon->setPosition(pos);
		icon->render();
		footerText.render(std::to_string(values[i]), pos + glm::vec2(44.f, 8.f), white);
	}
}

void Menu::keyPressed(int key)
{
	switch(screen)
	{
	case MAIN:
		mainKeyPressed(key);
		break;
	case LEVELS:
		levelsKeyPressed(key);
		break;
	default:
		// The info screens only react to Escape, which Game routes to back()
		break;
	}
}

void Menu::mainKeyPressed(int key)
{
	switch(key)
	{
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		mainSelected = (mainSelected + MAIN_NUM_ENTRIES - 1) % MAIN_NUM_ENTRIES;
		Audio::instance().play(Audio::MENU_MOVE);
		currentTime = 0.0f;
		break;
	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		mainSelected = (mainSelected + 1) % MAIN_NUM_ENTRIES;
		Audio::instance().play(Audio::MENU_MOVE);
		currentTime = 0.0f;
		break;
	case GLFW_KEY_ENTER:
	case GLFW_KEY_KP_ENTER:
	case GLFW_KEY_SPACE:
		Audio::instance().play(Audio::MENU_SELECT);
		if(mainSelected == MAIN_PLAY)
			screen = LEVELS;
		else if(mainSelected == MAIN_INSTRUCTIONS)
			screen = INSTRUCTIONS;
		else
			screen = CREDITS;
		currentTime = 0.0f;
		break;
	}
}

bool Menu::back()
{
	if(screen == MAIN)
		return false;
	screen = MAIN;
	currentTime = 0.0f;
	return true;
}

void Menu::levelsKeyPressed(int key)
{
	switch(key)
	{
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		moveSelection(-1);
		break;
	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		moveSelection(1);
		break;
	case GLFW_KEY_ENTER:
	case GLFW_KEY_KP_ENTER:
	case GLFW_KEY_SPACE:
		if(isPlayable(selected + 1))
			chosen = selected + 1;
		break;
	default:
		// Number keys jump straight into that level, even a locked one (debug shortcut)
		if(key >= GLFW_KEY_1 && key < GLFW_KEY_1 + MENU_NUM_LEVELS && available[key - GLFW_KEY_1])
		{
			selected = key - GLFW_KEY_1;
			chosen = selected + 1;
		}
		break;
	}
}

int Menu::takeChosenLevel()
{
	int level = chosen;
	chosen = 0;
	return level;
}

string Menu::levelFile(int level)
{
	stringstream file;
	file << "levels/level" << setw(2) << setfill('0') << level << ".txt";
	return file.str();
}

void Menu::unlockLevel(int level)
{
	if(level < 1 || level > MENU_NUM_LEVELS)
		return;
	unlocked[level - 1] = true;
	if(available[level - 1])
	{
		selected = level - 1;
		currentTime = 0.0f;
	}
}

bool Menu::isPlayable(int level) const
{
	return available[level - 1] && unlocked[level - 1];
}

// Moves to the next playable level in the given direction, wrapping around
void Menu::moveSelection(int step)
{
	for(int i = 1; i <= MENU_NUM_LEVELS; i++)
	{
		int candidate = ((selected + step * i) % MENU_NUM_LEVELS + MENU_NUM_LEVELS) % MENU_NUM_LEVELS;
		if(isPlayable(candidate + 1))
		{
			if(candidate != selected)
				Audio::instance().play(Audio::MENU_MOVE);
			selected = candidate;
			currentTime = 0.0f; // Restart the blink so the arrow shows right away
			return;
		}
	}
}

void Menu::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	program.init();
	program.addShader(vShader);
	program.addShader(fShader);
	program.link();
	if(!program.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << program.log() << endl << endl;
	}
	program.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}
