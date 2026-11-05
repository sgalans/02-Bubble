#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include "TileMap.h"


using namespace std;

#define PLATFORM_CRUMBLE_TIME 1000 // ms
#define PLATFORM_ABSENT_TIME 2000 // ms
#define PLATFORM_REFORM_TIME 1000 // ms
#define PLATFORM_FRAMES 3
#define PLATFORM_SOLID -3
#define TRAMPOLINE_SOLID -4
#define TRAMPOLINE_HOLD_TIME 1000 // ms at full extension
#define TRAMPOLINE_TIME (400 + TRAMPOLINE_HOLD_TIME) // ms, including retraction


TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);
	
	return map;
}


TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	map = NULL;
	bombMap = NULL;
	bushSprite = NULL;
	platformSprite = NULL;
	trampolineSprite = NULL;
	position = glm::ivec2(minCoords);
	loadLevel(levelFile);
	prepareArrays(minCoords, program);
	if(!platforms.empty())
	{
		glm::vec2 frameSize(16.f / tilesheet.width(), 16.f / tilesheet.height());
		platformSprite = Sprite::createSprite(glm::vec2(float(blockSize)), frameSize, &tilesheet, &program);
		platformSprite->setNumberAnimations(PLATFORM_FRAMES);
		for(int frame = 0; frame < PLATFORM_FRAMES; frame++)
		{
			platformSprite->setAnimationSpeed(frame, 1);
			int index = 9 + frame;
			platformSprite->addKeyframe(frame, glm::vec2(float(index % tilesheetSize.x), float(index / tilesheetSize.x)) * frameSize);
		}
	}
	if(!trampolines.empty())
	{
		glm::vec2 frameSize(16.f / tilesheet.width(), 16.f / tilesheet.height());
		trampolineSprite = Sprite::createSprite(glm::vec2(float(blockSize)), frameSize, &tilesheet, &program);
		trampolineSprite->setNumberAnimations(3);
		for(int frame = 0; frame < 3; frame++)
		{
			trampolineSprite->setAnimationSpeed(frame, 1);
			trampolineSprite->addKeyframe(frame, glm::vec2(float(frame), 4.f) * frameSize);
		}
	}
	for(int i = 0; i < mapSize.x * mapSize.y; i++)
	{
		if(map[i] != -1)
			continue;
		bushTexture.loadFromFile("images/personalized.png", TEXTURE_PIXEL_FORMAT_RGBA);
		bushTexture.setMinFilter(GL_NEAREST);
		bushTexture.setMagFilter(GL_NEAREST);
		// Tile 4 is at column 3, row 0 in the 5x5 tilesheet.
		const glm::vec2 bushTexSize(16.f / bushTexture.width(), 16.f / bushTexture.height());
		bushSprite = Sprite::createSprite(glm::vec2(float(blockSize)), bushTexSize, &bushTexture, &program);
		bushSprite->setNumberAnimations(1);
		bushSprite->setAnimationSpeed(0, 1);
		bushSprite->addKeyframe(0, glm::vec2(3.f * bushTexSize.x, 0.f));
		bushSprite->changeAnimation(0);
		break;
	}
}

TileMap::~TileMap()
{
	delete bushSprite;
	delete platformSprite;
	delete trampolineSprite;
	if(map != NULL)
		delete [] map;
	if(bombMap != NULL)
		delete [] bombMap;
	free();
}


