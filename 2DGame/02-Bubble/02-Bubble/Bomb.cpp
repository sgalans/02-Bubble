#include "Bomb.h"


#define FUSE_TIME 2500 //ms
#define EXPLOSION_TIME 600 //ms

// bombs.png holds three 16x16 bomb frames, each a bit smaller than the
// previous one, so cycling them makes the bomb pulse. They are not on a
// regular 16 px grid, so each frame is addressed by its pixel offset.
#define SHEET_WIDTH 49.f
#define SHEET_HEIGHT 16.f
#define FRAME_SIZE 16.f
#define ROW_Y 0.f
#define FRAME_A_X 0.f
#define FRAME_B_X 17.f
#define FRAME_C_X 33.f

// explosions.png is a regular grid of 16x16 frames: one column per
// FlamePiece and one row per intensity, from weakest to strongest.
#define N_INTENSITIES 3


enum BombAnims
{
	PULSE
};

// The flames grow and then fade: each entry lasts the same time
static const int INTENSITY_SEQUENCE[] = { 0, 1, 2, 2, 1, 0 };
static const int SEQUENCE_LENGTH = sizeof(INTENSITY_SEQUENCE) / sizeof(INTENSITY_SEQUENCE[0]);


Bomb::Bomb()
{
	sprite = NULL;
	flameSprite = NULL;
	map = NULL;
}

Bomb::~Bomb()
{
	if(sprite != NULL)
		delete sprite;
	if(flameSprite != NULL)
		delete flameSprite;
}

void Bomb::init(const glm::ivec2 &tile, int range, const glm::ivec2 &tileMapPos, TileMap *tileMap,
                Texture *bombSheet, Texture *flameSheet, ShaderProgram &shaderProgram)
{
	this->tile = tile;
	this->range = range;
	tileMapDispl = tileMapPos;
	map = tileMap;
	state = FUSE;
	timer = FUSE_TIME;

	sprite = Sprite::createSprite(glm::vec2(getSize()),
	                              glm::vec2(FRAME_SIZE / SHEET_WIDTH, FRAME_SIZE / SHEET_HEIGHT),
	                              bombSheet, &shaderProgram);
	sprite->setNumberAnimations(1);

		sprite->setAnimationSpeed(PULSE, 6);
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_C_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	sprite->changeAnimation(PULSE);
	sprite->setPosition(getPosition());

	// One single-frame animation per (piece, intensity). All flame cells share
	// this sprite: render() picks the frame and moves it before drawing each one.
	flameSprite = Sprite::createSprite(glm::vec2(getSize()),
	                                   glm::vec2(1.f / N_PIECES, 1.f / N_INTENSITIES),
	                                   flameSheet, &shaderProgram);
	flameSprite->setNumberAnimations(N_PIECES * N_INTENSITIES);
	for(int piece = 0; piece < N_PIECES; piece++)
	{
		for(int intensity = 0; intensity < N_INTENSITIES; intensity++)
		{
			int anim = piece * N_INTENSITIES + intensity;
			flameSprite->setAnimationSpeed(anim, 1);
			flameSprite->addKeyframe(anim, glm::vec2(float(piece) / N_PIECES, float(intensity) / N_INTENSITIES));
		}
	}
}

void Bomb::update(int deltaTime)
{
	if(state == DONE)
		return;

	timer -= deltaTime;
	if(state == FUSE)
	{
		sprite->update(deltaTime);
		if(timer <= 0)
			explode();
	}
	else if(state == EXPLODING && timer <= 0)
		state = DONE;
}

void Bomb::render()
{
	if(state == FUSE)
		sprite->render();
	else if(state == EXPLODING)
	{
		int step = (EXPLOSION_TIME - timer) * SEQUENCE_LENGTH / EXPLOSION_TIME;
		step = glm::clamp(step, 0, SEQUENCE_LENGTH - 1);
		int intensity = INTENSITY_SEQUENCE[step];
		for(unsigned int i = 0; i < flames.size(); i++)
		{
			flameSprite->changeAnimation(flames[i].piece * N_INTENSITIES + intensity);
			flameSprite->setPosition(tilePosition(flames[i].tile));
			flameSprite->render();
		}
	}
}

// Also used to detonate it early (e.g. when another explosion reaches it)
void Bomb::explode()
{
	if(state != FUSE)
		return;
	map->clearBomb(tile.x, tile.y);
	spreadFlames();
	state = EXPLODING;
	timer = EXPLOSION_TIME;
}

bool Bomb::flameCovers(const glm::ivec2 &cell) const
{
	if(state != EXPLODING)
		return false;
	for(unsigned int i = 0; i < flames.size(); i++)
	{
		if(flames[i].tile == cell)
			return true;
	}

	return false;
}

// Whether any flame cell overlaps the given box (in the same coordinates as
// the sprites, i.e. including the tile map displacement)
bool Bomb::flameTouches(const glm::vec2 &pos, const glm::ivec2 &size) const
{
	if(state != EXPLODING)
		return false;
	float cellSize = float(map->getTileSize());
	for(unsigned int i = 0; i < flames.size(); i++)
	{
		glm::vec2 cellPos = tilePosition(flames[i].tile);
		if(pos.x < cellPos.x + cellSize && cellPos.x < pos.x + size.x &&
		   pos.y < cellPos.y + cellSize && cellPos.y < pos.y + size.y)
			return true;
	}

	return false;
}

// Builds the cross of flames. Each arm stops before a wall, or on the cell
// of another bomb (which Scene will then detonate). The last cell of an arm
// uses the end piece so the flame looks closed.
void Bomb::spreadFlames()
{
	static const glm::ivec2 dirs[4] = { glm::ivec2(0, -1), glm::ivec2(0, 1), glm::ivec2(-1, 0), glm::ivec2(1, 0) };
	static const FlamePiece ends[4] = { END_UP, END_DOWN, END_LEFT, END_RIGHT };
	static const FlamePiece arms[4] = { ARM_V, ARM_V, ARM_H, ARM_H };

	Flame center = { tile, CENTER };
	flames.clear();
	flames.push_back(center);
	for(int d = 0; d < 4; d++)
	{
		int armStart = int(flames.size());
		for(int i = 1; i <= range; i++)
		{
			glm::ivec2 cell = tile + dirs[d] * i;
			if(map->isWall(cell.x, cell.y))
				break;
			Flame flame = { cell, arms[d] };
			flames.push_back(flame);
			if(map->hasBomb(cell.x, cell.y))
				break;
		}
		if(int(flames.size()) > armStart)
			flames.back().piece = ends[d];
	}
}
