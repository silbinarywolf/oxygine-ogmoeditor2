#include "OgmoEditor2.h"
#include <string>
using namespace oxygine;

OgmoProject* OgmoProject::load(const char* name, Resources* resource) {
	file::buffer bf;
    file::read(name, bf);

    pugi::xml_document doc;
    doc.load_buffer(&bf.data[0], bf.size());

	// Ensure it begins with root <project> tag
	auto project = doc.first_child();
	OX_ASSERT(project.name() != "project");

	OgmoProject* result = new OgmoProject;
	result->resource = resource;

	// Store Layer Definitions
	auto xmlLayersDefinitions = project.child("LayerDefinitions").children();
	auto layerDefinitions = &result->layerDefinitions;
	for (auto xmlIt = xmlLayersDefinitions.begin(); xmlIt != xmlLayersDefinitions.end(); ++xmlIt) {
		OgmoLayerDefinition it = {};
		auto type = xmlIt->attribute("xsi:type").as_string();
		if (strcmp(type, "TileLayerDefinition") == 0) {
			it.type = LAYER_DEFINITION_TILE;
		} else if (strcmp(type, "EntityLayerDefinition") == 0) {
			it.type = LAYER_DEFINITION_ENTITY;
		} else {
			OX_ASSERT("Invalid LayerDefinition xsi:type.");
		}
		it.name = xmlIt->child("Name").child_value();
		auto grid = xmlIt->child("Grid");
		it.gridWidth = atoi(grid.child("Width").child_value());
		it.gridHeight = atoi(grid.child("Height").child_value());
		auto scrollFactor = xmlIt->child("ScrollFactor");
		it.scrollFactorX = atoi(scrollFactor.child("X").child_value());
		it.scrollFactorY = atoi(scrollFactor.child("Y").child_value());
		layerDefinitions->push_back(it);
	}

	// Store Entity Definitions
	auto xmlEntityDefinitions = project.child("EntityDefinitions").children();
	auto entityDefinitions = &result->entityDefinitions;
	for (auto xmlIt = xmlEntityDefinitions.begin(); xmlIt != xmlEntityDefinitions.end(); ++xmlIt) {
		OgmoEntityDefinition it = {};
		it.name = xmlIt->attribute("Name").as_string();
		auto size = xmlIt->child("Size");
		it.sizeWidth = atoi(size.child("Width").child_value());
		it.sizeHeight = atoi(size.child("Height").child_value());
		auto origin = xmlIt->child("Origin");
		it.originX = atoi(origin.child("X").child_value());
		it.originY = atoi(origin.child("Y").child_value());
		entityDefinitions->push_back(it);
	}

	// Store Tilesets
	auto xmlTilesets = project.child("Tilesets").children();
	auto tilesets = &result->tilesets;
	for (auto xmlIt = xmlTilesets.begin(); xmlIt != xmlTilesets.end(); ++xmlIt) {
		OgmoTileset it = {};
		it.name = xmlIt->child("Name").child_value();
		it.filepath = xmlIt->child("FilePath").child_value();
		auto size = xmlIt->child("TileSize");
		it.tileWidth = atoi(size.child("Width").child_value());
		it.tileHeight = atoi(size.child("Height").child_value());
		it.tileSeperation = atoi(xmlIt->child("TileSep").child_value());
		tilesets->push_back(it);
	}

	return result;
}

