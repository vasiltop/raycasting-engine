#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

typedef uint32_t u32;
typedef uint8_t u8;

struct {

    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;

} state ;

#define MAP_SIZE 8
static u8 MAPDATA[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 4, 4, 0, 1,
    1, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 3, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static void render() {

    u32 color = 0xaaaaaa00;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        state.pixels[i] = color;
    }
}

int main(int argc, char *argv[]) {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO), "SDL failed to intitialize: %s\n", SDL_GetError());

    state.window = SDL_CreateWindow("DEMO", 
        SDL_WINDOWPOS_CENTERED_DISPLAY(0), 
        SDL_WINDOWPOS_CENTERED_DISPLAY(0),
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
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

    while(!state.quit) {

        SDL_Event ev;
        while(SDL_PollEvent(&ev)) {
            switch(ev.type) {
                case SDL_QUIT:
                    state.quit = true;
                break;
            }
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