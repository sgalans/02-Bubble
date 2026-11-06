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

// Background picture (160x80) scaled to the full screen height and anchored left,
// so the character stays visible and the right side (sky) is what gets cropped
#define BACKGROUND_FILE "images/menu_background.png"
// Main and level screens put their entries in this column, right of the character
#define COLUMN_X 240.f
#define COLUMN_WIDTH (SCREEN_WIDTH - COLUMN_X)
#define SHADOW_OFFSET 2.f // px, down and right
// Titles and the "START WITH" label
#define TITLE_COLOR glm::vec4(0.85f, 0.55f, 0.1f, 1.f)
// Level select: unlocked levels, and locked ones in a pale, muted yellow
#define LEVEL_COLOR glm::vec4(0.8f, 0.8f, 0.8f, 1.f)
#define LOCKED_COLOR glm::vec4(0.75f, 0.7f, 0.45f, 1.f)

// Entries of the main screen, in order
enum MainEntry { MAIN_PLAY, MAIN_INSTRUCTIONS, MAIN_CREDITS, MAIN_NUM_ENTRIES };
static const char *MAIN_LABELS[MAIN_NUM_ENTRIES] = { "PLAY", "INSTRUCTIONS", "CREDITS" };

// Text of the info screens, one line per "\n".
// The body uses the 16 px font, so keep each line within 40 characters.
static const char *INSTRUCTIONS_TEXT =
	"ARROWS/WASD: MOVE   UP/W: HIGH JUMP\n"
	"S: SHORT JUMP   SPACE: DROP A BOMB\n"
	"F11: TOGGLE FULLSCREEN\n"
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
	backgroundSprite = NULL;
}

Menu::~Menu()
{
	delete itemIcon;
	delete heartIcon;
	delete backgroundSprite;
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

	backgroundTexture.loadFromFile(BACKGROUND_FILE, TEXTURE_PIXEL_FORMAT_RGBA);
	backgroundTexture.setMinFilter(GL_NEAREST);
	backgroundTexture.setMagFilter(GL_NEAREST);
	float backgroundScale = float(SCREEN_HEIGHT) / backgroundTexture.height();
	backgroundSprite = Sprite::createSprite(backgroundScale * glm::vec2(backgroundTexture.width(), backgroundTexture.height()),
	                                        glm::vec2(1.f, 1.f), &backgroundTexture, &program);
	backgroundSprite->setNumberAnimations(1);
	backgroundSprite->setAnimationSpeed(0, 1);
	backgroundSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	backgroundSprite->changeAnimation(0);
	backgroundSprite->setPosition(glm::vec2(0.f, 0.f));

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
	// The info screens are long text, so they get plain black to stay readable
	if(screen == MAIN || screen == LEVELS)
		renderBackground();
	else
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.3f, 0.3f, 0.3f, 1.f); // Game's normal clear colour
	}

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
	renderInColumn(titleText, "MAIN MENU", TITLE_Y, TITLE_COLOR);

	for(int i = 0; i < MAIN_NUM_ENTRIES; i++)
	{
		float y = float(MAIN_FIRST_ENTRY_Y + i * MAIN_ENTRY_SPACING);
		glm::vec4 color = (i == mainSelected) ? glm::vec4(1.f, 1.f, 0.3f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
		renderInColumn(entryText, MAIN_LABELS[i], y, color);
		if(i == mainSelected)
			renderArrow(MAIN_LABELS[i], y, color);
	}

	renderShadowed(footerText, "ARROWS: MOVE  ENTER: SELECT  ESC: EXIT",
	               glm::vec2((SCREEN_WIDTH - footerText.width("ARROWS: MOVE  ENTER: SELECT  ESC: EXIT")) / 2.f, FOOTER_Y),
	               glm::vec4(1.f));
}

void Menu::renderTextScreen(const string &title, const string &body)
{
	titleText.renderCentered(title, TITLE_Y, SCREEN_WIDTH, TITLE_COLOR);

	stringstream lines(body);
	string line;
	for(int i = 0; getline(lines, line); i++)
		footerText.renderCentered(line, float(TEXT_FIRST_LINE_Y + i * TEXT_LINE_SPACING), SCREEN_WIDTH,
		                          glm::vec4(1.f, 1.f, 1.f, 1.f));

	footerText.renderCentered("ESC: BACK", FOOTER_Y, SCREEN_WIDTH, glm::vec4(1.f));
}

// Blinking arrow to the left of the selected entry
void Menu::renderArrow(const string &label, float y, const glm::vec4 &color)
{
	if(int(currentTime / BLINK_PERIOD) % 2 != 0)
		return;
	float x = COLUMN_X + (COLUMN_WIDTH - entryText.width(label)) / 2.f - entryText.width("> ");
	renderShadowed(entryText, ">", glm::vec2(x, y), color);
}

void Menu::renderBackground()
{
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	backgroundSprite->render();
}

void Menu::renderInColumn(Text &text, const string &str, float y, const glm::vec4 &color)
{
	renderShadowed(text, str, glm::vec2(COLUMN_X + (COLUMN_WIDTH - text.width(str)) / 2.f, y), color);
}

// Black copy offset down-right first, so text reads over the busy background
void Menu::renderShadowed(Text &text, const string &str, const glm::vec2 &pos, const glm::vec4 &color)
{
	text.render(str, pos + glm::vec2(SHADOW_OFFSET), glm::vec4(0.f, 0.f, 0.f, 1.f));
	text.render(str, pos, color);
}

void Menu::renderLevels()
{
	renderInColumn(titleText, "SELECT LEVEL", TITLE_Y, TITLE_COLOR);

	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		stringstream label;
		label << "LEVEL " << (i + 1);
		float y = float(FIRST_ENTRY_Y + i * ENTRY_SPACING);

		glm::vec4 color;
		if(!isPlayable(i + 1))
			color = LOCKED_COLOR;
		else if(i == selected)
			color = glm::vec4(1.f, 1.f, 0.3f, 1.f);
		else
			color = LEVEL_COLOR;
		renderInColumn(entryText, label.str(), y, color);

		if(i == selected)
			renderArrow(label.str(), y, color);
	}

	renderStartingStats();
	renderShadowed(footerText, "ARROWS: MOVE  ENTER: PLAY  ESC: BACK",
	               glm::vec2((SCREEN_WIDTH - footerText.width("ARROWS: MOVE  ENTER: PLAY  ESC: BACK")) / 2.f, FOOTER_Y),
	               glm::vec4(1.f));
}

void Menu::renderStartingStats()
{
	const Game &game = Game::instance();
	Player::PowerUps powerUps = game.getStartingPowerUps(selected + 1);
	const int values[] = { game.getStartingLives(selected + 1), powerUps.nBombs,
	                       powerUps.firePower, powerUps.getSpeedUpLevel() + 1 };
	glm::vec4 white(1.f);
	renderInColumn(footerText, "START WITH", 350.f, TITLE_COLOR);
	// Four icon+number pairs 100 px apart; the last pair is about 76 px wide
	const float rowWidth = 3 * 100.f + 76.f;
	const float rowX = COLUMN_X + (COLUMN_WIDTH - rowWidth) / 2.f;
	for(int i = 0; i < 4; i++)
	{
		glm::vec2 pos(rowX + i * 100.f, 378.f);
		Sprite *icon = i == 0 ? heartIcon : itemIcon;
		if(i > 0)
			itemIcon->changeAnimation(i - 1);
		program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		icon->setPosition(pos);
		icon->render();
		renderShadowed(footerText, std::to_string(values[i]), pos + glm::vec2(44.f, 8.f), white);
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
