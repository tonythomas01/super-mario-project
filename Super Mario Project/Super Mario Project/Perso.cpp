////////////////////////////////////////////////////////////////////////
// Perso.cpp
// Super Mario Project
// Copyright (C) 2011  
// Lionel Joseph lionel.r.joseph@gmail.com
// Olivier Guittonneau openmengine@gmail.com
////////////////////////////////////////////////////////////////////////

#include "CollisionManager.hpp"
#include "Perso.hpp"
#include "Monster.hpp"
#include "Block.hpp"
#include "Checkpoint.hpp"
#include "ProjectileOccurrence.hpp"
#include "Pipe.hpp"
#include "Item.hpp"
#include "ResourceManager.hpp"
#include "HUD.hpp"
#include "InputState.hpp"
#include "Finish.hpp"
#include "XMLPersoParser.hpp"
#include <fstream>
#include <sstream>

using namespace std;
using sf::Vector2f;
using sf::RenderWindow;

namespace smp
{
	Perso::Perso(const string& textureName, const Vector2f& position) : EntityMovable("textures/persos/" + textureName, position),
		_environment(GROUND), 
		_transformation(SMALL_MARIO), 
		_state(STANDING),
		_hud(new HUD()),
		_canClimb(false), 
		_acceleration(Vector2f()), 
		_animation(Animation<State>(NB_STATES)),
		_broughtMonster(nullptr)
	{
		initPerso(textureName);
		_invincibleStarTime.Reset(true);
		_invincibleTime.Reset(true);

		/* Setting animation Data */
		_animation.setCurrentState(_state);
	}

	void Perso::initPerso(const string& textureName)
	{
		_textureName = "textures/persos/" + textureName;
		_texture = ResourceManager::getTexture(_textureName);

		/* Fill Data for texture */
		_animation.reset();
		loadPerso();
	}

	HUD* Perso::getHUD()
	{
		return _hud;
	}

	Perso::State Perso::getState()
	{
		return _state;
	}

	Perso::Environment Perso::getEnvironment()
	{
		return _environment;
	}

	Perso::Transformation Perso::getTransformation()
	{
		return _transformation;
	}

	bool Perso::getCanClimb()
	{
		return _canClimb;
	}

	Vector2f& Perso::getAcceleration()
	{
		return _acceleration;
	}

	MonsterOccurrence* Perso::getBroughtMonster()
	{
		return _broughtMonster;
	}

	Pipe* Perso::getInsidePipe()
	{
		return _insidePipe;
	}

	Checkpoint* Perso::getCheckPointPassed()
	{
		return _checkpointPassed;
	}

	Animation<Perso::State>& Perso::getAnimation()
	{
		return _animation;
	}

	void Perso::setState(Perso::State state)
	{
		_animation.setCurrentState(state);
		_state = state;
	}

	void Perso::setEnvironment(const Environment &environment)
	{
		_environment = environment;
	}

	void Perso::setTransformation(const Transformation &transformation)
	{
		_transformation = transformation;
	}

	void Perso::setCanClimb(bool canClimb)
	{
		_canClimb = canClimb;
	}

	void Perso::setBroughtMonster(MonsterOccurrence* monster)
	{
		_broughtMonster = monster;
	}

	void Perso::setInsidePipe(Pipe* pipe)
	{
		_insidePipe = pipe;
	}

	void Perso::updateGraphicData(RenderWindow&)
	{
		/* Update animation */
		_animation.update();
	}

	void Perso::updatePhysicData(float, RenderWindow&)
	{
		// Nothing because we need InputState
	}

