#include <algorithm>

struct wizard
{
	v2i Pos;
	v2i Dir;
	u32 FrontTex, BackTex, LeftTex, RightTex;
	u32 CurrentTex;
	f32 TimeFromMovementPressedKey = 0.2f;
	f32 TimeFromSpellPressedKey = 0.2f;
	u32 Breads;
	u32 Rolls;
};

struct furnace
{
	enum class complete{No, ByBread, ByRoll};
	complete Complete;
};

struct bread
{
	v2i Dir;
	f32 TimeFromLastMove;

	enum type{Normal, Roll};
	type Type;
};

struct mirror
{
	b32 Left;
	b32 Red;
};

enum class entity_type
{
	Furnace, Bread, Box, Mirror, GreenMirror, Switch
};

struct entity
{
	v2i Pos;
	entity_type Type;
	b32 Alive;

	union
	{
		furnace Furnace;
		bread Bread;
		mirror Mirror;
	};
};

#define MAX_ENTITIES 50 

struct wall
{
	i32 X, Y;
};

struct world
{
	entity* Entities;
	wall* Walls;
	u32 UsedEntities;
	u32 DeadEntities;
	u32 CompleteFurnaces, AllFurnaces;
	u32 UsedWalls;
};

global wizard Wizard;
global world World;

global u32 QuadVao;
global u32 QuadShader;

global u32 FurnaceTex;
global u32 CompleteFurnaceTex;
global u32 CompleteFurnaceRollTex;
global u32 BreadTex;
global u32 RollTex;
global u32 WallTex;
global u32 BoxTex;
global u32 MirrorLeftTex;
global u32 MirrorRightTex;
global u32 RedMirrorLeftTex;
global u32 RedMirrorRightTex;
global u32 GreenMirrorTex;
global u32 SwitchTex;

global constexpr f32 BreadMovementDelay = 0.1f;

global f32 TimeFromComletingLevel = 0.f;
global u32 CurrentLevel = 1;
global char* Map = null;

/* NOTE:
	. - nothing
	w - wall
	F - furnace
	P - player
	B - box
	L - left mirror
	R - right mirror
	l - red mirror left
	r - red mirror right
	S - mirror switch
*/