void TileMap::render() const
{
	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
	glDisable(GL_TEXTURE_2D);
	for(unsigned int i = 0; i < trampolines.size(); i++)
	{
		trampolineSprite->changeAnimation(trampolines[i].frame);
		trampolineSprite->setPosition(glm::vec2(position + trampolines[i].tile * tileSize));
		trampolineSprite->render();
	}
	for(unsigned int i = 0; i < platforms.size(); i++)
	{
		const CrumblingPlatform &platform = platforms[i];
		int frame = 0;
		if(platform.elapsed >= PLATFORM_CRUMBLE_TIME + PLATFORM_ABSENT_TIME)
		{
			int time = platform.elapsed - PLATFORM_CRUMBLE_TIME - PLATFORM_ABSENT_TIME;
			frame = PLATFORM_FRAMES - 1 - glm::min(time * PLATFORM_FRAMES / PLATFORM_REFORM_TIME, PLATFORM_FRAMES - 1);
		}
		else if(platform.elapsed >= PLATFORM_CRUMBLE_TIME)
			continue;
		else if(platform.elapsed >= 0)
			frame = platform.elapsed * PLATFORM_FRAMES / PLATFORM_CRUMBLE_TIME;
		platformSprite->changeAnimation(frame);
		platformSprite->setPosition(glm::vec2(position + platform.tile * tileSize));
		platformSprite->render();
	}
	if(bushSprite != NULL)
	{
		for(int y = 0; y < mapSize.y; y++)
		{
			for(int x = 0; x < mapSize.x; x++)
			{
				if(map[y * mapSize.x + x] != -1)
					continue;
				bushSprite->setPosition(glm::vec2(position + glm::ivec2(x, y) * tileSize));
				bushSprite->render();
			}
		}
	}
}

void TileMap::update(int deltaTime, const std::vector<glm::ivec4> &occupiedBoxes)
{
	for(unsigned int i = 0; i < trampolines.size(); i++)
	{
		Trampoline &spring = trampolines[i];
		spring.launched = false;
		if(spring.elapsed < 0)
			continue;
		int previous = spring.elapsed;
		int previousFrame = spring.frame;
		int previousTop = getTrampolineTop(spring);
		spring.elapsed += glm::clamp(deltaTime, 0, TRAMPOLINE_TIME - spring.elapsed);
		spring.launched = previous < 200 && spring.elapsed >= 200;
		// Show full extension for at least one frame, even after a long frame.
		if(spring.launched)
			spring.elapsed = 200;
		if(spring.elapsed < 100 || spring.elapsed >= 300 + TRAMPOLINE_HOLD_TIME)
			spring.frame = 0;
		else if(spring.elapsed < 200 || spring.elapsed >= 200 + TRAMPOLINE_HOLD_TIME)
			spring.frame = 1;
		else
			spring.frame = 2;
		int top = getTrampolineTop(spring);
		// Wait rather than squeeze a rider into a ceiling while extending.
		bool blocked = false;
		for(unsigned int j = 0; top < previousTop && j < occupiedBoxes.size(); j++)
		{
			const glm::ivec4 &box = occupiedBoxes[j];
			int left = spring.tile.x * tileSize;
			if(box.y + box.w != previousTop || box.x >= left + tileSize || box.x + box.z <= left)
				continue;
			for(int y = box.y - 1; y >= top - box.w; y--)
			{
				int correctedY = y;
				if(collisionMoveUp(glm::ivec2(box.x, y), glm::ivec2(box.z, box.w), &correctedY))
				{
					blocked = true;
					break;
				}
			}
		}
		if(blocked)
		{
			spring.elapsed = previous;
			spring.frame = previousFrame;
			spring.launched = false;
		}
		if(spring.elapsed == TRAMPOLINE_TIME)
			spring.elapsed = -1;
	}
	const int cycleTime = PLATFORM_CRUMBLE_TIME + PLATFORM_ABSENT_TIME + PLATFORM_REFORM_TIME;
	for(unsigned int i = 0; i < platforms.size(); i++)
	{
		CrumblingPlatform &platform = platforms[i];
		if(platform.elapsed < 0)
			continue;
		platform.elapsed += glm::clamp(deltaTime, 0, cycleTime - platform.elapsed);
		int &cell = map[platform.tile.y * mapSize.x + platform.tile.x];
		if(platform.elapsed >= PLATFORM_CRUMBLE_TIME)
			cell = 0;
		if(platform.elapsed < cycleTime)
			continue;
		// Never restore collision inside a player, enemy or bomb.
		bool occupied = hasBomb(platform.tile.x, platform.tile.y);
		glm::ivec2 pos = platform.tile * tileSize;
		for(unsigned int j = 0; j < occupiedBoxes.size(); j++)
		{
			const glm::ivec4 &box = occupiedBoxes[j];
			if(pos.x < box.x + box.z && box.x < pos.x + blockSize &&
			   pos.y < box.y + box.w && box.y < pos.y + blockSize)
				occupied = true;
		}
		if(!occupied)
		{
			cell = PLATFORM_SOLID;
			platform.elapsed = -1;
		}
	}
}

