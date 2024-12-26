
// This program demonstrates handling mouse events in SDL2.
// It creates a window with a blue rectangle that can be dragged around.
// The rectangle can be dragged by clicking and holding the left mouse button
// on it and moving the mouse.

// Compile with:
// gcc mouse_demo.c -o mouse_demo $(pkg-config --cflags --libs SDL2)
// gcc mouse_demo.c -o mouse_demo -lSDL2

// Run with:
// ./mouse_demo

// Press 'X' on the window or 'Ctrl+C' to quit.

// Include the SDL2 library

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct
{
  int x;
  int y;
  bool isDragging;
  SDL_Rect rect; // Rectangle that can be dragged
} MouseState;

void initMouseState(MouseState *state)
{
  state->isDragging = false;
  state->rect.x = WINDOW_WIDTH / 2 - 50; // Center the rectangle
  state->rect.y = WINDOW_HEIGHT / 2 - 50;
  state->rect.w = 100;
  state->rect.h = 100;
}

bool handleMouseEvent(SDL_Event *event, MouseState *state)
{
  switch (event->type)
  {
  case SDL_MOUSEBUTTONDOWN:
    if (event->button.button == SDL_BUTTON_LEFT)
    {
      int mouseX = event->button.x;
      int mouseY = event->button.y;

      // Check if click is inside the rectangle
      if (mouseX >= state->rect.x && mouseX <= state->rect.x + state->rect.w &&
          mouseY >= state->rect.y && mouseY <= state->rect.y + state->rect.h)
      {
        state->isDragging = true;
        state->x = mouseX - state->rect.x; // Store offset
        state->y = mouseY - state->rect.y;
        printf("Started dragging at (%d, %d)\n", mouseX, mouseY);
      }
      else
      {
        printf("Clicked at (%d, %d)\n", mouseX, mouseY);
      }
    }
    break;

  case SDL_MOUSEBUTTONUP:
    if (event->button.button == SDL_BUTTON_LEFT)
    {
      if (state->isDragging)
      {
        state->isDragging = false;
        printf("Stopped dragging at (%d, %d)\n", event->button.x,
               event->button.y);
      }
    }
    break;

  case SDL_MOUSEMOTION:
    if (state->isDragging)
    {
      // Update rectangle position while maintaining offset
      state->rect.x = event->motion.x - state->x;
      state->rect.y = event->motion.y - state->y;

      // Keep rectangle within window bounds
      if (state->rect.x < 0)
        state->rect.x = 0;
      if (state->rect.y < 0)
        state->rect.y = 0;
      if (state->rect.x + state->rect.w > WINDOW_WIDTH)
        state->rect.x = WINDOW_WIDTH - state->rect.w;
      if (state->rect.y + state->rect.h > WINDOW_HEIGHT)
        state->rect.y = WINDOW_HEIGHT - state->rect.h;
    }
    break;
  }
  return true;
}

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL initialization failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow(
      "SDL Mouse Events Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

  if (!window)
  {
    printf("Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    printf("Renderer creation failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  MouseState mouseState;
  initMouseState(&mouseState);

  bool running = true;
  SDL_Event event;

  while (running)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
      }
      handleMouseEvent(&event, &mouseState);
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Draw draggable rectangle
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &mouseState.rect);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
