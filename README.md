# Oxygine Ogmo Editor 2

Tools for loading Ogmo Editor 2 levels in the Oxygine Framework
[http://www.ogmoeditor.com](http://www.ogmoeditor.com/)

NOTE: This is very early in development. If any of this can be improved to work with the Oxygine engine in a better way, please contribute!

# Quick Start / Example Code

Load files
```
// Your load resources code
resources.loadXML("res.xml");

// Load Ogmo Project
auto project = OgmoProject::load("rooms/project.oep", &resources);

// Load level
auto level = new OgmoLevel; // Loaded like this to allow for subclassing (ie. make 'GameLevel' class that inherits OgmoLevel)
project->loadLevelInto(level, "rooms/mylevel.oel");
level->attachTo(this); // Assumes 'this' context is your 'Stage' object, whereever you want the level to render on.
```

Example of creating entities in subclass of OgmoLevel
```
void GameLevel::init() {
	// NOTE: entityLayers belongs to OgmoLevel
	for (size_t l = 0; l < entityLayers.size(); ++l) {
		auto entityLayer = entityLayers.at(l);
		for (size_t i = 0; i < entityLayer.entities.size(); ++i) {
			auto entityInfo = entityLayer.entities.at(i);

			spActor entity = NULL;
			if (entityName == "Crate") {
				entity = new Crate;
			} else if (entityName == "Player") {
				entity = new Player;
			} else {
				OX_ASSERT(!"Undefined entity.");
			}
			assert(entity != NULL);
		}
	}
}
```

Check for collision on a specific tile layer
```
bool GameLevel::isTileCollisionAt(float xx, float yy) {
	bool foundCollision = false;
	int x = (int)xx;
	int y = (int)yy;
	for (size_t i = 0; i < tileLayers.size(); ++i) {
		OgmoTileLayer* tileLayer = &tileLayers[i];
		if (tileLayer->name != "Walls") {
			continue;
		}
		int tileID = getTileAt(tileLayer, x, y);
		foundCollision = foundCollision || (tileID != OGMO_BLANK_TILE);
	}
	return foundCollision;
}
```