void TileMap::stepOnPlatforms(const glm::ivec2 &pos, const glm::ivec2 &size)
{
	for(unsigned int i = 0; i < platforms.size(); i++)
	{
		CrumblingPlatform &platform = platforms[i];
		glm::ivec2 top = platform.tile * tileSize;
		if(platform.elapsed == -1 && pos.y + size.y == top.y &&
		   pos.x < top.x + tileSize && top.x < pos.x + size.x)
			platform.elapsed = 0;
	}
}

void TileMap::free()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

bool TileMap::loadLevel(const string &levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	stringstream sstream;
	char tile;
	
	fin.open(levelFile.c_str());
	if(!fin.is_open())
		return false;
	getline(fin, line);
	if(line.compare(0, 7, "TILEMAP") != 0)
		return false;
	getline(fin, line);
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;
	getline(fin, line);
	sstream.str(line);
	sstream >> tileSize >> blockSize;
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetFile;
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;
	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);
	
	map = new int[mapSize.x * mapSize.y];
	spawns.clear();
	platforms.clear();
	trampolines.clear();
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			fin.get(tile);
			if(tile == ' ')
				map[j*mapSize.x+i] = 0;
			else if(tile == '*')
				map[j*mapSize.x+i] = -1;
			else if(tile == 'a')
			{
				map[j*mapSize.x+i] = PLATFORM_SOLID;
				CrumblingPlatform platform = { glm::ivec2(i, j), -1 };
				platforms.push_back(platform);
			}
			else if(tile >= '!' && tile <= '%')
				map[j*mapSize.x+i] = 36 + (tile - '!');
			else if(tile == 'T')
			{
				map[j*mapSize.x+i] = TRAMPOLINE_SOLID;
				Trampoline spring = { glm::ivec2(i, j), -1, 0, false };
				trampolines.push_back(spring);
			}
			else if(isupper(tile) || tile == '@')
			{
				// Items hide in bushes; key gates block movement and flames.
				map[j*mapSize.x+i] = (tile == 'B' || tile == 'F' || tile == 'S' || tile == 'P' || tile == 'K') ? -1 : 0;
				if(tile == 'L')
					map[j*mapSize.x+i] = -2;
				Spawn spawn = { tile, glm::ivec2(i, j) };
				spawns.push_back(spawn);
			}
			else if(islower(tile))
				map[j*mapSize.x+i] = 10 + (tile - int('a'));
			else
				map[j*mapSize.x+i] = tile - int('0');
		}
		fin.get(tile);
#ifndef _WIN32
		fin.get(tile);
#endif
	}
	fin.close();

	bombMap = new int[mapSize.x * mapSize.y];
	for(int i=0; i<mapSize.x * mapSize.y; i++)
		bombMap[i] = NO_BOMB;

	return true;
}

void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	updateTileVertices();
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
}

