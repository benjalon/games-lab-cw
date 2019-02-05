static const float SCREEN_WIDTH = 1600.0f;
static const float SCREEN_HEIGHT = 1000.0f;

static void RenderSceneCB();
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
static void CompileShaders();
static void InitializeGlutCallbacks();