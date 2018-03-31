#include "Defs.h"
#include "p2Log.h"

#include "DynamicEntity.h"

#include "j1App.h"
#include "j1Render.h"
#include "j1Collision.h"
#include "j1Map.h"
#include "j1Pathfinding.h"
#include "j1EntityFactory.h"
#include "j1Input.h"
#include "j1Movement.h"
#include "j1PathManager.h"
#include "Goal.h"

#include "Brofiler\Brofiler.h"

DynamicEntity::DynamicEntity(fPoint pos, iPoint size, int currLife, uint maxLife, const UnitInfo& unitInfo) : Entity(pos, size, currLife, maxLife), unitInfo(unitInfo)
{
	// Movement
	/// UnitInfo
	if (this->unitInfo.currSpeed == 0.0f)
		this->unitInfo.currSpeed = this->unitInfo.maxSpeed;

	/// SingleUnit
	singleUnit = new SingleUnit(this, nullptr);
	App->movement->CreateGroupFromUnit(this);

	/// Walkability map
	navgraph = new Navgraph();
	navgraph->CreateNavgraph();

	/// PathPlanner
	pathPlanner = new PathPlanner(this, *navgraph);

	// Goals
	brain = new Goal_Think(this);
}

DynamicEntity::~DynamicEntity() 
{
	animation = nullptr;

	// Remove goals
	if (brain != nullptr)
		delete brain;
	brain = nullptr;

	// Remove Movement
	if (pathPlanner != nullptr)
		delete pathPlanner;
	pathPlanner = nullptr;

	if (navgraph != nullptr)
		delete navgraph;
	navgraph = nullptr;

	if (singleUnit != nullptr)
		delete singleUnit;
	singleUnit = nullptr;

	// Remove Attack
	target = nullptr;

	// Remove Colliders
	if (sightRadiusCollider != nullptr)
		sightRadiusCollider->isRemove = true;
	sightRadiusCollider = nullptr;

	if (attackRadiusCollider != nullptr)
		attackRadiusCollider->isRemove = true;
	attackRadiusCollider = nullptr;
}

void DynamicEntity::Move(float dt) {}

void DynamicEntity::Draw(SDL_Texture* sprites)
{
	if (animation != nullptr)
		App->render->Blit(sprites, pos.x, pos.y, &(animation->GetCurrentFrame()));

	if (isSelected)
		DebugDrawSelected();
}

void DynamicEntity::DebugDrawSelected() 
{
	const SDL_Rect entitySize = { pos.x, pos.y, size.x, size.y };
	App->render->DrawQuad(entitySize, 255, 255, 255, 255, false);
}

void DynamicEntity::OnCollision(ColliderGroup* c1, ColliderGroup* c2, CollisionState collisionState) {}

// -------------------------------------------------------------

Animation* DynamicEntity::GetAnimation() const 
{
	return animation;
}

Goal_Think* DynamicEntity::GetBrain() const
{
	return brain;
}

// State machine
void DynamicEntity::UnitStateMachine(float dt) {}

void DynamicEntity::SetUnitState(UnitState unitState)
{
	this->unitState = unitState;
}

UnitState DynamicEntity::GetUnitState() const
{
	return unitState;
}

// Movement
SingleUnit* DynamicEntity::GetSingleUnit() const 
{
	return singleUnit;
}

PathPlanner* DynamicEntity::GetPathPlanner() const 
{
	return pathPlanner;
}

float DynamicEntity::GetSpeed() const 
{
	return unitInfo.currSpeed;
}

uint DynamicEntity::GetPriority() const 
{
	return unitInfo.priority;
}

uint DynamicEntity::GetDamage() const 
{
	return unitInfo.damage;
}

// Animations
void DynamicEntity::LoadAnimationsSpeed() {}

void DynamicEntity::UpdateAnimationsSpeed(float dt) {}

bool DynamicEntity::ChangeAnimation() { return true; }

// Direction
void DynamicEntity::SetUnitDirection(UnitDirection unitDirection)
{
	switch (unitDirection) {

	case UnitDirection_NoDirection:

		direction.x = 0.0f;
		direction.y = 0.0f;
		break;

	case UnitDirection_Up:

		direction.x = 0.0f;
		direction.y = -1.0f;
		break;

	case UnitDirection_Down:

		direction.x = 0.0f;
		direction.y = 1.0f;
		break;

	case UnitDirection_Left:

		direction.x = -1.0f;
		direction.y = 0.0f;
		break;

	case UnitDirection_Right:

		direction.x = 1.0f;
		direction.y = 0.0f;
		break;

	case UnitDirection_UpLeft:

		direction.x = -1.0f;
		direction.y = -1.0f;
		break;

	case UnitDirection_UpRight:

		direction.x = 1.0f;
		direction.y = -1.0f;
		break;

	case UnitDirection_DownLeft:

		direction.x = -1.0f;
		direction.y = 1.0f;
		break;

	case UnitDirection_DownRight:

		direction.x = 1.0f;
		direction.y = 1.0f;
		break;
	}
}