void OgmoProject::loadLevelInto(OgmoLevel* result, const char* name) {
	assert(result != NULL);
	file::buffer bf;
    file::read(name, bf);

    pugi::xml_document doc;
    doc.load_buffer(&bf.data[0], bf.size());

	// Ensure it begins with root <level> tag
	auto levelTag = doc.first_child();
	OX_ASSERT(levelTag.name() != "level");

	result->roomWidth = levelTag.attribute("width").as_int();
	result->roomHeight = levelTag.attribute("height").as_int();
	result->project = this;

	auto levelChildren = levelTag.children();
	for (auto xmlIt = levelChildren.begin(); xmlIt != levelChildren.end(); ++xmlIt) {
		auto exportMode = xmlIt->attribute("exportMode").as_string();
		if (strcmp(exportMode, "") == 0)
		{
			OgmoEntityLayer entityLayer = {};
			auto entities = &entityLayer.entities;
			entities->reserve(10);

			// Entities
			auto xmlEntities = xmlIt->children();
			for (auto xmlIt = xmlEntities.begin(); xmlIt != xmlEntities.end(); ++xmlIt) {
				OgmoEntity it = {};
				it.name = xmlIt->name();
				auto attributes = xmlIt->attributes();
				for (auto attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt) {
					std::string name = attrIt->name();
					if (name == "id") {
						it.id = attrIt->as_int();
					} else if (name == "x") {
						it.x = attrIt->as_int();
					} else if (name == "y") {
						it.y = attrIt->as_int();
					} else {
						it.properties[name] = attrIt->value();
					}
				}
				entities->push_back(it);
			}
			result->entityLayers.push_back(entityLayer);
		}
		else if (strcmp(exportMode, "Bitstring") == 0) 
		{
			// Grid
			// Not implemented.
			assert(false);
		}
		else
		{
			// Tiles
			OX_ASSERT(strcmp(exportMode, "CSV") == 0);

			// todo(Jake): make this pull grid_x from the OgmoLayerDefinition from OgmoProject
			int grid_x = 32;
			int grid_y = 32;

			int room_width_grid = result->roomWidth / grid_x;
			int room_height_grid = result->roomHeight / grid_y;

			OgmoTileLayer tileLayer = {};
			tileLayer.name = xmlIt->name();
			{
				auto name = xmlIt->attribute("tileset").as_string();
				OgmoTileset* first = &tilesets[0];
				for (size_t i = 0; i < tilesets.size(); ++i) {
					auto it = &first[i];
					if (it->name == name) {
						tileLayer.tileset = it;
					}
				}
				assert(tileLayer.tileset != NULL);
			}
			{
				auto name = tileLayer.name;
				auto first = &layerDefinitions[0];
				for (size_t i = 0; i < layerDefinitions.size(); ++i) {
					auto it = &first[i];
					if (it->name == name) {
						tileLayer.definition = it;
					}
				}
				assert(tileLayer.definition != NULL);
			}
			std::vector<int>* tiles = &tileLayer.tiles;

			std::string csvData = xmlIt->child_value();
			tiles->reserve(room_width_grid * room_height_grid);
			int prevIndex = 0;
			size_t i = 0;
			for (; i < csvData.length(); ++i) 
			{
				if (csvData[i] == ',' || csvData[i] == '\n')
				{
					auto tileID = atoi(csvData.substr(prevIndex,i).c_str());
					tiles->push_back(tileID);
					prevIndex = i + 1;
				}
			}
			// Add last tile
			auto tileID = atoi(csvData.substr(prevIndex,i).c_str());
			tiles->push_back(tileID);
			result->tileLayers.push_back(tileLayer);
		}
	}
}

int OgmoLevelData::getTileAt(OgmoTileLayer* tileLayer, int x, int y) {
	int gridWidth = tileLayer->definition->gridWidth;
	int gridHeight = tileLayer->definition->gridHeight;
	size_t columns = roomWidth / gridWidth;
	size_t rows = roomHeight / gridHeight;

	int entityGridX = x / gridWidth;
	int entityGridY = y / gridHeight;

	size_t tileIndex = entityGridX + (entityGridY * columns);
	auto tiles = tileLayer->tiles;
	int tileID = OGMO_BLANK_TILE;
	if (tileIndex >= 0 && tileIndex < tiles.size()) {
		tileID = tiles.at(tileIndex);
	}
	return tileID;
}

void OgmoLevel::doRender(const RenderState& rs) {
	// todo(Jake): Allow rendering of OgmoLevel based on a camera
	// todo(Jake): Render each layer on a seperate Actor object so 'setPriority()' can be setup.
	// http://gamedev.stackexchange.com/questions/32140/effecient-tilemap-rendering
	auto tileLayers = this->tileLayers;
	Sprite sprite;
	for (size_t layerIndex = 0; layerIndex < tileLayers.size(); ++layerIndex) {
		auto tileLayer = &tileLayers.at(layerIndex);

		// todo(Jake): At some point, ensure that doing this every frame isn't too costly.
		sprite.setResAnim(project->resource->getResAnim(tileLayer->tileset->name));

		int gridX = tileLayer->definition->gridWidth;
		int gridY = tileLayer->definition->gridHeight;

		size_t columns = roomWidth / gridX;
		size_t rows = roomHeight / gridY;

		auto tiles = &tileLayer->tiles;
		size_t tilesSize = tiles->size();
		
		size_t i = 0;
		float yy = 0;
		for (size_t y = 0; y < rows; ++y) {
			float xx = 0;
			for (size_t x = 0; x < columns; ++x) {
				if (tiles->at(i) != OGMO_BLANK_TILE) {
					sprite.setPosition(xx, yy);
					sprite.render(rs);
				}
				xx += 32;
				++i;
			}
			yy += 32;
		}
	}
}