global char Map1[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwww...F...wwwww"
"wwww.......wwwww"
"wwww.......wwwww"
"wwww.......wwwww"
"wwww...P...wwwww"
"wwww.......wwwww"
"wwwwwwwwwwwwwwww";

global char Map2[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwww.....wF.wwww"
"wwww..F..w.wwwww"
"wwww......B.wwww"
"wwww.....w.wwwww"
"wwww........wwww"
"wwww...P....wwww"
"wwwwwwwwwwwwwwww";

global char Map3[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwF....Lwwwwwww"
"wwwwwwww.wwwwwww"
"wwwww.......wwww"
"wwwww...P...wwww"
"wwwww.......wwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww";

global char Map4[] = 
"wwwwwwwwwwwwwwww"
"wwFwwwwwwwwwwwFw"
"ww.wwwwwwwwwww.w"
"wwL...........Rw"
"wwwwwww.w.wwwwww"
"wwwwww.....wwwww"
"wwwwww.RPL.wwwww"
"wwwwww.....wwwww"
"wwwwwwwwwwwwwwww";

global char Map5[] = 
"wwwwwwwwwwwwwwww"
"wwFwwwwwwwwwwwFw"
"ww.wwwwwwwwwww.w"
"wwL.....L.....Lw"
"wwwwwwww.wwwwwww"
"wwwww......wwwww"
"wwwwwS..P..wwwww"
"wwwww......wwwww"
"wwwwwwwwwwwwwwww";

global char Map6[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwFwwwwwww"
"wwwwwwww.wwwwwww"
"wwwww...P..wwwww"
"wwF.........Fwww"
"wwwww......wwwww"
"wwwwwwwwFwwwwwww"
"wwwwwwwwwwwwwwww";

global char Map7[] = 
"wwwwwwwwwwwwwwww"
"wwwF....Lwwwwwww"
"wwwwwwww.wwwwwww"
"wwFwwwww.wwwwwww"
"ww.ww...P..wwwww"
"wwL...B.....Swww"
"wwwww......wwwww"
"wwwwwwwwFwwwwwww"
"wwwwwwwwwwwwwwww";

global char Map8[] = 
"wwwwwwwwwwwwwwww"
"wwwF....R...Fwww"
"wwwwwwww.wwwwwww"
"wwSwwwww.wwwwwww"
"ww.ww...BP..Swww"
"wwL...B.....Swww"
"wwwwwwww.wwwwwww"
"wwF.....L.....Fw"
"wwwwwwwwwwwwwwww";

global char Map9[] = 
"wwwwwwwwwwwwwwww"
"wwFwF..G...FwFww"
"ww.wwww.wwwww.ww"
"ww.ww..B..www.ww"
"wwG....P.B...Gww"
"ww.ww.....www.ww"
"ww.wwww.wwwww.ww"
"wwFwF..G...FwFww"
"wwwwwwwwwwwwwwww";

global char Map10[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwF.......wwwww"
"wwwwFwww.L...Rww"
"wwww.www..Pww.ww"
"wwwwR......wwFww"
"wwww.wwwww.wwwww"
"wwwwFwwwwwl.Swww"
"wwwwwwwwwwwwwwww";

global char Map11[] = 
"wwwwwwwwwwwwwwww"
"wwF..L..FwFwwwww"
"wSwww.wwww.wwwww"
"w.ww...P..L..wFw"
"w.ww.R..ww...w.w"
"wl.......G....Rw"
"wwwww.wwww...w.w"
"wwF..r....FwwwFw"
"wwwwwwwwwwwwwwww";

global char EndMap[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww";

static void
AddEntity(const entity& Entity)
{
	assert(World.UsedEntities < MAX_ENTITIES);

	*(World.Entities + World.UsedEntities) = Entity;
	++World.UsedEntities;
}

static void
AddBreadEntity(v2i Pos, v2i Dir, bread::type Type)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Bread;
	Entity.Alive = true;
	Entity.Bread.Dir = Dir;
	Entity.Bread.TimeFromLastMove = 0.f;
	Entity.Bread.Type = Type;
	AddEntity(Entity);
}

static void
AddFurnaceEntity(v2i Pos)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Furnace;
	Entity.Alive = true;
	Entity.Furnace.Complete = furnace::complete::No;
	AddEntity(Entity);
}

static void
AddBoxEntity(v2i Pos)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Box;
	Entity.Alive = true;
	AddEntity(Entity);
}

static void
AddMirrorEntity(v2i Pos, b32 Left, b32 Red)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Mirror;
	Entity.Alive = true;
	Entity.Mirror.Left = Left;
	Entity.Mirror.Red = Red;
	AddEntity(Entity);
}

static void
AddGreenMirrorEntity(v2i Pos)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::GreenMirror;
	Entity.Alive = true;
	AddEntity(Entity);
}

static void
AddSwitchEntity(v2i Pos)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Switch;
	Entity.Alive = true;
	AddEntity(Entity);
}

static void
RemoveEntity(u32 EntityIndex)
{
	++World.DeadEntities;
	World.Entities[EntityIndex].Alive = false;
}

static b32
IsThereEntityOfType(entity_type Type)
{
	for(u32 EntityIndex = 0;
	    EntityIndex < World.UsedEntities;
	    ++EntityIndex)
	{
		auto& Entity = World.Entities[EntityIndex];
		if(Entity.Type == Type)
		{
			return true;
		}
	}
	return false;
}

static void
SortAliveEntities()
{
	if(World.DeadEntities > 0)
	{
		std::sort(World.Entities, World.Entities + World.UsedEntities,
		[](const entity& A, const entity& B)
		{
			return A.Alive && !B.Alive;	
		});	
		World.UsedEntities -= World.DeadEntities;
		World.DeadEntities = 0;
	}
}

