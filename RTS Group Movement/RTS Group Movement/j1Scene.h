#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"

#include <vector>
#include <string>
using namespace std;

struct SDL_Texture;

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Save
	bool Save(pugi::xml_node&) const;

	// Load
	bool Load(pugi::xml_node&);

public:

	// Camera
	float up = false, down = false, left = false, right = false;
	uint width = 0;
	uint height = 0;
	uint scale = 0;

private:

	// Map
	string warcraftMap, warcraftTexName;
	bool warcraftActive;
	SDL_Texture* debugTex = nullptr;

	// Draw rectangle
	iPoint startRectangle = { 0,0 };

public:

	bool debugDrawMovement = true;
	bool debugDrawPath = false;
	bool debugDrawMap = false;
};

#endif //__j1SCENE1_H__