	void Perso::updatePerso(float time)
	{
		InputState& inputState = *(InputState::getInput());

		/* Applying gravity */
		if(_environment != PIPE	&& _state != CLIMB_LADDER)
			gravity(_speed, time);

		/* compute acceleration */
		solve_acc(inputState);

		/* Lateral movements management */
		if(_state != PUSH_SHELL)
			lateral_move(time, inputState);

		/* Test for jump state */
		if(inputState[KEY_JUMP] == KEY_STATE_JUST_PRESSED && inputState[KEY_UP] == KEY_STATE_RELEASED
		&& (_environment == GROUND || _state == CLIMB_LADDER))
			jump();

		/* jump continue if key is always pressed */
		/*if(_state == JUMP || _state == JUMP_SHELL)
        {			
			if(inputState[KEY_JUMP] == KEY_STATE_PRESSED)
			{
				if((_jumpTime.GetElapsedTime() < 300 && (_speed.x >= RUN_SPEED || _speed.x <= -RUN_SPEED)) ||
					(_jumpTime.GetElapsedTime() < 175))
					_speed.y = JUMP_SPEED;
			}
        }

		if(_speed.y != 0)
			_environment = AIR;*/

		if(_speed.y < 0 && (_state == JUMP || _state == ATTACK))
			setState(JUMP_FALLING);

		/* Test for states : LOOK_TOP_* */
		if(_broughtMonster == nullptr)
		{
			if(inputState[KEY_UP] == KEY_STATE_PRESSED
			&& _environment == GROUND
				&& _state == STANDING)
				setState(LOOK_TOP);
			else if(inputState[KEY_UP] == KEY_STATE_PRESSED
				&& inputState[KEY_DOWN] == KEY_STATE_PRESSED)
				setState(STANDING);
		}
		else
		{
			if(inputState[KEY_UP] == KEY_STATE_PRESSED
				&& _environment == GROUND
				&& _state == STANDING_SHELL)
				setState(LOOK_TOP_SHELL);
			else if(inputState[KEY_UP] == KEY_STATE_PRESSED
				&& inputState[KEY_DOWN] == KEY_STATE_PRESSED)
				setState(STANDING_SHELL);
		}

		/* Test for states : LOWERED_* */
		if(_broughtMonster == nullptr)
		{
			if(inputState[KEY_JUMP] == KEY_STATE_PRESSED
				&& inputState[KEY_DOWN] == KEY_STATE_JUST_PRESSED
				&& _environment == GROUND)
				setState(LOWERED_JUMP);
			else if(inputState[KEY_DOWN]
			&& _environment == GROUND &&
				(_state == STANDING || _state == WALK
				|| _state == RUN_1 || _state == RUN_2
				|| _state == SKID))
				setState(LOWERED);
		}
		else
		{
			if(inputState[KEY_JUMP] == KEY_STATE_PRESSED
				&& inputState[KEY_DOWN] == KEY_STATE_PRESSED
				&& _environment == GROUND)
				setState(LOWERED_JUMP_SHELL);
			else if(inputState[KEY_DOWN] == KEY_STATE_PRESSED
				&& _environment == GROUND)
				setState(LOWERED_SHELL);
		}

		/* Save actual position as previous position */
		_previousHitboxPosition = _hitboxPosition;
		_previousPosition = _position;

		updatePositions(_hitboxPosition.x + time * _speed.x, _hitboxPosition.y + time * _speed.y);
		
		/* Special Cases */
		if(_hitboxPosition.y < 0)
		{
			updatePositions(_hitboxPosition.x, 0);
			_environment = GROUND;
		}

		if(_hitboxPosition.x < 0)
		{
			updatePositions(0, _hitboxPosition.y);
		}
	}

	void Perso::solve_acc(InputState& inputState)
	{
		float coeff;

		/* Frottements sont différents selon l'environnement 
		dans le lequel se trouve le personnage */
		if(_environment == GROUND)
			coeff = PhysicConstants::CLASSIC_COEFF_FRICTION;
		else
			coeff = PhysicConstants::AIR_COEFF_FRICTION;

		/* Modification de l'accélèration en fonction de l'appui
		ou non sur la touche d'accélèration */
		if(_state != GO_TO_CASTLE && _state != CLIMB_LADDER)
		{
			if(inputState[KEY_RUN] == KEY_STATE_PRESSED)
				_acceleration.x = PhysicConstants::RUN_ACCEL * coeff;
			else
				_acceleration.x = PhysicConstants::WALK_ACCEL * coeff;
		}
		else
		{
			_acceleration.x = 20 * PhysicConstants::RUN_ACCEL * coeff;
		}
	}