static void
InitLevel(u32 Level)
{
	Wizard.Breads = 0;
	Wizard.Rolls = 0;

	TimeFromComletingLevel = 0.f;

	Wizard.Dir = V2i(0, -1);
	Wizard.CurrentTex = Wizard.BackTex; 

	World.UsedEntities = 0;	
	World.AllFurnaces = 0;
	World.CompleteFurnaces = 0;
	World.UsedWalls = 0;

	switch(Level)
	{
		case 1: {
			Map = Map1;	
			Wizard.Breads = 1;
		} break;

		case 2: {
			Map = Map2;	
			Wizard.Breads = 2;
		} break;

		case 3: {
			Map = Map3;
			Wizard.Breads = 1;
		} break;

		case 4: {
			Map = Map4;
			Wizard.Breads = 2;
		} break;

		case 5: {
			Map = Map5;
			Wizard.Breads = 3;
		} break;

		case 6: {
			Map = Map6;
			Wizard.Rolls = 1;
		} break;

		case 7: {
			Map = Map7;
			Wizard.Rolls = 1;
		} break;

		case 8: {
			Map = Map8;
			Wizard.Rolls = 2;
		} break;

		case 9: {
			Map = Map9;
			Wizard.Rolls = 2;
		} break;

		case 10: {
			Map = Map10;
			Wizard.Breads = 1;
			Wizard.Rolls = 2;
		} break;

		case 11: {
			Map = Map11;
			Wizard.Rolls = 2;
		} break;

		case 12: {
			Map = EndMap;
			Wizard.Dir = V2i(0, 1);
			Wizard.CurrentTex = Wizard.FrontTex; 
			Wizard.Pos = V2i(8, 1);
		} break;
	}

	// draw tilemap
	for(i32 Y = 0;
	    Y < 9;
	    ++Y)
	{	
		for(i32 X = 0;
			X < 16;
			++X)
		{
			char Tile = Map[Y*16 + X];
			if(Tile != '.')
			{
				switch(Tile)
				{	
					case 'P': {
						Wizard.Pos = V2i(X, Y);
					} break;

					case 'F': {
						++World.AllFurnaces;
						AddFurnaceEntity(V2i(X, Y));
					} break;

					case 'B': {
						AddBoxEntity(V2i(X, Y));
					} break;

					case 'L': {
						AddMirrorEntity(V2i(X, Y), true, false);
					} break;

					case 'R': {
						AddMirrorEntity(V2i(X, Y), false, false);
					} break;

					case 'l': {
						AddMirrorEntity(V2i(X, Y), true, true);
					} break;

					case 'r': {
						AddMirrorEntity(V2i(X, Y), false, true);
					} break;

					case 'G': {
						AddGreenMirrorEntity(V2i(X, Y));
					} break;

					case 'S': {
						AddSwitchEntity(V2i(X, Y));
					} break;

					case 'w': {
						*(World.Walls + World.UsedWalls) = {X, Y};
						++World.UsedWalls;
					} break;

					default: continue;
				}
			}
		}
	}
}

