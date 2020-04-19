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
};

struct furnace
{
	b32 Complete;
};

struct bread
{
	v2i Dir;
	f32 TimeFromLastMove;
};

enum class entity_type
{
	Furnace, Bread, Box
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
global u32 BreadTex;
global u32 WallTex;
global u32 BoxTex;

global constexpr f32 BreadMovementDelay = 0.2f;

global f32 TimeFromComletingLevel = 0.f;
global u32 CurrentLevel = 1;
global char* Map = null;

global char Map1[] = 
"wwwwwwwwwwwwwwww"
"wwwwwwwwwwwwwwww"
"wwww...f...wwwww"
"wwww.......wwwww"
"wwww.......wwwww"
"wwww.......wwwww"
"wwww...p...wwwww"
"wwww.......wwwww"
"wwwwwwwwwwwwwwww";

global char Map2[] = 
"wwwwwwwwwwwwwwww"
"wwwww....wf.wwww"
"wwwww.f..w.wwwww"
"wwwww.....b.wwww"
"wwwww.b..w.wwwww"
"wwwww.......wwww"
"wwwww..p....wwww"
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
AddBreadEntity(v2i Pos, v2i Dir)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Bread;
	Entity.Alive = true;
	Entity.Bread.Dir = Dir;
	Entity.Bread.TimeFromLastMove = 0.f;
	AddEntity(Entity);
}

static void
AddFurnaceEntity(v2i Pos)
{
	entity Entity;
	Entity.Pos = Pos;
	Entity.Type = entity_type::Furnace;
	Entity.Alive = true;
	Entity.Furnace.Complete = false;
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
RemoveEntity(u32 EntityIndex)
{
	++World.DeadEntities;
	World.Entities[EntityIndex].Alive = false;
}

static void
SortAliveEntities()
{
	if(World.DeadEntities > 0)
	{
		std::sort(World.Entities, World.Entities + World.UsedEntities,
		[](const entity& A, const entity& B)
		{
			return !B.Alive;	
		});	
		World.UsedEntities -= World.DeadEntities;
		World.DeadEntities = 0;
	}
}

static void
InitLevel(u32 Level)
{
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
	}

	TimeFromComletingLevel = 0.f;

	Wizard.Dir = V2i(0, 1);

	World.UsedEntities = 0;	
	World.AllFurnaces = 0;
	World.CompleteFurnaces = 0;
	World.UsedWalls = 0;

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
					case 'p': {
						Wizard.Pos = V2i(X, Y);
					} break;

					case 'f': {
						++World.AllFurnaces;
						AddFurnaceEntity(V2i(X, Y));
					} break;

					case 'b': {
						AddBoxEntity(V2i(X, Y));
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
	BreadTex = LoadTexture("resources/textures/bread.png");
	WallTex = LoadTexture("resources/textures/wall.png");
	BoxTex = LoadTexture("resources/textures/box.png");

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
IsFurnace(v2i Pos)
{
	for(u32 EntityIndex = 0;
	    EntityIndex < World.UsedEntities;
		++EntityIndex)
	{
		auto& Entity = World.Entities[EntityIndex];
		if(Entity.Pos == Pos && Entity.Type == entity_type::Furnace)
		{
			return true;
		}
	}
	return false;
}

static b8
IsCollision(v2i Pos)
{
	return IsWall(Pos) || IsFurnace(Pos);
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
			return;
		}

		Entity.Pos += Entity.Bread.Dir;
		Bread.TimeFromLastMove = 0.f;
			

		for(u32 EntityIndex = 0;
			EntityIndex < World.UsedEntities;
			++EntityIndex)
		{
			auto& Furnace = World.Entities[EntityIndex];
			if(Furnace.Type == entity_type::Furnace &&
			   Furnace.Pos == Entity.Pos &&
			   !Furnace.Furnace.Complete)
			{
				Furnace.Furnace.Complete = true;
				++World.CompleteFurnaces;
				if(World.CompleteFurnaces == World.AllFurnaces)
				{
					TimeFromComletingLevel += Dt;
				}
				RemoveEntity(BreadEntityIndex);
			}
		}
	}

	DrawQuad(Entity.Pos, BreadTex);
}

static void
UpdateFurnace(entity& Entity)
{
	if(Entity.Furnace.Complete)
	{
		DrawQuad(Entity.Pos, CompleteFurnaceTex);
	}
	else
	{
		DrawQuad(Entity.Pos, FurnaceTex);
	}
}

static void
UpdateBox(entity& Entity)
{
	DrawQuad(Entity.Pos, BoxTex);
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

			// handle boxes
			for(u32 EntityIndex = 0;
			    EntityIndex < World.UsedEntities;
			    ++EntityIndex)
			{
				auto& Box = World.Entities[EntityIndex];
				if(Box.Type == entity_type::Box && Box.Pos == Wizard.Pos)
				{
					if(IsCollision(Box.Pos + Wizard.Dir))
					{
						Wizard.Pos = LastPos;
						break;
					}
					Box.Pos += Wizard.Dir;
				}
			}
		}
	}
	else
	{
		Wizard.TimeFromMovementPressedKey += Dt;
	}

	if(Wizard.TimeFromSpellPressedKey > 0.5f && Wizard.Breads > 0)
	{
		if(glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			AddBreadEntity(Wizard.Pos + Wizard.Dir, Wizard.Dir);
			--Wizard.Breads;
			Wizard.TimeFromSpellPressedKey = 0.f;
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

			default: assert(true);
		}
	}

	SortAliveEntities();

	// render player ui
	for(i32 X = 0;
	    X < Wizard.Breads;
		++X)
	{
		DrawQuad(V2i(X, 0), BreadTex);
	}

	#define EDITOR 1
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
	if(ImGui::Button("Level 1"))
	{
		InitLevel(1);
		CurrentLevel = 1;
	}
	if(ImGui::Button("Level 2"))
	{
		InitLevel(2);
		CurrentLevel = 2;
	}

	ImGui::End();
	#endif
}