	void Perso::jump()
	{
		/* key just pressed, clock begins */
		_jumpTime.Reset(true);

		_speed.y = PhysicConstants::JUMP_SPEED;

		// Play jump sound here !

		if(_environment == GROUND)
			_environment = AIR;

		if(_broughtMonster == nullptr)
		{
			if(_state == LOWERED)
				setState(LOWERED_JUMP);
			else
				setState(JUMP);
		}
		else
		{
			if(_state == LOWERED_SHELL)
				setState(LOWERED_JUMP_SHELL);
			else
				setState(JUMP_SHELL);
		}
	}

	void Perso::render(RenderWindow& app)
	{
		Vector2f spritePosition = Vector2f(_hitboxPosition.x - (_side == LEFT_SIDE) * _hitboxSize.x, _hitboxPosition.y);
		_animation.render(_texture, spritePosition, _side == LEFT_SIDE);

#ifdef _RELEASE
		/* Drawing HitBox */
		sf::RectangleShape rect = sf::RectangleShape(sf::Vector2f(_hitboxSize.x, _hitboxSize.y));
		rect.setPosition(spritePosition);
		rect.setFillColor(sf::Color(0, 255, 0, 122));

		app.pushGLStates();
		app.draw(rect);
		app.popGLStates();
#endif
	}

	void Perso::lateral_move(float time, InputState& inputState)
	{		
		if(_state != GO_TO_CASTLE)
		{
			if(_state != FINISH)
			{
				if(inputState[KEY_BACKWARD] == KEY_STATE_PRESSED)
					_side = LEFT_SIDE;
				else
					_side = RIGHT_SIDE;

				if(inputState[KEY_FORWARD] == KEY_STATE_PRESSED)
				{
					if(inputState[KEY_DOWN] == KEY_STATE_RELEASED)
					{
						if(_speed.x < 0)
						{
							if(_environment != AIR && _broughtMonster == nullptr && _state != SKID)
							{
								setState(SKID);
								// play a sound here !
							}
							frictions(time);
						}
						else
						{
							if(_broughtMonster == nullptr)
							{
								if(_environment == GROUND)
								{
									if(_state != CLIMB_LADDER)
									{
										if(inputState[KEY_RUN] == KEY_STATE_PRESSED)
										{
											setState(RUN_1);
										}
										else
										{
											setState(WALK);
										}
										_hud->setNbMonstersKilled(0);
									}
								}
							}
							else
							{
								if(_environment == GROUND)
								{
									setState(WALK_SHELL);
								}
							}
						}
						_speed.x = _speed.x + _acceleration.x * time;
					}
				}
				else if(inputState[KEY_BACKWARD] == KEY_STATE_PRESSED)
				{
					if(inputState[KEY_DOWN] == KEY_STATE_RELEASED)
					{
						if(_speed.x > 0)
						{
							if(_environment != AIR && _broughtMonster == nullptr && _state != SKID)
							{
								setState(SKID);
								// play a sound here !
							}
							frictions(time);
						}
						else
						{
							if(_broughtMonster == nullptr)
							{
								if(_environment == GROUND)
								{
									if(_state != CLIMB_LADDER)
									{
										if(inputState[KEY_RUN] == KEY_STATE_PRESSED)
										{
											setState(RUN_1);
										}
										else
										{
											setState(WALK);
										}
										_hud->setNbMonstersKilled(0);
									}
								}
							}
							else
							{
								if(_environment == GROUND)
								{
									setState(WALK_SHELL);
								}
							}
						}
						_speed.x = _speed.x - _acceleration.x * time;
					}
				}
				else// if(_specialAttackTime. && !_attackTime)
				{

					if(_broughtMonster == nullptr)
					{
						if(_state == CLIMB_LADDER)
						{
							if(inputState[KEY_DOWN] == KEY_STATE_PRESSED)
								_speed.y = _speed.y - _acceleration.y * time;
							else if(inputState[KEY_UP] == KEY_STATE_PRESSED)
								_speed.y = _speed.y + _acceleration.y * time;
							else
								_speed.y = 0;
						}
						else
						{
							if(_environment == AIR)
							{
								if(inputState[KEY_BACKWARD] == KEY_STATE_PRESSED)
									setState(LOWERED_JUMP);
								//else
								//	setState(STANDING);
							}
							else if(_environment == GROUND)
							{
								setState(STANDING);
								_hud->setNbMonstersKilled(0);
							}
						}
					}
					else
					{
						if(_environment == AIR)
						{
							if(inputState[KEY_BACKWARD] == KEY_STATE_PRESSED)
							setState(LOWERED_JUMP_SHELL);
						}
						else if(_environment == GROUND)
						{
							setState(STANDING_SHELL);
							_hud->setNbMonstersKilled(0);
						}
					}
				}
			}

			if((inputState[KEY_FORWARD] == KEY_STATE_RELEASED && inputState[KEY_BACKWARD] == KEY_STATE_RELEASED)
				|| (inputState[KEY_DOWN] == KEY_STATE_PRESSED && _environment == GROUND))
				frictions(time);
		}
		else
		{
			/* Character walks to castle without control on him */
			_speed.x = _acceleration.x * time;
		}
	}

