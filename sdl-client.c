#include <stdlib.h>
#include <unistd.h>
#include <SDL3/SDL.h>

int main()
{
    char *png_path = NULL;
    SDL_Surface *png_tmp = NULL, *shared_contents = NULL;
    SDL_SharedSurface *shared_surface = NULL;
    SDL_IPC *ipc = SDL_GetParentIPC();
    ssize_t size;

    if (!ipc) {
        SDL_Log("Couldn't get parent IPC");
        return EXIT_FAILURE;
    }

    /*
     * We are only creating an SDL_Surface to load the PNG.
     * Once we have loaded the PNG, we then create a SharedSurface with its
     * properties, and copy the pixel data over.
     *
     * The shared surface will allow us to send the pixel data to
     * the parent process that spawned this process.
     */
    SDL_asprintf(&png_path, "%sSDL/test/sample.png", SDL_GetBasePath());  /* allocate a string of the full file path */
    png_tmp = SDL_LoadPNG(png_path);
    if (!png_tmp) {
        SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    shared_surface = SDL_CreateSharedSurface(png_tmp->w, png_tmp->h, png_tmp->format);
    if (!shared_surface) {
        SDL_Log("Couldn't allocate shared surface: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    shared_contents = SDL_WriteLockSharedSurface(shared_surface);
    if (!shared_contents) {
        SDL_Log("Couldn't lock shared surface for writing");
        return EXIT_FAILURE;
    }

    size = png_tmp->h * png_tmp->pitch;
    SDL_memcpy(shared_contents->pixels, png_tmp->pixels, size);
    SDL_DestroySurface(png_tmp);
    SDL_UnlockSharedSurface(shared_surface);

    SDL_SendSharedSurface(ipc, shared_surface);

    // pause();
    SDL_DestroySharedSurface(shared_surface);
}
