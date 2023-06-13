#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 216
#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

#define dot(v0, v1)                  \
    ({ const v2 _v0 = (v0), _v1 = (v1); (_v0.x * _v1.x) + (_v0.y * _v1.y); })
#define length(v) ({ const v2 _v = (v); sqrtf(dot(_v, _v)); })
#define normalize(u) ({              \
        const v2 _u = (u);           \
        const f32 l = length(_u);    \
        (v2) { _u.x / l, _u.y / l }; \
    })
typedef uint32_t u32;
typedef uint8_t u8;
typedef float f32;
typedef int32_t i32;
typedef struct v2_s {f32 x, y;} v2;
typedef struct v2i_s { i32 x, y; } v2i;

struct {

    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;

    v2 pos, dir, plane;

} state;

#define MAP_SIZE 8
static u8 MAPDATA[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 3, 3, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static void verticalLine(int x, int y0, int y1, u32 color) {
    for (int y = y0; y <= y1; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}


static void render() {
    
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        f32 cameraX = (2 * x / (f32) SCREEN_WIDTH) - 1;

        v2 rayDir = {
            state.dir.x + state.plane.x * cameraX,
            state.dir.y + state.plane.y * cameraX
        };

        v2 pos = state.pos;
        v2i ipos = { (int) pos.x, (int) pos.y };
        
        v2 deltaDist = {
            (rayDir.x == 0) ? 1e30 : fabsf(1 / rayDir.x),
            (rayDir.y == 0) ? 1e30 : fabsf(1 / rayDir.y),
        };

        f32 perpWallDist;
        v2 sideDist;

        v2i step;
        int hit = 0;
        int side;
        
        if(rayDir.x < 0) {
            step.x = -1;
            sideDist.x = (pos.x - ipos.x) * deltaDist.x;
        } else {
            step.x = 1;
            sideDist.x = (ipos.x + 1.0 - pos.x) * deltaDist.x;
        }

        if(rayDir.y < 0) {
            step.y = -1;
            sideDist.y = (pos.y - ipos.y) * deltaDist.y;
        } else {
            step.y = 1;
            sideDist.y = (ipos.y + 1.0 - pos.y) * deltaDist.y;
        }

        while (hit == 0) {
            if(sideDist.x < sideDist.y) {
                sideDist.x += deltaDist.x;
                ipos.x += step.x;
                side = 0;
            } else {
                sideDist.y += deltaDist.y;
                ipos.y += step.y;
                side = 1;
            }

            ASSERT(
                ipos.x >= 0
                && ipos.x < MAP_SIZE
                && ipos.y >= 0
                && ipos.y < MAP_SIZE,
                "DDA out of bounds");

            hit = MAPDATA[ipos.y * MAP_SIZE + ipos.x];
        }

        u32 color;
        switch (hit) {
        case 1: color = 0xFF0000FF; break;
        case 2: color = 0xFF00FF00; break;
        case 3: color = 0xFFFF0000; break;
        case 4: color = 0xFFFF00FF; break;
        };

        if(side == 0) {
            perpWallDist = (sideDist.x - deltaDist.x);
        } else {
            perpWallDist = (sideDist.y - deltaDist.y);
        }
 
        int lineHeight = (int) (SCREEN_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
        
        verticalLine(x, drawStart, drawEnd, color);
        
    }
}

int main(int argc, char *argv[]) {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO), "SDL failed to intitialize: %s\n", SDL_GetError());

    state.window = SDL_CreateWindow("DEMO", 
        SDL_WINDOWPOS_CENTERED_DISPLAY(0), 
        SDL_WINDOWPOS_CENTERED_DISPLAY(0),
        1600,
        900,
        SDL_WINDOW_ALLOW_HIGHDPI);

    ASSERT(state.window, "failed to create SDL window: %s\n", SDL_GetError());

    state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
    ASSERT(state.renderer,"failed to create SDL renderer: %s\n", SDL_GetError());

    state.texture = SDL_CreateTexture(
        state.renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);

    ASSERT(state.texture, "failed to create SDL texture: %s\n", SDL_GetError());


    state.pos = (v2) { 2, 2 };
    state.dir = normalize(((v2) { -1.0f, 0.1f }));
    state.plane = (v2) { 0.0f, 0.66f };

    while(!state.quit) {

        SDL_Event ev;
        while(SDL_PollEvent(&ev)) {
            switch(ev.type) {
                case SDL_QUIT:
                    state.quit = true;
                break;
            }
        }
        const f32 rotspeed = 1.0f * 0.016f, movespeed = 3.0f * 0.016f;

        const u8 *keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_LEFT]) {
            v2 d = state.dir, p = state.plane;
            state.dir.x = d.x * cos(rotspeed) - d.y * sin(rotspeed);
            state.dir.y = d.x * sin(rotspeed) + d.y * cos(rotspeed);
            state.plane.x = p.x * cos(rotspeed) - p.y * sin(rotspeed);
            state.plane.y = p.x * sin(rotspeed) + p.y * cos(rotspeed);
        }

        if (keystate[SDL_SCANCODE_RIGHT]) {
            v2 d = state.dir, p = state.plane;
            state.dir.x = d.x * cos(-rotspeed) - d.y * sin(-rotspeed);
            state.dir.y = d.x * sin(-rotspeed) + d.y * cos(-rotspeed);
            state.plane.x = p.x * cos(-rotspeed) - p.y * sin(-rotspeed);
            state.plane.y = p.x * sin(-rotspeed) + p.y * cos(-rotspeed);
        }
   

        if (keystate[SDL_SCANCODE_UP]) {
            state.pos.x += state.dir.x * movespeed;
            state.pos.y += state.dir.y * movespeed;
        }

        if (keystate[SDL_SCANCODE_DOWN]) {
            state.pos.x -= state.dir.x * movespeed;
            state.pos.y -= state.dir.y * movespeed;
        }

        memset(state.pixels, 0, sizeof(state.pixels));
        render();

        SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(
            state.renderer,
            state.texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    return 0;
    
}