	void Perso::hurted()
	{
		switch(_transformation)
		{
		case FIRE_MARIO:
			transform(SUPER_MARIO);
			/*p->tps_invincible = 2000;
			p->tps_transformation = 1000;
			FSOUND_PlaySound(FSOUND_FREE, p->sons[SND_TOUCHE]);*/
			break;

		case SUPER_MARIO:
			transform(SMALL_MARIO);
			/*p->tps_invincible = 2000;
			p->tps_transformation = 1000;
			FSOUND_PlaySound(FSOUND_FREE, p->sons[SND_TOUCHE]);*/
			break;

		default:
			setState(DEAD);
			_speed.x = 0;
			_speed.y = PhysicConstants::EJECTION_SPEED_Y * 5;
			/*p->tps_mort = TPS_MORT;
			FSOUND_PlaySound(FSOUND_FREE, p->sons[SND_DIE]);
			FSOUND_Stream_Stop(n->musique);*/
		}
	}

	void Perso::transform(Transformation nextTransformation)
	{
		if(nextTransformation > _transformation)
		{
			/* According to actual transformation,
			we load the desired texture */
			switch(nextTransformation) {
			case SMALL_MARIO :
				initPerso("small_mario");
				_transformation = SMALL_MARIO;
				break;
			case SUPER_MARIO :
				initPerso("super_mario");
				_transformation = SUPER_MARIO;
				break;
			case FIRE_MARIO :
				initPerso("fire_mario");
				_transformation = FIRE_MARIO;
				break;
			case ICE_MARIO :
				initPerso("ice_mario");
				_transformation = ICE_MARIO;
				break;
			default : break;
			}
		}
		_hud->addPoints(1000);
	}

	void Perso::onCollision(Collisionable* c, int collision_type)
	{
		if(_environment != PIPE)
		{
			/* Collision with a BlockOccurrence */
			BlockOccurrence* block = dynamic_cast<BlockOccurrence*>(c);
			if(block != NULL)
			{
				return onCollision(block, collision_type);
			}

			/* Collision with an Item */
			ItemOccurrence* itemOccurrence = dynamic_cast<ItemOccurrence*>(c);
			if(itemOccurrence != NULL)
			{
				return onCollision(itemOccurrence);
			}

			/* Collision with a Projectile */
			ProjectileOccurrence* projectileOccurrence = dynamic_cast<ProjectileOccurrence*>(c);
			if(projectileOccurrence != NULL)
			{
				return onCollision(projectileOccurrence);
			}

			/* Collision with a Pipe */
			Pipe* pipe = dynamic_cast<Pipe*>(c);
			if(pipe != NULL)
			{
				return onCollision(pipe, collision_type);
			}

			/* Collision with a MonsterOccurrence */
			MonsterOccurrence* monsterOccurrence = dynamic_cast<MonsterOccurrence*>(c);
			if(monsterOccurrence != NULL)
			{
				return onCollision(monsterOccurrence, collision_type);
			}

			/* Collision with Checkpoint */
			Checkpoint* checkpoint = dynamic_cast<Checkpoint*>(c);
			if(checkpoint != NULL)
			{
				checkpoint->setState(Checkpoint::PASSED);
				return;
			}

			/* Collision with Finish */
			Finish* finish = dynamic_cast<Finish*>(c);
			if(finish != NULL)
			{
				if(_hitboxPosition.x + _hitboxSize.x >= finish->getHitboxPosition().x + finish->getHitboxSize().x / 2)
				{
					setState(FINISH);
					_speed.x = 0;
				}
				else
				{
					setState(GO_TO_CASTLE);
					finish->setState(Finish::FINISH);
				}
			}
		}
	}

