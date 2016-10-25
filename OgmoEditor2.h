#pragma once

#include <vector>
#include <map>

#include "oxygine-framework.h"
using namespace oxygine;

#define OGMO_OUT_OF_BOUNDS_TILE -2
#define OGMO_BLANK_TILE -1

enum OgmoLayerDefinitionType {
	LAYER_DEFINITION_UNKNOWN = 0,
	LAYER_DEFINITION_TILE	 = 1,
	LAYER_DEFINITION_ENTITY	 = 2,
	//LAYER_DEFINITION_GRID    = 3, // Not implemented
};

class OgmoTileset {
public:
	std::string name;
	std::string filepath;
	int tileWidth;
	int tileHeight;
	int tileSeperation;
};

class OgmoLayerDefinition {
public:
	OgmoLayerDefinitionType type;
	std::string name;
	int gridWidth;
	int gridHeight;
	int scrollFactorX;
	int scrollFactorY;
};

class OgmoEntityDefinition {
public:
	std::string name;
	int sizeWidth;
	int sizeHeight;
	int originX;
	int originY;
};

class OgmoTileLayer {
public:
	std::string name;
	OgmoTileset* tileset;
	OgmoLayerDefinition* definition;
	std::vector<int> tiles;
};

class OgmoEntity {
public:
	std::string name;
	int id;
	int x;
	int y;
	std::map<std::string, std::string> properties;
};

class OgmoEntityLayer {
public:
	std::vector<OgmoEntity> entities;
};

class OgmoProject;

class OgmoLevelData {
public:
	OgmoLevelData() : roomWidth(0),roomHeight(0),project(0) {};
	int roomWidth;
	int roomHeight;
	OgmoProject* project;
	std::vector<OgmoTileLayer> tileLayers;
	std::vector<OgmoEntityLayer> entityLayers;
	int getTileAt(OgmoTileLayer* layer, int x, int y);
	int getTileAt(OgmoTileLayer* layer, Vector2& position) { return getTileAt(layer, (int)position.x, (int)position.y); }
	bool isOutOfBounds(int x, int y) { return (x < 0 || y < 0 || x >= roomWidth || y >= roomHeight); }
	bool isOutOfBounds(Vector2& position) { return isOutOfBounds((int)position.x, (int)position.y); }
};

class OgmoLevel : public Actor, public OgmoLevelData {
public:
	void setSize(float w, float h) { Actor::setSize(w, h); roomWidth = (int)w; roomHeight = (int)h; }
	void setSize(int w, int h) { Actor::setSize((float)w, (float)h); roomWidth = w; roomHeight = h; }
	void doRender(const RenderState& rs);
};

class OgmoProject {
public:
	static OgmoProject* load(const char* name, Resources* resource);
	void loadLevelInto(OgmoLevel* level, const char* name);
	Resources* resource;
protected:
	std::vector<OgmoLayerDefinition> layerDefinitions;
	std::vector<OgmoEntityDefinition> entityDefinitions;
	std::vector<OgmoTileset> tilesets;
};