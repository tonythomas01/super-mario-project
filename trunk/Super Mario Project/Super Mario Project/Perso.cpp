////////////////////////////////////////////////////////////////////////
// Perso.cpp
// Super Mario Project
// Copyright (C) 2011  
// Lionel Joseph lionel.r.joseph@gmail.com
// Olivier Guittonneau openmengine@gmail.com
////////////////////////////////////////////////////////////////////////

#include "Perso.hpp"
#include "InputState.hpp"
#include <fstream>
#include <sstream>

using namespace SuperMarioProject;

namespace Collisions
{
	Perso::Perso(const string& textureName, Vector2f& position) : EntityMovable(textureName, position), 
		_environment(GROUND), 
		_transformation(SMALL_MARIO), 
		_state(STANDING), 
		_hud(new HUD()), 
		_canClimb(false), 
		_acceleration(Vector2f()), 
		_invincibleTime(0), 
		_invincibleStarTime(0),
		_transformationTime(0), 
		_attackTime(0), 
		_specialAttackTime(0), 
		_throwShellTime(0), 
		_deathTime(0), 
		_finishTime(0),
		_jumpTime(0)
	{
		loadPerso(textureName);
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

	Perso::Transformations Perso::getTransformation()
	{
		return _transformation;
	}

	bool Perso::getCanClimb()
	{
		return _canClimb;
	}

	Vector2f Perso::getAcceleration()
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

	int Perso::getInvincibleTime()
	{
		return _invincibleTime;
	}

	int Perso::getInvincibleStarTime()
	{
		return _invincibleStarTime;
	}

	int Perso::getTransformationTime()
	{
		return _transformationTime;
	}

	int Perso::getAttackTime()
	{
		return _attackTime;
	}

	int Perso::getSpecialAttackTime()
	{
		return _specialAttackTime;
	}

	int Perso::getThrowShellTime()
	{
		return _throwShellTime;
	}

	int Perso::getDeathTime()
	{
		return _deathTime;
	}

	int Perso::getFinishTime()
	{
		return _finishTime;
	}

	int Perso::getJumpTime()
	{
		return _jumpTime;
	}

	void Perso::setState(Perso::State state)
	{
		_state = state;
	}

	void Perso::setEnvironment(const Environment &environment)
	{
		_environment = environment;
	}

	void Perso::setTransformation(const Transformations &transformation)
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

	void Perso::setCheckPointPassed(Checkpoint* checkpoint)
	{
		_checkpointPassed = checkpoint;
	}

	void Perso::setInvincibleTime(int invincibleTime)
	{
		_invincibleTime = invincibleTime;
	}

	void Perso::setInvincibleStarTime(int invincibleStarTime)
	{
		_invincibleStarTime = invincibleStarTime;
	}

	void Perso::setTransformationTime(int transformationTime)
	{
		_transformationTime = transformationTime;
	}

	void Perso::setAttackTime(int attackTime)
	{
		_attackTime = attackTime;
	}

	void Perso::setSpecialAttackTime(int specialAttackTime)
	{
		_specialAttackTime = specialAttackTime;
	}

	void Perso::setThrowShellTime(int throwShellTime)
	{
		_throwShellTime = throwShellTime;
	}

	void Perso::setDeathTime(int deathTime)
	{
		_deathTime = deathTime;
	}

	void Perso::setFinishTime(int finishTime)
	{
		_finishTime = finishTime;
	}

	void Perso::setJumpTime(int jumpTime)
	{
		_jumpTime = jumpTime;
	}

	void Perso::update(const RenderWindow& app)
	{
		updatePerso(app, nullptr);
	}

	void Perso::updatePerso(const RenderWindow& app, InputState* inputState)
	{
		gravity(_speed, app.GetFrameTime());

		/* Lateral movements management */
		lateral_move(app);

		/* Save actual position as previous position */
		_previousPosition = _position;

		/* Compute new position */
		this->setPosition(this->getPosition().x + app.GetFrameTime() * getSpeed().x, 
			this->getPosition().y + app.GetFrameTime() * getSpeed().y);
	}

	void Perso::render(const RenderWindow& app)
	{

	}

	void Perso::lateral_move(const RenderWindow& app)
	{
		int time = app.GetFrameTime();
		const Input& input = app.GetInput();

		if(_state != FINISH_CASTLE)
		{
			if(_state != FINISH)
			{
				if(input.IsKeyDown(sf::Key::Right))
					_side = RIGHT_SIDE;
				else
					_side = LEFT_SIDE;

				if(input.IsKeyDown(sf::Key::Right))
				{
					if(!input.IsKeyDown(sf::Key::Down))
					{
						if(_speed.x < 0)
						{
							if(_environment != AIR && _broughtMonster == nullptr && _state != SKID)
							{
								_state = SKID;
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
										if(input.IsKeyDown(sf::Key::B))
										{
											_state = RUN_1;
										}
										else
										{
											_state = WALK;
										}
										_hud->setNbMonstersKilled(0);
									}
								}
							}
							else
							{
								if(_environment == GROUND)
								{
									_state = WALK_SHELL;
								}
							}
						}
						_speed.x = _speed.x + _acceleration.x * time;
					}
				}
				else if(input.IsKeyDown(sf::Key::Left))
				{
					if(!input.IsKeyDown(sf::Key::Down))
					{
						if(_speed.x > 0)
						{
							if(_environment != AIR && _broughtMonster == nullptr && _state != SKID)
							{
								_state = SKID;
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
										if(input.IsKeyDown(sf::Key::B))
										{
											_state = RUN_1;
										}
										else
										{
											_state = WALK;
										}
										_hud->setNbMonstersKilled(0);
									}
								}
							}
							else
							{
								if(_environment == GROUND)
								{
									_state = WALK_SHELL;
								}
							}
						}
						_speed.x = _speed.x - _acceleration.x * time;
					}
				}
				else if(!_specialAttackTime && !_attackTime){

					if(_broughtMonster == nullptr) 
					{
						if(_state == CLIMB_LADDER)
						{
							if(input.IsKeyDown(sf::Key::Down))
								_speed.y = _speed.y - _acceleration.y * time;
							else if(input.IsKeyDown(sf::Key::Up))
								_speed.y = _speed.y + _acceleration.y * time;
							else
								_speed.y = 0;

						}
						else
						{
							if(_environment == GROUND)
							{
								/*if(k->precedent[BAS] && k->actuel[BAS])*/
								_state = LOWERED_JUMP;
							}
							else
							{
								_state = STANDING;
								_hud->setNbMonstersKilled(0);
							}
						}
					}
					else
					{
						if(_environment == AIR)
						{
							//if(k->precedent[BAS] && k->actuel[BAS])
							_state = LOWERED_JUMP_SHELL;
						}
						else
						{
							_state = STANDING_SHELL;
							_hud->setNbMonstersKilled(0);
						}
					}
				}
			}

			if((!input.IsKeyDown(sf::Key::Right) && !input.IsKeyDown(sf::Key::Left))
				|| (input.IsKeyDown(sf::Key::Down) && _environment == GROUND))
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
		if(_transformation >= FIRE_MARIO)
		{
			transform(SUPER_MARIO);
			/*p->tps_invincible = 2000;
			p->tps_transformation = 1000;
			FSOUND_PlaySound(FSOUND_FREE, p->sons[SND_TOUCHE]);*/
		}
		else if(_transformation == SMALL_MARIO)
		{
			transform(SUPER_MARIO);
			_state = DEAD;
			_speed.x = 0;
			_speed.y = EJECTION_SPEED_Y * 5;
			/*p->tps_mort = TPS_MORT;
			FSOUND_PlaySound(FSOUND_FREE, p->sons[SND_DIE]);
			FSOUND_Stream_Stop(n->musique);*/
		}
		else
		{
			transform(SMALL_MARIO);
			/*p->tps_invincible = 2000;
			p->tps_transformation = 1000;
			FSOUND_PlaySound(FSOUND_FREE, p->sons[SND_TOUCHE]);*/
		}
	}

	void Perso::transform(Transformations nextTransformation)
	{
		/* Selon le futur �tat du personnage,
		on charge la texture appropri�e */
		switch(nextTransformation) {
		case SMALL_MARIO :
			loadPerso("small_mario");
			_transformation = SMALL_MARIO;

			/* Faire correspondre les deux coins bas_gauche de la hitbox */
			//p->position.x = p->position.x + (copy.abscisse_bas - p->texture_act->abscisse_bas);
			break;
		case SUPER_MARIO :
			loadPerso("super_mario");
			_transformation = SUPER_MARIO;

			/* Faire correspondre les deux coins bas_gauche de la hitbox */
			//p->position.x = p->position.x + (copy.abscisse_bas - p->texture_act->abscisse_bas);
			break;
		case FIRE_MARIO :
			loadPerso("fire_mario");
			_transformation = FIRE_MARIO;
			break;
		case ICE_MARIO :
			loadPerso("ice_mario");
			_transformation = ICE_MARIO;
			break;
		default : break;
		}
	}

	void Perso::frictions(float time)
	{
		float coeff;

		/* Frictions are different according to environment*/
		if(_environment == GROUND)
			coeff = CLASSIC_COEFF_FRICTION;
		else
			coeff = AIR_COEFF_FRICTION;

		if(time != 0)
		{
			_speed.x /= 1 + 5 * coeff * time * SLIDE_COEFF_FRICTION;
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
			_state = GET_OUT_FROM_PIPE_VERTICAL;
			_position.x = _insidePipe->getPosition().x * BLOCK_WIDTH + BLOCK_WIDTH - _hitboxSize.x / 2;
			_position.y = _insidePipe->getPosition().y * BLOCK_WIDTH + (_insidePipe->getLenght() + 1) * BLOCK_WIDTH - _hitboxSize.y;
			break;

		case Pipe::TO_BOTTOM:
			_state = GET_OUT_FROM_PIPE_VERTICAL;
			_position.x = _insidePipe->getPosition().x * BLOCK_WIDTH + BLOCK_WIDTH - _hitboxSize.x / 2;
			_position.y = _insidePipe->getPosition().y * BLOCK_WIDTH;
			break;

		case Pipe::TO_LEFT:
			_side = RIGHT_SIDE;
			_state = GET_OUT_FROM_PIPE_HORIZONTAL;
			_position.x = (_insidePipe->getPosition().x + _insidePipe->getLenght()) * BLOCK_WIDTH, 
				_position.y = _insidePipe->getPosition().y * BLOCK_WIDTH;
			break;

		case Pipe::TO_RIGHT:
			_side = LEFT_SIDE;
			_state = GET_OUT_FROM_PIPE_HORIZONTAL;
			_position.x = _insidePipe->getPosition().x * BLOCK_WIDTH;
			_position.y = _insidePipe->getPosition().y * BLOCK_WIDTH;
			break;

		default :
			break;
		}
		// Play sound pipe here !
	}

	void Perso::loadPerso(const string& textureName)
	{
		int abscisse_bas = 0, ordonnee_haut = 0;
		string fileName = textureName + ".perso";
		ifstream stream(fileName.c_str());

		if(stream)
		{
			string word;

			/* We read file to search the keyword and read his value */
			while(getline(stream, word))
			{
				/* Main keywords */
				int found = word.find("abscisse_bas=");
				if(found != string::npos)
				{
					istringstream abscisseBas(word.substr(found + 13));
					abscisseBas >> abscisse_bas;
					continue;
				}
				
				found = word.find("ordonnee_haut=");
				if(found != string::npos)
				{
					istringstream ordonneeHaut(word.substr(found + 14));
					ordonneeHaut >> ordonnee_haut;
					continue;
				}

				{ /* 'nb_sprites_xxx' keyword */
					
					found = word.find("nb_sprites_marche=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbWalkingSpritesStream(word.substr(found + 18));
						nbWalkingSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::WALK, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_course_1=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbRunningSpritesStream(word.substr(found + 20));
						nbRunningSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::RUN_1, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_course_2=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbRunningSpritesStream(word.substr(found + 20));
						nbRunningSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::RUN_2, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_derape=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSkidSpritesStream(word.substr(found + 18));
						nbSkidSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::SKID, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_saut=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbJumpSpritesStream(word.substr(found + 16));
						nbJumpSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::JUMP, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_saut_descendant=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbJumpSpritesStream(word.substr(found + 27));
						nbJumpSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::JUMP_FALLING, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_nage=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSwimmingSpritesStream(word.substr(found + 16));
						nbSwimmingSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::SWIMMING, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_face=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbFaceSpritesStream(word.substr(found + 16));
						nbFaceSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::GET_IN_FROM_PIPE_VERTICAL, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_back=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbBackSpritesStream(word.substr(found + 16));
						nbBackSpritesStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::BACK, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_look_top=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 20));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::LOOK_TOP, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_lowered=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 20));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::LOWERED, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_climb_ladder=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 24));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::CLIMB_LADDER, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_standing_shell=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 26));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::STANDING_SHELL, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_walk_shell=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 22));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::WALK_SHELL, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_push_shell=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 22));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::PUSH_SHELL, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_lowered_shell=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 24));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::LOWERED_SHELL, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_look_top_shell=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 26));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::LOOK_TOP_SHELL, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_attack=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 18));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::ATTACK, nbSprites));
						continue;
					}

					found = word.find("nb_sprites_special_attack=");
					if(found != string::npos)
					{
						int nbSprites = 0;
						istringstream nbSpriteStream(word.substr(found + 26));
						nbSpriteStream >> nbSprites;
						_nbSpritesByState.insert(pair<Perso::State, int>(State::SPECIAL_ATTACK, nbSprites));
						continue;
					}
				}
			}
		}
		else
		{
			string exceptionName = "Exception occured while opening " + fileName;
			throw exception(exceptionName.c_str());
		}

		/* Compute Hitbox Size */
		//_hitboxSize.x = _texture->getImage()->GetWidth() / nb_sprites_max - 2 * abscisse_bas;
		_hitboxSize.y = ordonnee_haut;
	}


	Perso::~Perso()
	{
	}
} // namespace