	void Perso::onCollision(Pipe* pipe, int collision_type)
	{
		CollisionManager::Type type = static_cast<CollisionManager::Type>(collision_type);

		if(type == CollisionManager::FROM_LEFT)
		{
			updatePositions(pipe->getHitboxPosition().x + pipe->getHitboxSize().x, _hitboxPosition.y);
		}
		else if(type == CollisionManager::FROM_RIGHT)
		{
			updatePositions(pipe->getHitboxPosition().x - _hitboxSize.x - _deltaX, _hitboxPosition.y);
		}
		else if(type == CollisionManager::FROM_BOTTOM)
		{
			updatePositions(_hitboxPosition.x, pipe->getHitboxPosition().y + pipe->getHitboxSize().y);
			_environment = GROUND;
		}
		else if(type == CollisionManager::FROM_TOP)
		{
			updatePositions(_hitboxPosition.x, pipe->getHitboxPosition().y - _hitboxSize.y);
			_speed.y = 0;
		}
	}

	void Perso::onCollision(ProjectileOccurrence* projectileOccurrence)
	{
		if(projectileOccurrence->getSender() == ProjectileOccurrence::VILAIN)
		{
			hurted();
		}
	}

	void Perso::onCollision(MonsterOccurrence* monsterOccurrence, int collision_type)
	{
		InputState& input = *InputState::getInput();

		CollisionManager::Type type = static_cast<CollisionManager::Type>(collision_type);

		Monster* monster = monsterOccurrence->getModel();
		if((type == CollisionManager::FROM_LEFT || type == CollisionManager::FROM_RIGHT)
			&& _invincibleStarTime.GetElapsedTime() == 0 && _invincibleTime.GetElapsedTime() == 0)
			hurted();

		if(type == CollisionManager::FROM_BOTTOM && monster->checkFeature(MonsterConstants::CAN_BE_KILL_BY_JUMP))
			if(input[KEY_JUMP] == KEY_STATE_PRESSED && input[KEY_UP] == KEY_STATE_RELEASED)
				jump();

		if(type == CollisionManager::FROM_BOTTOM && !monster->checkFeature(MonsterConstants::CAN_BE_KILL_BY_JUMP))
			hurted();
	}

	void Perso::onCollision(ItemOccurrence* itemOccurrence)
	{
		Item* item = itemOccurrence->getModel();
		switch(item->getType())
		{
		case Item::COIN:
			_hud->addCoin();
			break;

		case Item::MUSHROOM:
			transform(SUPER_MARIO);
			break;

		case Item::FLOWER:
			transform(FIRE_MARIO);
			break;

		case Item::ICE_FLOWER:
			transform(ICE_MARIO);
			break;

		case Item::MINI_MUSHROOM:
			transform(MINI_MARIO);
			break;

		case Item::POISON_MUSHROOM:
			hurted();
			break;

		case Item::STAR:
			_invincibleStarTime.Reset();
			break;

		case Item::LIFE_MUSHROOM:
			_hud->addLife();
			break;

		default:
			break;
		}
	}

