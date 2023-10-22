/**
* Author: Amy Fouzia
* Assignment: Pong Clone
* Date due: 2023-10-14, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.549f,
BG_BLUE = 0.2902f,
BG_GREEN = 0.059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

SDL_Window* g_display_window;

bool g_game_is_running = true;
float g_previous_ticks = 0.0f;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix,
g_model_matrix,
e_model_matrix,
l_model_matrix,
p1_model_matrix,
p2_model_matrix,
g_projection_matrix,
g_tran_matrix;

glm::vec3 g_position = glm::vec3(-4.0f, 0.0f, 0.0f);
glm::vec3 g_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 e_position = glm::vec3(4.0f, 0.0f, 0.0f);
glm::vec3 e_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 l_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 l_movement = glm::vec3(0.5f, 0.5f, 0.0f);

//off screen
glm::vec3 p1_position = glm::vec3(0.0f, -10.0f, 0.0f);
glm::vec3 p1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 p2_position = glm::vec3(0.0f, -10.0f, 0.0f);
glm::vec3 p2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

const float MILLISECONDS_IN_SECOND      = 1000.0, 
            MINIMUM_COLLISION_DISTANCE  = 1.0f,  
            PADDLE_WIDTH                = 0.25f,
            PADDLE_HEIGHT               = 1.5f,
            BALL_WIDTH                  = 0.75f,
            BALL_HEIGHT                 = 0.5f,
            PLAYER_SPEED                = 1.0f;

bool multiplayer = true,
     game_end    = false;

int p1_pts = 0,
    p2_pts = 0;

//defining imgs
const char RACKET_ONE_FILEPATH[] = "racketOne.png";
const char RACKET_TWO_FILEPATH[] = "racketTwo.png";
const char BIRDIE_ONE_FILEPATH[] = "birdieOne.png";
const char P1_FILEPATH[] = "P1_Wins.jpg";
const char P2_FILEPATH[] = "P2_Wins.jpg";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0; // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0; // this value MUST be zero

GLuint racketOne_texture_id;
GLuint racketTwo_texture_id;
GLuint birdie_texture_id;
GLuint P1_texture_id;
GLuint P2_texture_id;

GLuint load_texture(const char* filepath)
{
    // load image file
    int width, height, num_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &num_components, STBI_rgb_alpha);

    // quit if it fails
    if (image == NULL)
    {
        std::cout << "Unable to load image. Make sure the path is correct.";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA,
        width, height,
        TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE,
        image
    );

    // mag/min
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // release mem
    stbi_image_free(image);

    return textureID;
}

void initialise()
{   
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Simple 2D Scene",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);  
    g_model_matrix = glm::mat4(1.0f); 
    e_model_matrix = glm::mat4(1.0f); 
    l_model_matrix = glm::mat4(1.0f);
    p1_model_matrix = glm::mat4(1.0f);
    p2_model_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); 
    g_tran_matrix = g_model_matrix;

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    GLuint racketOne_texture_id = load_texture(RACKET_ONE_FILEPATH);
    GLuint racketTwo_texture_id = load_texture(RACKET_TWO_FILEPATH);
    GLuint birdie_texture_id = load_texture(BIRDIE_ONE_FILEPATH);
    GLuint P1_texture_id = load_texture(P1_FILEPATH);
    GLuint P2_texture_id = load_texture(P2_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool check_collision(glm::vec3& paddle_position)
{
    float x_distance = fabs(paddle_position[0] - l_position[0]) - ((PADDLE_WIDTH + BALL_WIDTH) / 2.0f);
    float y_distance = fabs(paddle_position[1] - l_position[1]) - ((PADDLE_HEIGHT + BALL_HEIGHT) / 2.0f);

    if (x_distance < 0 && y_distance < 0)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

void process_input()
{
    g_movement = glm::vec3(0.0f);
    e_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))                                            
    {                                                                       
        switch (event.type)                                                   
        {                                                                     
        // End game                                                       
        case SDL_QUIT:                                                    
        case SDL_WINDOWEVENT_CLOSE:                                       
            g_game_is_running = false;                                    
            break;                                                        
            
        case SDL_KEYDOWN:                                                 
            switch (event.key.keysym.sym)                                 
            {  
                // Quit game with q   
                case SDLK_q:                                                                
                    g_game_is_running = false;                            
                    break;                                                
            
                //toggle multiplayer mode with t
                case SDLK_t:
                    multiplayer = (!multiplayer);
                    break;

                default:                                                  
                    break;                                                
            }                                                             
                                                                          
        default:                                                          
            break;                                                        
        }                                                                     
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);   
    // player two movement
    if (key_state[SDL_SCANCODE_UP] && multiplayer == true && e_position[1] < 3)
    {                                                                        
        e_movement.y = 1.0f;
    }                                                                        
    else if (key_state[SDL_SCANCODE_DOWN] && multiplayer == true && e_position[1] > -3)
    {                                                                        
        e_movement.y = -1.0f;
    }

    //player one movement
    if (key_state[SDL_SCANCODE_W] && g_position[1] < 3)
    {
        g_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S] && g_position[1] > -3)
    {
        g_movement.y = -1.0f;
    }

    if (glm::length(g_movement) > 1.0f)
    {
        g_movement = glm::normalize(g_movement);
    }

    if (glm::length(e_movement) > 1.0f)
    {
        e_movement = glm::normalize(e_movement);
    }
}

float move = 1.0f;

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    if (e_position[1] > 3) {
        move = -1.0f;
    }
    if (e_position[1] < -3) {
        move = 1.0f;
    }

    //player two solo-mode
    if (!multiplayer)
    {
        e_movement.y = move;
    }
    g_model_matrix = glm::mat4(1.0f);
    g_position += g_movement * PLAYER_SPEED * delta_time;
    g_model_matrix = glm::translate(g_model_matrix, g_position);

    e_model_matrix = glm::mat4(1.0f);
    e_position += e_movement * PLAYER_SPEED * delta_time;
    e_model_matrix = glm::translate(e_model_matrix, e_position);

    l_model_matrix = glm::mat4(1.0f);
    l_position += l_movement * PLAYER_SPEED * delta_time;
    l_model_matrix = glm::translate(l_model_matrix, l_position);

    p1_model_matrix = glm::mat4(1.0f);
    p1_position += p1_movement * PLAYER_SPEED * delta_time;
    p1_model_matrix = glm::translate(p1_model_matrix, p1_position);

    p2_model_matrix = glm::mat4(1.0f);
    p2_position += p2_movement * PLAYER_SPEED * delta_time;
    p2_model_matrix = glm::translate(p2_model_matrix, p2_position);

    //ball to paddle one collision
    if (check_collision(e_position))
    {
        l_movement.x *= -1.0;
    }

     //ball to paddle two collision
    if (check_collision(g_position))
    {
        l_movement.x *= -1.0;
    }

    //ball to top/bottom wall collision
    if(l_position[1] > 3.5 || l_position[1] < -3.5)
    {
        l_movement.y *= -1.0;
    }

    //ball to right wall collision
    if (l_position[0] > 4.875)
    {   
        p1_pts += 1;
        game_end = true;
    }

    //ball to left wall collision
    if (l_position[0] < -4.875)
    {
        p2_pts += 1;
        game_end = true;
    
    }

    if (game_end)
    {
        if (p1_pts > p2_pts) {
            //print "Player One Wins"
            p1_position[1] = 0.0f;
            g_position[1] = -20.0f;
            e_position[1] = -20.0f;
            l_position[1] = -20.0f;
            //g_game_is_running = false;
        }
        else {
            //print "Player Two Wins"
            p2_position[1] = 0.0f;
            g_position[1] = -20.0f;
            e_position[1] = -20.0f;
            l_position[1] = -20.0f;
            //g_game_is_running = false;
        }
    }

}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float paddle_one_vertices[] = {
        -0.125f, -0.75f, 
         0.125f, -0.75f, 
         0.125f,  0.75f,  // triangle 1

        -0.125f, -0.75f, 
         0.125f,  0.75f,
        -0.125f,  0.75f   // triangle 2
    };

    float paddle_one_texture_coordinates[] = {
        0.0f, 1.0f, 
        1.0f, 1.0f, 
        1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 
        1.0f, 0.0f, 
        0.0f, 0.0f,     // triangle 2
    };

    float paddle_two_vertices[] = {
        -0.125f, -0.75f,
         0.125f, -0.75f,
         0.125f,  0.75f,  // triangle 1

        -0.125f, -0.75f,
         0.125f,  0.75f,
        -0.125f,  0.75f   // triangle 2
    };

    float paddle_two_texture_coordinates[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,     // triangle 2
    };

    float ball_vertices[] = {
       -0.375f, -0.25f, 
       0.375f, -0.25f, 
       0.375f, 0.25f,  // triangle 1
        
       -0.375f, -0.25f, 
       0.375f, 0.25f, 
       -0.375f, 0.25f   // triangle 2
    };

    float ball_texture_coordinates[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,     // triangle 2
    };

    float P1_vertices[] = {
       -3.0f, -3.0f,
       3.0f, -3.0f,
       3.0f, 3.0f,  // triangle 1

       -3.0f, -3.0f,
       3.0f, 3.0f,
       -3.0f, 3.0f   // triangle 2
    };

    float P1_texture_coordinates[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,     // triangle 2
    };

    float P2_vertices[] = {
       -3.0f, -3.0f,
       3.0f, -3.0f,
       3.0f, 3.0f,  // triangle 1

       -3.0f, -3.0f,
       3.0f, 3.0f,
       -3.0f, 3.0f   // triangle 2
    };

    float P2_texture_coordinates[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, paddle_one_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, paddle_one_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(g_model_matrix, racketOne_texture_id);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, paddle_two_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, paddle_two_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(e_model_matrix, racketTwo_texture_id);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, ball_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, ball_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(l_model_matrix, birdie_texture_id);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, P1_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, P1_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(p1_model_matrix, P1_texture_id);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, P2_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, P2_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(p2_model_matrix, P2_texture_id);
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}