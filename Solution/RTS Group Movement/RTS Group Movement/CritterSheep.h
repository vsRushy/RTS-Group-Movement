#ifndef __CritterSheep_H__
#define __CritterSheep_H__

#include "DynamicEntity.h"

#include <list>
using namespace std;

struct Particle;

struct Collider;
struct ColliderGroup;

enum CollisionState;

struct CritterSheepInfo
{
	UnitInfo unitInfo;
	uint restoredHealth = 0;

	// Unit animations
	Animation up, down, left, right;
	Animation upLeft, upRight, downLeft, downRight;
	Animation deathUpLeft, deathUpRight, deathDownLeft, deathDownRight;
};

class CritterSheep :public DynamicEntity
{
public:

	CritterSheep(fPoint pos, iPoint size, int currLife, uint maxLife, const UnitInfo& unitInfo, const CritterSheepInfo& critterSheepInfo);
	~CritterSheep() {};
	void Move(float dt);
	void Draw(SDL_Texture* sprites);
	void DebugDrawSelected();
	void OnCollision(ColliderGroup* c1, ColliderGroup* c2, CollisionState collisionState);

	// State machine
	void UnitStateMachine(float dt);

	// Animations
	bool ChangeAnimation();

	// Paws
	void UpdatePaws();

private:

	CritterSheepInfo critterSheepInfo;

	// Paws particles
	iPoint lastPawTile = { -1,-1 };
	Particle* lastPaw = nullptr;
};

#endif //__CritterSheep_H__