	void Perso::onCollision(BlockOccurrence* block, int collision_type)
	{
		CollisionManager::Type type = static_cast<CollisionManager::Type>(collision_type);

		if(type == CollisionManager::FROM_RIGHT && (block->getActualModel()->getPhysic() & BlocksConstants::LEFT_WALL))
		{
			float hitboxPositionBlockX = block->getHitboxPosition().x;
			int hitboxSizeBlockX = block->getHitboxSize().x;
			updatePositions(hitboxPositionBlockX - hitboxSizeBlockX, _hitboxPosition.y);
		}

		if(type == CollisionManager::FROM_TOP && (block->getActualModel()->getPhysic() & BlocksConstants::ROOF))
		{
			updatePositions(_hitboxPosition.x, block->getHitboxPosition().y);
			_speed.y = 0;
		}

		if(type == CollisionManager::FROM_LEFT && (block->getActualModel()->getPhysic() & BlocksConstants::RIGHT_WALL))
		{
			updatePositions(block->getHitboxPosition().x + block->getHitboxSize().x, _hitboxPosition.y);
		}

		if(type == CollisionManager::FROM_BOTTOM && (block->getActualModel()->getPhysic() & BlocksConstants::GROUND))
		{
			float hitboxPositionBlockY = block->getHitboxPosition().y;
			int hitboxSizeBlockY = block->getHitboxSize().y;
			updatePositions(_hitboxPosition.x, hitboxPositionBlockY + hitboxSizeBlockY);
			_environment = GROUND;
		}
	}

	void Perso::frictions(float time)
	{
		float coeff;

		/* Frictions are different according to environment*/
		if(_environment == GROUND)
			coeff = PhysicConstants::CLASSIC_COEFF_FRICTION;
		else
			coeff = PhysicConstants::AIR_COEFF_FRICTION;

		if(time != 0)
		{
			_speed.x /= 1 + 5 * coeff * time * PhysicConstants::SLIDE_COEFF_FRICTION;
		}

		/* To avoid character moving suddenly of one pixel after a moment of immobility */
		if(_speed.x < SPEED_X_MIN && _speed.x > -SPEED_X_MIN)
			_speed.x = 0;
	}

	void Perso::updateInPipe()
	{
		switch(_insidePipe->getDirection())
		{
		case Pipe::TO_TOP:
			setState(GET_OUT_FROM_PIPE_VERTICAL);
			_hitboxPosition.x = _insidePipe->getPosition().x * BLOCK_WIDTH + BLOCK_WIDTH - _hitboxSize.x / 2;
			_hitboxPosition.y = _insidePipe->getPosition().y * BLOCK_WIDTH + (_insidePipe->getLenght() + 1) * BLOCK_WIDTH - _hitboxSize.y;
			break;

		case Pipe::TO_BOTTOM:
			setState(GET_OUT_FROM_PIPE_VERTICAL);
			_hitboxPosition.x = _insidePipe->getPosition().x * BLOCK_WIDTH + BLOCK_WIDTH - _hitboxSize.x / 2;
			_hitboxPosition.y = _insidePipe->getPosition().y * BLOCK_WIDTH;
			break;

		case Pipe::TO_LEFT:
			_side = RIGHT_SIDE;
			_state = GET_OUT_FROM_PIPE_HORIZONTAL;
			_hitboxPosition.x = (_insidePipe->getPosition().x + _insidePipe->getLenght()) * BLOCK_WIDTH, 
				_hitboxPosition.y = _insidePipe->getPosition().y * BLOCK_WIDTH;
			break;

		case Pipe::TO_RIGHT:
			_side = LEFT_SIDE;
			_state = GET_OUT_FROM_PIPE_HORIZONTAL;
			_hitboxPosition.x = _insidePipe->getPosition().x * BLOCK_WIDTH;
			_hitboxPosition.y = _insidePipe->getPosition().y * BLOCK_WIDTH;
			break;

		default :
			break;
		}
		// Play sound pipe here !
	}

	void Perso::loadPerso()
	{
		XMLPersoParser::getParser()->loadPerso(_texture->name() + ".xml", this);

		/* Compute Hitbox Size */
		_hitboxSize.x = _texture->getSize().x / _animation.getNbSpritesMax() - 2 * _deltaX;
		_hitboxSize.y = _texture->getSize().y / _animation.getNbStates();
		_hitboxSize.y -= _deltaY;
	}

	Perso::~Perso()
	{
		delete _hud;
	}
} // namespace