static void
InitApp()
{
	SoundEngine->play2D("resources/audio/mainTheme.wav", true);

	glClearColor(0.1f, 0.1f, 0.1f, 1.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	QuadShader = LoadShader("src/shaders/quad.glsl");
	
	Wizard.FrontTex = LoadTexture("resources/textures/wizardFront.png");
	Wizard.BackTex = LoadTexture("resources/textures/wizardBack.png");
	Wizard.LeftTex = LoadTexture("resources/textures/wizardLeft.png");
	Wizard.RightTex = LoadTexture("resources/textures/wizardRight.png");
	Wizard.CurrentTex = Wizard.FrontTex;

	Wizard.Breads = 2;

	FurnaceTex = LoadTexture("resources/textures/furnace.png");
	CompleteFurnaceTex = LoadTexture("resources/textures/completeFurnace.png");
	CompleteFurnaceRollTex = LoadTexture("resources/textures/completeFurnaceRoll.png");
	BreadTex = LoadTexture("resources/textures/bread.png");
	RollTex = LoadTexture("resources/textures/roll.png");
	WallTex = LoadTexture("resources/textures/wall.png");
	BoxTex = LoadTexture("resources/textures/box.png");
	MirrorLeftTex = LoadTexture("resources/textures/mirrorLeft.png");
	MirrorRightTex = LoadTexture("resources/textures/mirrorRight.png");
	RedMirrorLeftTex = LoadTexture("resources/textures/redMirrorLeft.png");
	RedMirrorRightTex = LoadTexture("resources/textures/redMirrorRight.png");
	GreenMirrorTex = LoadTexture("resources/textures/greenMirror.png");
	SwitchTex = LoadTexture("resources/textures/switch.png");

	World.Entities = cast<entity*>(malloc(MAX_ENTITIES * sizeof(entity)));
	World.Walls = cast<wall*>(malloc(16 * 9 * sizeof(wall)));

	InitLevel(1);

	glUseProgram(QuadShader);
	mat4 Projection = Mat4Orthographic(0.f, 16.f, 9.f, 0.f, 0.f, 1.f);
	SetUniformMat4(QuadShader, "Projection", Projection);

	glGenVertexArrays(1, &QuadVao);
	glBindVertexArray(QuadVao);

	f32 VertexData[] = {
		0.f, 0.f, 0.f, 0.f,
		1.f, 0.f, 1.f, 0.f,
		0.f, 1.f, 0.f, 1.f,
		1.f, 1.f, 1.f, 1.f
	};

	u32 Vbo;
	glGenBuffers(1, &Vbo);
	glBindBuffer(GL_ARRAY_BUFFER, Vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), null);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
}