UnitDirection DynamicEntity::GetUnitDirection() const
{
	if (direction.x > 0.0f) {

		if (direction.y > 0.0f)
			return UnitDirection_DownRight;

		else if (direction.y < 0.0f)
			return UnitDirection_UpRight;

		else
			return UnitDirection_Right;
	}
	else if (direction.x < 0.0f) {

		if (direction.y > 0.0f)
			return UnitDirection_DownLeft;

		else if (direction.y < 0.0f)
			return UnitDirection_UpLeft;

		else
			return UnitDirection_Left;
	}
	else {

		if (direction.y > 0.0f)
			return UnitDirection_Down;

		else if (direction.y < 0.0f)
			return UnitDirection_Up;

		else
			return UnitDirection_NoDirection;
	}

	return UnitDirection_NoDirection;
}

void DynamicEntity::SetUnitDirectionByValue(fPoint unitDirection)
{
	direction = unitDirection;
}

fPoint DynamicEntity::GetUnitDirectionByValue() const
{
	return direction;
}

// Selection color
void DynamicEntity::SetColor(SDL_Color color, string colorName)
{
	this->color = color;
	this->colorName = colorName;
}

SDL_Color DynamicEntity::GetColor() const
{
	return color;
}

string DynamicEntity::GetColorName() const
{
	return colorName;
}

// Collision
ColliderGroup* DynamicEntity::GetSightRadiusCollider() const
{
	return sightRadiusCollider;
}

ColliderGroup* DynamicEntity::GetAttackRadiusCollider() const 
{
	return attackRadiusCollider;
}

ColliderGroup* DynamicEntity::CreateRhombusCollider(ColliderType colliderType, uint radius)
{
	vector<Collider*> colliders;
	iPoint currTilePos = App->map->MapToWorld(singleUnit->currTile.x, singleUnit->currTile.y);

	int sign = 1;
	for (int y = -(int)radius + 1; y < (int)radius; ++y) {

		if (y == 0)
			sign *= -1;

		for (int x = (-sign * y) - (int)radius + 1; x < (int)radius + (sign * y); ++x) {

			SDL_Rect rect = { currTilePos.x + x * App->map->data.tile_width, currTilePos.y + y * App->map->data.tile_height, App->map->data.tile_width, App->map->data.tile_height };
			colliders.push_back(App->collision->CreateCollider(rect));
		}
	}
	
	return App->collision->CreateAndAddColliderGroup(colliders, colliderType, App->entities, this);
}

void DynamicEntity::UpdateRhombusColliderPos(ColliderGroup* collider, uint radius)
{
	vector<Collider*>::const_iterator it = collider->colliders.begin();

	while (it != collider->colliders.end()) {

		iPoint currTilePos = App->map->MapToWorld(singleUnit->currTile.x, singleUnit->currTile.y);

		int sign = 1;
		for (int y = -(int)radius + 1; y < (int)radius; ++y) {

			if (y == 0)
				sign *= -1;

			for (int x = (-sign * y) - (int)radius + 1; x < (int)radius + (sign * y); ++x) {

				iPoint definitivePos = { currTilePos.x + x * App->map->data.tile_width, currTilePos.y + y * App->map->data.tile_height };
				iPoint definitiveTile = App->map->WorldToMap(definitivePos.x, definitivePos.y);

				if (App->pathfinding->IsWalkable(definitiveTile)) {

					(*it)->isValid = true;

					SDL_Rect rect = { definitivePos.x, definitivePos.y, App->map->data.tile_width, App->map->data.tile_height };
					(*it)->SetPos(rect.x, rect.y);
				}
				else
					(*it)->isValid = false;

				it++;
			}
		}
	}
}

// Attack
Entity* DynamicEntity::GetTarget() const 
{
	return target;
}

void DynamicEntity::SetTarget(Entity* target) 
{
	this->target = target;
}

bool DynamicEntity::IsTargetPresent() const 
{
	if (target == nullptr)
		return false;

	if (target->GetCurrLife() <= 0)
		return false;

	return true;
}

bool DynamicEntity::IsSightSatisfied() const
{
	return isSightSatisfied;
}

bool DynamicEntity::IsAttackSatisfied() const
{
	return isSightSatisfied && isAttackSatisfied;
}

void DynamicEntity::SetHitting(bool isHitting) 
{
	this->isHitting = isHitting;
}

bool DynamicEntity::IsHitting() const 
{
	return isHitting;
}

// Player commands
bool DynamicEntity::SetCommand(UnitCommand unitCommand) 
{
	bool ret = false;

	if (this->unitCommand == UnitCommand_NoCommand) {

		this->unitCommand = unitCommand;
		ret = true;
	}

	return ret;
}