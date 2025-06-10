#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupSprite();
int loadTexture(string filePath);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 800;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 void main()
 {
	tex_coord = vec2(texc.s, 1.0 - texc.t);
	gl_Position = vec4(position, 1.0);
 }
 )";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 void main()
 {
	 color = texture(tex_buff,tex_coord);
 }
 )";

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Configurações da janela GLFW
	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "ObjetosTexturizados", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();
	GLuint VAO = setupSprite();

	// --- Aqui carregamos 3 texturas diferentes para cada retângulo ---
	GLuint texCentro = loadTexture("../assets/sprites/Meio.png");
	GLuint texEsquerda = loadTexture("../assets/sprites/Esquerda.png");
	GLuint texDireita = loadTexture("../assets/sprites/Direita.png");
	GLuint texFundo = loadTexture("../assets/sprites/Fundo.png");

	glUseProgram(shaderID);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double prev_s = glfwGetTime();
	double title_countdown_s = 0.1;

	while (!glfwWindowShouldClose(window))
	{
		// Atualiza título com FPS
		{
			double curr_s = glfwGetTime();
			double elapsed_s = curr_s - prev_s;
			prev_s = curr_s;

			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s;
				char tmp[256];
				sprintf(tmp, "ObjetosTexturizados\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);
				title_countdown_s = 0.1;
			}
		}

		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO);

		// Desenha o fundo primeiro (índices 0 a 3)
		glBindTexture(GL_TEXTURE_2D, texFundo);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// Desenha centro (índices 4 a 7)
		glBindTexture(GL_TEXTURE_2D, texCentro);
		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

		// Desenha esquerda (índices 8 a 11)
		glBindTexture(GL_TEXTURE_2D, texEsquerda);
		glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

		// Desenha direita (índices 12 a 15)
		glBindTexture(GL_TEXTURE_2D, texDireita);
		glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// Setup do shader
int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Setup dos vértices dos retângulos
int setupSprite()
{
	GLfloat vertices[] = {
		// Retângulo de fundo (cobre toda a tela)
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		
		// Retângulo 1 (centro)
		-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

		// Retângulo 2 (esquerda)
		-1.0f, 0.5f, 0.0f, 0.0f, 1.0f,
		-1.0f, -0.5f, 0.0f, 0.0f, 0.0f,
		-0.0f, 0.5f, 0.0f, 1.0f, 1.0f,
		-0.0f, -0.5f, 0.0f, 1.0f, 0.0f,

		// Retângulo 3 (direita)
		0.1f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.1f, -0.5f, 0.0f, 0.0f, 0.0f,
		1.1f, 0.5f, 0.0f, 1.0f, 1.0f,
		1.1f, -0.5f, 0.0f, 1.0f, 0.0f};

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

// Função de carregar textura da imagem
int loadTexture(string filePath)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // png e outros com alpha
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture: " << filePath << std::endl;
	}

	stbi_image_free(data);

	return texID;
}