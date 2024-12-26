#include "SDL.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

// Compile with:
// g++ multi_drag.cpp -o multi_drag $(pkg-config --cflags --libs SDL2)
// g++ multi_drag.cpp -o multi_drag -lSDL2

// Run with:
// ./multi_drag

class SDLApp
{
private:
  static constexpr int WINDOW_WIDTH = 800;
  static constexpr int WINDOW_HEIGHT = 600;

  struct SDL_Deleter
  {
    void operator()(SDL_Window *w) const { SDL_DestroyWindow(w); }
    void operator()(SDL_Renderer *r) const { SDL_DestroyRenderer(r); }
  };

  std::unique_ptr<SDL_Window, SDL_Deleter> window;
  std::unique_ptr<SDL_Renderer, SDL_Deleter> renderer;

  class DraggableObject
  {
  public:
    SDL_Rect rect;
    SDL_Color color;
    bool isDragging;
    int dragOffsetX;
    int dragOffsetY;

    DraggableObject(int x, int y, int w, int h, SDL_Color c)
        : isDragging(false), dragOffsetX(0), dragOffsetY(0), color(c)
    {
      rect.x = x;
      rect.y = y;
      rect.w = w;
      rect.h = h;
    }

    bool containsPoint(int x, int y) const
    {
      return x >= rect.x && x < rect.x + rect.w && y >= rect.y &&
             y < rect.y + rect.h;
    }

    void drag(int mouseX, int mouseY)
    {
      if (isDragging)
      {
        rect.x = mouseX - dragOffsetX;
        rect.y = mouseY - dragOffsetY;

        // Keep within window bounds
        rect.x = std::clamp(rect.x, 0, WINDOW_WIDTH - rect.w);
        rect.y = std::clamp(rect.y, 0, WINDOW_HEIGHT - rect.h);
      }
    }
  };

  class ObjectManager
  {
  private:
    std::vector<DraggableObject> objects;
    std::mt19937 rng;
    std::uniform_int_distribution<int> colorDist;

  public:
    ObjectManager() : rng(std::random_device{}()), colorDist(0, 255)
    {
      // Create some initial objects
      addObject(100, 100);
      addObject(300, 200);
      addObject(500, 300);
    }

    SDL_Color generateRandomColor()
    {
      return SDL_Color{static_cast<Uint8>(colorDist(rng)),
                       static_cast<Uint8>(colorDist(rng)),
                       static_cast<Uint8>(colorDist(rng)), 255};
    }

    void addObject(int x, int y)
    {
      SDL_Color color = generateRandomColor();
      objects.emplace_back(x, y, 80, 80, color);
    }

    void handleMouseDown(const SDL_MouseButtonEvent &event)
    {
      int mouseX = event.x;
      int mouseY = event.y;

      if (event.button == SDL_BUTTON_LEFT)
      {
        // Check objects in reverse order (top to bottom)
        for (auto it = objects.rbegin(); it != objects.rend(); ++it)
        {
          if (it->containsPoint(mouseX, mouseY))
          {
            it->isDragging = true;
            it->dragOffsetX = mouseX - it->rect.x;
            it->dragOffsetY = mouseY - it->rect.y;

            // Move clicked object to front
            DraggableObject obj = *it;
            objects.erase(std::next(it).base());
            objects.push_back(obj);

            return;
          }
        }
        // If no object was clicked, create a new one
        addObject(mouseX, mouseY);
      }
    }

    void handleMouseUp(const SDL_MouseButtonEvent &event)
    {
      if (event.button == SDL_BUTTON_LEFT)
      {
        for (auto &obj : objects)
        {
          obj.isDragging = false;
        }
      }
    }

    void handleMouseMotion(const SDL_MouseMotionEvent &event)
    {
      for (auto &obj : objects)
      {
        obj.drag(event.x, event.y);
      }
    }

    const std::vector<DraggableObject> &getObjects() const { return objects; }
  };

  ObjectManager objectManager;
  bool running;

public:
  SDLApp() : running(true)
  {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      throw std::runtime_error(std::string("SDL initialization failed: ") +
                               SDL_GetError());
    }

    window.reset(SDL_CreateWindow("Multiple Draggable Objects Demo",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                                  WINDOW_HEIGHT, SDL_WINDOW_SHOWN));

    if (!window)
    {
      throw std::runtime_error(std::string("Window creation failed: ") +
                               SDL_GetError());
    }

    renderer.reset(
        SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

    if (!renderer)
    {
      throw std::runtime_error(std::string("Renderer creation failed: ") +
                               SDL_GetError());
    }
  }

  ~SDLApp() { SDL_Quit(); }

  void handleEvents()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_MOUSEBUTTONDOWN:
        objectManager.handleMouseDown(event.button);
        break;
      case SDL_MOUSEBUTTONUP:
        objectManager.handleMouseUp(event.button);
        break;
      case SDL_MOUSEMOTION:
        objectManager.handleMouseMotion(event.motion);
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
          running = false;
        }
        break;
      }
    }
  }

  void render()
  {
    // Clear screen
    SDL_SetRenderDrawColor(renderer.get(), 240, 240, 240, 255);
    SDL_RenderClear(renderer.get());

    // Draw all objects
    for (const auto &obj : objectManager.getObjects())
    {
      SDL_SetRenderDrawColor(renderer.get(), obj.color.r, obj.color.g,
                             obj.color.b, obj.color.a);
      SDL_RenderFillRect(renderer.get(), &obj.rect);

      // Draw border
      SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
      SDL_RenderDrawRect(renderer.get(), &obj.rect);
    }

    SDL_RenderPresent(renderer.get());
  }

  void run()
  {
    while (running)
    {
      handleEvents();
      render();
      SDL_Delay(16); // Cap at roughly 60 FPS
    }
  }
};

int main(int argc, char *argv[])
{
  try
  {
    SDLApp app;
    app.run();
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