static void
DrawQuad(v2i Pos, u32 Texture)
{
	glBindTexture(GL_TEXTURE_2D, Texture);
	SetUniform2f(QuadShader, "Pos", Pos);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static b8
IsWall(v2i Pos)
{
	for(u32 WallIndex = 0;
	    WallIndex < World.UsedWalls;
		++WallIndex)
	{
		wall& Wall = World.Walls[WallIndex];
		if(Wall.X == Pos.X() && Wall.Y == Pos.Y())
		{
			return true;
		}
	}
	return false;
}

static b8
IsEntityOfType(v2i Pos, entity_type Type)
{
	for(u32 EntityIndex = 0;
	    EntityIndex < World.UsedEntities;
		++EntityIndex)
	{
		auto& Entity = World.Entities[EntityIndex];
		if(Entity.Pos == Pos && Entity.Type == Type)
		{
			return true;
		}
	}
	return false;
}

static void
UpdateBread(entity& Entity, u32 BreadEntityIndex)
{
	auto& Bread = Entity.Bread;

	Bread.TimeFromLastMove += Dt;
	if(Bread.TimeFromLastMove > BreadMovementDelay)
	{
		if(IsWall(Entity.Pos + Bread.Dir))
		{
			RemoveEntity(BreadEntityIndex);
			SoundEngine->play2D("resources/audio/breadWallCollision.wav", false);
			return;
		}

		Entity.Pos += Entity.Bread.Dir;
		Bread.TimeFromLastMove = 0.f;
			

		for(u32 EntityIndex = 0;
			EntityIndex < World.UsedEntities;
			++EntityIndex)
		{
			auto& OtherEntity = World.Entities[EntityIndex];
			if(OtherEntity.Pos == Entity.Pos)
			{
				switch(OtherEntity.Type)
				{
					case entity_type::Furnace: 
					{
						auto& Furnace = OtherEntity.Furnace;
						if(Furnace.Complete == furnace::complete::No)
						{
							if(Bread.Type == bread::Normal)
							{
								Furnace.Complete = furnace::complete::ByBread;
							}
							else if(Bread.Type == bread::Roll)
							{
								Furnace.Complete = furnace::complete::ByRoll;
							}

							++World.CompleteFurnaces;
							if(World.CompleteFurnaces == World.AllFurnaces)
							{
								TimeFromComletingLevel += Dt;
							}
							RemoveEntity(BreadEntityIndex);
							SoundEngine->play2D("resources/audio/breadInFurnace.wav", false);
						}
						else
						{
							SoundEngine->play2D("resources/audio/breadWallCollision.wav", false);
						}
					} break;

					case entity_type::Mirror:
					{
						auto& Mirror = OtherEntity.Mirror;
						if(Mirror.Left)
						{
							Bread.Dir = V2i(Bread.Dir.Y(), Bread.Dir.X());
						}
						else
						{
							Bread.Dir = V2i(-Bread.Dir.Y(), -Bread.Dir.X());
						}
						SoundEngine->play2D("resources/audio/breadMirrorCollision.wav", false);
					} break;

					case entity_type::GreenMirror:
					{
						AddBreadEntity(Entity.Pos, V2i(Bread.Dir.Y(), Bread.Dir.X()), Bread.Type);
						AddBreadEntity(Entity.Pos, V2i(-Bread.Dir.Y(), -Bread.Dir.X()), Bread.Type); 
						SoundEngine->play2D("resources/audio/breadMirrorCollision.wav", false);
					} break;

					case entity_type::Box:
					{
						RemoveEntity(BreadEntityIndex);
						SoundEngine->play2D("resources/audio/breadWallCollision.wav", false);
						return;
					} break;
					
					case entity_type::Switch:
					{
						for(u32 EntityIndex2 = 0;
						    EntityIndex2 < World.UsedEntities;
						    ++EntityIndex2)
						{
							auto& Mirror = World.Entities[EntityIndex2];
							if(Mirror.Type == entity_type::Mirror)
							{	
								if(!Mirror.Mirror.Red)
								{
									Mirror.Mirror.Left = !Mirror.Mirror.Left;
								}
							}
						}
						SoundEngine->play2D("resources/audio/switch.wav", false);
					} break;
				}
			}
		}
	}

	u32 Tex;
	switch(Bread.Type)
	{
		case bread::Normal: Tex = BreadTex; break;
		case bread::Roll: Tex = RollTex; break;
	}
	DrawQuad(Entity.Pos, Tex);
}

static void
UpdateFurnace(entity& Entity)
{
	switch(Entity.Furnace.Complete)
	{
		case furnace::complete::No: {
			DrawQuad(Entity.Pos, FurnaceTex);
		} break;

		case furnace::complete::ByBread: {
			DrawQuad(Entity.Pos, CompleteFurnaceTex);
		} break;

		case furnace::complete::ByRoll: {
			DrawQuad(Entity.Pos, CompleteFurnaceRollTex);
		} break;
	}
}

static void
UpdateBox(entity& Entity)
{
	DrawQuad(Entity.Pos, BoxTex);
}

static void
UpdateMirror(entity& Entity)
{
	if(Entity.Mirror.Red)
	{
		if(Entity.Mirror.Left)
		{
			DrawQuad(Entity.Pos, RedMirrorLeftTex);
		}
		else
		{
			DrawQuad(Entity.Pos, RedMirrorRightTex);
		}
	}
	else
	{
		if(Entity.Mirror.Left)
		{
			DrawQuad(Entity.Pos, MirrorLeftTex);
		}
		else
		{
			DrawQuad(Entity.Pos, MirrorRightTex);
		}
	}
}

static void
UpdateGreenMirror(entity& Entity)
{
	DrawQuad(Entity.Pos, GreenMirrorTex);
}

static void
UpdateSwitch(entity& Entity)
{
	DrawQuad(Entity.Pos, SwitchTex);
}

static void
UpdateAndRender()
{
	// complete level
	if(TimeFromComletingLevel > 0.f)
	{
		TimeFromComletingLevel += Dt;
		if(TimeFromComletingLevel > 0.5f)
		{
			++CurrentLevel;
			InitLevel(CurrentLevel);	
		}
	}

	// wizard movement
	if(Wizard.TimeFromMovementPressedKey > 0.14f)
	{
		v2i LastPos = Wizard.Pos;
		b32 Move = false;

		auto IsCollision = [](v2i Pos)
		{
			return IsWall(Pos) || IsEntityOfType(Pos, entity_type::Furnace); 
		};

		if(glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS &&
		   !IsCollision(Wizard.Pos + V2i(-1, 0)))
		{
			Move = true;
			Wizard.Dir = V2i(-1, 0);
			Wizard.CurrentTex = Wizard.LeftTex;
		}
		else if(glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS &&
				!IsCollision(Wizard.Pos + V2i(1, 0)))
		{
			Move = true;
			Wizard.Dir = V2i(1, 0);
			Wizard.CurrentTex = Wizard.RightTex;
		}
		else if(glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS &&
				!IsCollision(Wizard.Pos + V2i(0, -1)))
		{
			Move = true;
			Wizard.Dir = V2i(0, -1);
			Wizard.CurrentTex = Wizard.BackTex;
		}
		else if(glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS &&
				!IsCollision(Wizard.Pos + V2i(0, 1)))
		{
			Move = true;
			Wizard.Dir = V2i(0, 1);
			Wizard.CurrentTex = Wizard.FrontTex;
		}

		if(Move)
		{
			Wizard.Pos += Wizard.Dir;
			Wizard.TimeFromMovementPressedKey = 0.f;

			// handle moveable objects
			for(u32 EntityIndex = 0;
			    EntityIndex < World.UsedEntities;
			    ++EntityIndex)
			{
				auto& Entity = World.Entities[EntityIndex];
				if((Entity.Type == entity_type::Box ||
				    Entity.Type == entity_type::Mirror ||
					Entity.Type == entity_type::GreenMirror) &&
				   Entity.Pos == Wizard.Pos)
				{
					if(IsCollision(Entity.Pos + Wizard.Dir))
					{
						Wizard.Pos = LastPos;
						break;
					}
					Entity.Pos += Wizard.Dir;
					SoundEngine->play2D("resources/audio/boxMove.wav", false);
				}
			}
		}
	}
	else
	{
		Wizard.TimeFromMovementPressedKey += Dt;
	}

	if(Wizard.TimeFromSpellPressedKey > 0.5f)
	{
		if(glfwGetKey(Window, GLFW_KEY_1) == GLFW_PRESS && Wizard.Breads > 0)
		{
			--Wizard.Breads;
			AddBreadEntity(Wizard.Pos, Wizard.Dir, bread::Normal);
			Wizard.TimeFromSpellPressedKey = 0.f;
			SoundEngine->play2D("resources/audio/spellCast.wav", false);
		}
		else if(glfwGetKey(Window, GLFW_KEY_2) == GLFW_PRESS && Wizard.Rolls > 0)
		{
			--Wizard.Rolls;
			AddBreadEntity(Wizard.Pos, V2i(1, 0), bread::Roll);
			AddBreadEntity(Wizard.Pos, V2i(-1, 0), bread::Roll);
			AddBreadEntity(Wizard.Pos, V2i(0, 1), bread::Roll);
			AddBreadEntity(Wizard.Pos, V2i(0, -1), bread::Roll);
			Wizard.TimeFromSpellPressedKey = 0.f;
			SoundEngine->play2D("resources/audio/spellCast.wav", false);
		}
	}
	else
	{
		Wizard.TimeFromSpellPressedKey += Dt;
	}

	if(glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
	{
		InitLevel(CurrentLevel);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(QuadShader);
	glBindVertexArray(QuadVao);

	// draw walls
	for(u32 WallIndex = 0;
	    WallIndex < World.UsedWalls;
	    ++WallIndex)
	{	
		wall& Wall = World.Walls[WallIndex];
		DrawQuad(V2i(Wall.X, Wall.Y), WallTex);
	}

	// update and draw entities
	DrawQuad(Wizard.Pos, Wizard.CurrentTex);

	for(u32 EntityIndex = 0;
	    EntityIndex < World.UsedEntities;
		++EntityIndex)
	{
		auto& Entity = World.Entities[EntityIndex];
		
		switch(Entity.Type)
		{
			case entity_type::Bread: UpdateBread(Entity, EntityIndex); break;
			case entity_type::Furnace: UpdateFurnace(Entity); break;
			case entity_type::Box: UpdateBox(Entity); break;
			case entity_type::Mirror: UpdateMirror(Entity); break;
			case entity_type::GreenMirror: UpdateGreenMirror(Entity); break;
			case entity_type::Switch: UpdateSwitch(Entity); break;

			default: assert(true);
		}
	}

	SortAliveEntities();

	// render player ui
	i32 BreadUIX = 0;

	for(i32 Bread = 0;
	    Bread < cast<i32>(Wizard.Breads);
	    ++Bread, ++BreadUIX)
	{
		DrawQuad(V2i(BreadUIX, 0), BreadTex);
	}

	for(i32 Roll = 0;
	    Roll < cast<i32>(Wizard.Rolls);
		++Roll, ++BreadUIX)
	{
		DrawQuad(V2i(BreadUIX, 0), RollTex);
	}

	auto BeginTutorialWindow = [](f32 X, f32 Y, const char* Name = "Tutorial") 
	{
		ImGui::SetNextWindowPos({X, Y});
		ImGui::Begin(Name, null, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetWindowFontScale(3.f);
	};

	switch(CurrentLevel)
	{
		case 1: {
			BeginTutorialWindow(700.f, 30.f);
			ImGui::TextUnformatted("AWSD - Movement");
			ImGui::TextUnformatted("Num 1 - Cast bread spell");
			ImGui::End();
		} break;

		case 6: {
			BeginTutorialWindow(700.f, 30.f);
			ImGui::TextUnformatted("Num 2 - Cast roll spell");
			ImGui::End();
		} break;

		case 12: {
			ImGui::SetNextWindowPos({200.f, 400.f});
			ImGui::SetNextWindowSize({1500.f, 450.f});
			ImGui::Begin("End", null, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
			ImGui::SetWindowFontScale(4.f);
			ImGui::TextUnformatted("Thanks for playing");
			ImGui::TextUnformatted("");
			ImGui::TextUnformatted("Game was made by:");
			ImGui::TextUnformatted("Grzegorz \"Czapa\" Bednorz - Coding & Design & Graphics");
			ImGui::TextUnformatted("Jerzy Wujczyk - Sound & Music");
			ImGui::TextUnformatted("");
			if(ImGui::Button("Exit game"))
			{
				glfwSetWindowShouldClose(Window, GLFW_TRUE);
			}
			ImGui::End();
		} break;
	}

	if(Wizard.Breads == 0 && Wizard.Rolls == 0 &&
	   TimeFromComletingLevel == 0.f && !IsThereEntityOfType(entity_type::Bread) &&
	   CurrentLevel != 12)
	{
		BeginTutorialWindow(30.f, 30.f, "Reset");
		ImGui::TextUnformatted("R - Reset level");
		ImGui::End();
	}

	#define EDITOR 0 
	#if EDITOR
	ImGui::Begin("Editor");
	
	char Text[100];

	#define DEBUG_INFO(spf) spf; ImGui::TextUnformatted(Text);

	DEBUG_INFO(sprintf(Text, "Player pos: %i %i", Wizard.Pos.X(), Wizard.Pos.Y()));
	DEBUG_INFO(sprintf(Text, "Breads: %u", Wizard.Breads));
	DEBUG_INFO(sprintf(Text, "CurrentLevel: %u", CurrentLevel));
	DEBUG_INFO(sprintf(Text, "Number of entities: %u", World.UsedEntities));
	DEBUG_INFO(sprintf(Text, "Complete furnaces: %u", World.CompleteFurnaces));
	DEBUG_INFO(sprintf(Text, "All furnaces: %u", World.AllFurnaces));

	if(ImGui::Button("More Bread"))
	{
		Wizard.Breads += 5;	
	}

	for(u32 Level = 1;
	    Level <= 12;
		++Level)
	{
		char Name[50];
		sprintf(Name, "Level %u", Level);
		if(ImGui::Button(Name))
		{
			InitLevel(Level);
			CurrentLevel = Level;
		}
	}

	ImGui::End();
	#endif
}
