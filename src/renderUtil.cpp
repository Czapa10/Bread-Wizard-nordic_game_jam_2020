static void
CheckShaderCompilationErrors(u32 Shader, const char* Type)
{
	i32 Success;
	char InfoLog[1024];
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
	if(!Success)
	{
		glGetShaderInfoLog(Shader, 1024, null, InfoLog);
		printf("%s shader compilation error \n %s \n\n\n", Type, InfoLog);
	}
}

static void
CheckProgramLinkingErrors(u32 Program)
{
	i32 Success;
	char InfoLog[1024];
	glGetProgramiv(Program, GL_LINK_STATUS, &Success);
	if(!Success)
	{
		glGetProgramInfoLog(Program, 1024, null, InfoLog);
		printf("Program linking error \n %s \n\n\n", InfoLog);
	}
}

static u32 
LoadShaderFromStrings(const char* VertexCode, const char* FragmentCode)
{
	u32 VS = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VS, 1, &VertexCode, null);
	glCompileShader(VS);
	CheckShaderCompilationErrors(VS, "Vertex");

	u32 FS = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FS, 1, &FragmentCode, null);
	glCompileShader(FS);
	CheckShaderCompilationErrors(VS, "Fragment");

	u32 Program = glCreateProgram();
	glAttachShader(Program, VS);
	glAttachShader(Program, FS);
	glLinkProgram(Program);
	CheckProgramLinkingErrors(Program);

	glDeleteShader(VS);
	glDeleteShader(FS);

	return Program;
}

// TODO: Pass Arena
static u32
LoadShaderFromFile(const char* Path)
{
	FILE* File;	
	
	File = fopen(Path, "r");
	if(!File)
	{
		printf("Failed to open vertex shader file \"%s\"", Path);
		return 0;
	}

	fseek(File, 0, SEEK_END);	
	u32 FileLength = ftell(File);
	rewind(File);
	char* AllCode = cast<char*>(calloc(FileLength + 2, sizeof(char)));
	fread(AllCode, 1, FileLength, File);

	char* Border = strstr(AllCode, "===");
	if(!Border)
	{
		puts("Failed to find === border between vertex and fragment shader");
		return 0;
	}
	Border[0] = '\n';
	Border[1] = '\0';

	return LoadShaderFromStrings(AllCode, Border + 3); 
}

static u32
LoadTexture(const char* Path)
{
	u32 Texture = 0;
	i32 Width, Height, BytesPerPixel;
	unsigned char* Data = stbi_load(Path, &Width, &Height, &BytesPerPixel, 0);
	if(Data)
	{
		glGenTextures(1, &Texture);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		u32 Format, InternalFormat;
		if(BytesPerPixel == 4)
		{
			Format = GL_RGBA;
			InternalFormat = GL_RGBA8;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		else if(BytesPerPixel == 3)
		{
			Format = GL_RGB;
			InternalFormat = GL_RGB8;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
	}
	else
	{
		printf("Failed to load texture \"%s\"", Path);
	}
	stbi_image_free(Data);
	return Texture;
}