void TileMap::updateTileVertices()
{
	int tile;
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;
	
	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if(tile > 0)
			{
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(position.x + i * tileSize, position.y + j * tileSize);
				texCoordTile[0] = glm::vec2(float((tile-1)%tilesheetSize.x) / tilesheetSize.x, float((tile-1)/tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				//texCoordTile[0] += halfTexel;
				texCoordTile[1] -= halfTexel;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), vertices.empty() ? NULL : &vertices[0], GL_STATIC_DRAW);
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = pos.x / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(overlapsSolid(x, y, pos, size))
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = (pos.x + size.x - 1) / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(overlapsSolid(x, y, pos, size))
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;
	
	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;
	for(int x=x0; x<=x1; x++)
	{
		if(overlapsSolid(x, y, pos, size))
		{
			int top = solidTop(x, y);
			if(*posY - top + size.y <= 4)
			{
				*posY = top - size.y;
				return true;
			}
		}
	}
	
	return false;
}

bool TileMap::collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;

	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = pos.y / tileSize;
	for(int x=x0; x<=x1; x++)
	{
		if(overlapsSolid(x, y, pos, size))
		{
			*posY = tileSize * (y + 1);
			return true;
		}
	}

	return false;
}

bool TileMap::isWall(int x, int y) const
{
	return !insideMap(x, y) || map[y*mapSize.x+x] != 0;
}

int TileMap::getTrampolineTop(const Trampoline &spring) const
{
	static const int heights[] = { 9, 12, 16 };
	return (spring.tile.y + 1) * tileSize - heights[spring.frame] * tileSize / 16;
}

bool TileMap::activateTrampoline(int x, int y)
{
	for(unsigned int i = 0; i < trampolines.size(); i++)
	{
		Trampoline &spring = trampolines[i];
		if(spring.tile != glm::ivec2(x, y))
			continue;
		if(spring.elapsed < 0)
			spring.elapsed = 0;
		return true;
	}
	return false;
}

int TileMap::solidTop(int x, int y) const
{
	if(insideMap(x, y) && map[y * mapSize.x + x] == TRAMPOLINE_SOLID)
	{
		for(unsigned int i = 0; i < trampolines.size(); i++)
		{
			if(trampolines[i].tile == glm::ivec2(x, y))
				return getTrampolineTop(trampolines[i]);
		}
	}
	return y * tileSize;
}

bool TileMap::overlapsSolid(int x, int y, const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	return isSolid(x, y) && pos.y + size.y > solidTop(x, y) && pos.y < (y + 1) * tileSize;
}

bool TileMap::destroyDestructible(int x, int y)
{
	if(!insideMap(x, y) || map[y*mapSize.x+x] != -1)
		return false;
	map[y*mapSize.x+x] = 0;
	return true;
}

bool TileMap::openKeyGate(int x, int y)
{
	if(!insideMap(x, y) || map[y*mapSize.x+x] != -2)
		return false;
	map[y*mapSize.x+x] = 0;
	return true;
}

bool TileMap::setBomb(int x, int y)
{
	if(!insideMap(x, y) || map[y*mapSize.x+x] != 0 || bombMap[y*mapSize.x+x] != NO_BOMB)
		return false;
	bombMap[y*mapSize.x+x] = BOMB_PASSABLE;

	return true;
}

void TileMap::clearBomb(int x, int y)
{
	if(insideMap(x, y))
		bombMap[y*mapSize.x+x] = NO_BOMB;
}

bool TileMap::hasBomb(int x, int y) const
{
	return insideMap(x, y) && bombMap[y*mapSize.x+x] != NO_BOMB;
}

bool TileMap::isBombPassable(int x, int y) const
{
	return insideMap(x, y) && bombMap[y*mapSize.x+x] == BOMB_PASSABLE;
}

// Called once nobody is standing inside the bomb any more
void TileMap::solidifyBomb(int x, int y)
{
	if(isBombPassable(x, y))
		bombMap[y*mapSize.x+x] = BOMB_SOLID;
}

bool TileMap::insideMap(int x, int y) const
{
	return x >= 0 && x < mapSize.x && y >= 0 && y < mapSize.y;
}

bool TileMap::isSolid(int x, int y) const
{
	return !insideMap(x, y) || map[y*mapSize.x+x] != 0 || bombMap[y*mapSize.x+x] == BOMB_SOLID;
}





























