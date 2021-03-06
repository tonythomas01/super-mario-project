////////////////////////////////////////////////////////////////////////
// BlockOccurrence.cpp
// Super Mario Project
// Copyright (C) 2011  
// Lionel Joseph lionel.r.joseph@gmail.com
// Olivier Guittonneau openmengine@gmail.com
////////////////////////////////////////////////////////////////////////

#include "CollisionManager.hpp"
#include "BlockOccurrence.hpp"
#include "Block.hpp"
#include "Tileset.hpp"
#include "Perso.hpp"

using namespace std;
using namespace sf;

namespace smp
{
	BlockOccurrence::BlockOccurrence(
		const string& textureName, 
		const Vector2f& position, 
		const Vector2f& speed,
		State state, 
		Side side,
		int physicIndex,
		Block* actual,
		Block* alternative)
		: EntityMovable(textureName, position, speed, side),
		_state(state),
		_animation(NB_STATES)
    {
		/* initialize pointer to models */
		_actualBlock = actual;
		_alternativeBlock = alternative;

		_hitboxSize.x = _texture->getSize().x / _actualBlock->getTileset()->getNbSprites().x;
		_hitboxSize.y = _texture->getSize().y / _actualBlock->getTileset()->getNbSprites().y;
		_hitboxPosition = Vector2f(position.x, position.y);

		Vector2i nbSprites = _actualBlock->getTileset()->getNbSprites();

		/* Compute blocs coords into the texture */
		_size.y = (_texture->getSize().y / nbSprites.y);
		_size.x = (_texture->getSize().x / nbSprites.x);

		_coordSprite.y = (physicIndex / nbSprites.x) * _size.y;
		_coordSprite.x = (physicIndex % nbSprites.x) * _size.x;

		/* Animation */
		if(_actualBlock->getTileset()->getFrameDelay() > 0)
		{
			/* So, we have a special block */
			_animation.addNbSpritesForGivenState(state, nbSprites.x);
			_animation.addFrameDelayForGivenState(state, _actualBlock->getTileset()->getFrameDelay());
			_animation.setCurrentState(state);
		}
		else
		{
			_animation.addNbSpritesForGivenState(state, 1);
		}
    }

	Block* BlockOccurrence::getActualModel()
	{
		return _actualBlock;
	}

	void BlockOccurrence::onCollision(Collisionable* c, int collision_type)
	{
		/* Collisions vs Perso */
		Perso* perso = dynamic_cast<Perso*>(c);
		if(perso != NULL)
		{
			return onCollision(perso, collision_type);
		}
	}

	void BlockOccurrence::onCollision(Perso* , int collision_type)
	{
		CollisionManager::Type type = static_cast<CollisionManager::Type>(collision_type);

		if(type == CollisionManager::FROM_TOP && !(_actualBlock->getType() & BlocksConstants::BLOCK_TYPE_EMPTY))
		{
			/* Normally, according to block type,  we do a specific action but now, we have only BLOCK_TYPE_EMPTY types */
			_actualBlock->removeBlockOccurrence(this);
		}
	}

	void BlockOccurrence::updateGraphicData(RenderWindow& )
	{
		if(_isActive && _animation.getNbSpritesMax() > 1)
		{
			_animation.update();
		}
	}

	void BlockOccurrence::updatePhysicData(float, RenderWindow& app)
	{
		setActivity(app);

		if(_isActive)
		{
			_previousHitboxPosition = _hitboxPosition;
		}
	}
	
	void BlockOccurrence::render(RenderWindow& app)
	{
		if(_isActive)
		{
			if(_animation.getNbSpritesMax() > 1)
			{
				_animation.render(_texture, _hitboxPosition, _side == LEFT_SIDE);
			}
			else
			{
				_animation.render(_texture, _hitboxPosition, _coordSprite, _size);
			}
		}
	}

    BlockOccurrence::~BlockOccurrence()
    {
    }
} // namespace
