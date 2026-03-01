#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static int texture_width = 0;
static int texture_height = 0;
static SDL_Process *process;
static SDL_SharedSurface *shared_surface;
static SDL_Texture *texture;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

static SDL_Process *createProcessWithIPC(const char * const *args)
{
    SDL_PropertiesID props = SDL_CreateProperties();
    if (!props)
        return NULL;

    SDL_SetBooleanProperty(props, SDL_PROP_PROCESS_CREATE_SDL_IPC, true);
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, (void *)args);
    return SDL_CreateProcessWithProperties(props);
}

SDL_AppResult SDL_AppIterate(void *appstore)
{
    SDL_FRect dst_rect = {
        .x = 0.0f,
        .y = 0.0f,
        .w = texture_width,
        .h = texture_height,
    };
    SDL_RenderTexture(renderer, texture, NULL, &dst_rect);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstore, int argc, char **argv)
{
    static const char *args[] = {
        "./sdl-client",
        NULL,
    };

    SDL_SharedSurface *shared_surface = NULL;
    SDL_Surface *surface = NULL;

    process = createProcessWithIPC(args);
    if (!process) {
        SDL_Log("createProcess: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_IPC *ipc = SDL_GetProcessIPC(process);
    SDL_SharedResource resource = SDL_ReceiveSharedResource(ipc);

    if (resource.type != SDL_SHARED_SURFACE) {
        SDL_Log("resourceType: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_SUCCESS;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroySharedSurface(shared_surface);
    SDL_DestroyProcess(process);
}
