
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
};

struct bread
{
	v2i Dir;
	f32 TimeFromLastMove;
};

enum class entity_type
{
	Furnace, Bread
};

struct entity
{
	v2i Pos;
	entity_type Type;

	union
	{
		furnace Furnace;
		bread Bread;
	};
};

#define MAX_ENTITIES 50 

struct world
{
	entity* Entities;
	u32 Used;
	u32 ActiveFurnaces, AllFurnaces;
};

global wizard Wizard;
global world World;

global u32 QuadVao;
global u32 QuadShader;

global u32 FurnaceTex;
global u32 BreadTex;

global constexpr f32 BreadMovementDelay = 0.2f;

global u32 CurrentLevel = 1;
global char* Map = null;

global char Map1[17 * 9] = 
"................"
"................"
".......f........"
"................"
"................"
"................"
".......p........"
"................"
"................";

global char Map2[17 * 9] = 
"................"
"................"
".f.f...f........"
"................"
"................"
"................"
".......p........"
"................"
"................";

static void
AddEntity(const entity& Entity)
{
	assert(World.Used < MAX_ENTITIES);

	*(World.Entities + World.Used) = Entity;
	++World.Used;
}

static void
InitLevel(u32 Level)
{
	switch(Level)
	{
		case 1: Map = Map1; break;
		case 2: Map = Map2; break;
	}

	World.Used = 0;	
	World.AllFurnaces = 0;
	World.ActiveFurnaces = 0;

	// draw tilemap
	for(u32 Y = 0;
	    Y < 9;
	    ++Y)
	{	
		for(u32 X = 0;
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

						entity Furnace;
						Furnace.Pos = V2i(X, Y);
						Furnace.Type = entity_type::Furnace;
						AddEntity(Furnace);
					}

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
	BreadTex = LoadTexture("resources/textures/bread.png");

	World.Entities = cast<entity*>(malloc(MAX_ENTITIES * sizeof(entity)));

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

static void
UpdateBread(entity& Entity)
{
	auto& Bread = Entity.Bread;

	Bread.TimeFromLastMove += Dt;
	if(Bread.TimeFromLastMove > BreadMovementDelay)
	{
		Entity.Pos += Entity.Bread.Dir;
		Bread.TimeFromLastMove = 0.f;

		for(u32 EntityIndex = 0;
			EntityIndex < World.Used;
			++EntityIndex)
		{
			auto& Furnace = World.Entities[EntityIndex];
			if(Furnace.Type == entity_type::Furnace &&
			   Furnace.Pos == Entity.Pos)
			{
				++World.ActiveFurnaces;
				printf("COMPLETE!");
				if(World.ActiveFurnaces == World.AllFurnaces)
				{
					++CurrentLevel;
					InitLevel(CurrentLevel);	
				}
			}
		}
	}

	DrawQuad(Entity.Pos, BreadTex);
}

static void
UpdateFurnace(entity& Entity)
{
	DrawQuad(Entity.Pos, FurnaceTex);
}

static void
UpdateAndRender()
{
	// wizard movement
	if(Wizard.TimeFromMovementPressedKey > 0.14f)
	{
	    if(glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS &&
		   Wizard.Pos.X() > 0)
		{
			Wizard.Dir = V2i(-1, 0);
			Wizard.Pos += Wizard.Dir;
			Wizard.TimeFromMovementPressedKey = 0.f;
			Wizard.CurrentTex = Wizard.LeftTex;
		}
		if(glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS &&
		   Wizard.Pos.X() < 16)
		{
			Wizard.Dir = V2i(1, 0);
			Wizard.Pos += Wizard.Dir;
			Wizard.TimeFromMovementPressedKey = 0.f;
			Wizard.CurrentTex = Wizard.RightTex;
		}
		if(glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS &&
		   Wizard.Pos.Y() > 0)
		{
			Wizard.Dir = V2i(0, -1);
			Wizard.Pos += Wizard.Dir;
			Wizard.TimeFromMovementPressedKey = 0.f;
			Wizard.CurrentTex = Wizard.BackTex;
		}
		if(glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS &&
		   Wizard.Pos.Y() < 8)
		{
			Wizard.Dir = V2i(0, 1);
			Wizard.Pos += Wizard.Dir;
			Wizard.TimeFromMovementPressedKey = 0.f;
			Wizard.CurrentTex = Wizard.FrontTex;
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
			entity Entity;
			Entity.Pos = Wizard.Pos + Wizard.Dir;
			Entity.Type = entity_type::Bread;
			Entity.Bread.Dir = Wizard.Dir;
			Entity.Bread.TimeFromLastMove = 0.f;
			AddEntity(Entity);

			--Wizard.Breads;
			Wizard.TimeFromSpellPressedKey = 0.f;
		}
	}
	else
	{
		Wizard.TimeFromSpellPressedKey += Dt;
	}


	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(QuadShader);
	glBindVertexArray(QuadVao);

	// update and draw entities
	DrawQuad(Wizard.Pos, Wizard.CurrentTex);

	for(u32 EntityIndex = 0;
	    EntityIndex < World.Used;
		++EntityIndex)
	{
		auto& Entity = World.Entities[EntityIndex];
		
		switch(Entity.Type)
		{
			case entity_type::Bread: UpdateBread(Entity); break;
			case entity_type::Furnace: UpdateFurnace(Entity); break;

			default: assert(true);
		}
	}

	#define EDITOR 1
	#if EDITOR
	ImGui::Begin("Editor");
	
	char Text[100];

	#define DEBUG_INFO(spf) spf; ImGui::TextUnformatted(Text);

	DEBUG_INFO(sprintf(Text, "Player pos: %i %i", Wizard.Pos.X(), Wizard.Pos.Y()));
	DEBUG_INFO(sprintf(Text, "Breads: %u", Wizard.Breads));
	DEBUG_INFO(sprintf(Text, "CurrentLevel: %u", CurrentLevel));
	DEBUG_INFO(sprintf(Text, "Number of entities: %u", World.Used));
	DEBUG_INFO(sprintf(Text, "Active furnaces: %u", World.ActiveFurnaces));
	DEBUG_INFO(sprintf(Text, "All furnaces: %u", World.AllFurnaces));

	if(ImGui::Button("More Bread"))
	{
		Wizard.Breads += 5;	
	}

	ImGui::End();
	#endif
}
