# Adding a new application

Apps are the easiest thing to add to this OS — the whole point of the
`App` interface (`gui/window.h`) is that a new app never needs to touch
the compositor, window manager, or any other app's code.

## The interface

```cpp
class App {
public:
    virtual ~App() {}
    virtual void draw(int x, int y, int w, int h) = 0;   // required
    virtual void on_key(char c) {}                        // optional
    virtual void on_click(int local_x, int local_y) {}    // optional
    virtual const char* title() const = 0;                // required
};
```

- `draw` is called once per frame. `(x, y)` is the top-left of your
  app's content area (the window frame and title bar are already drawn
  for you); `(w, h)` is how much space you have. Use `fb::` drawing
  functions from `drivers/framebuffer.h` (`fill_rect`, `draw_rect`,
  `draw_line`, `draw_string`, ...).
- `on_key` fires for each typed character while your window has focus.
- `on_click` fires on left-click inside your content area, with
  coordinates relative to your content area's top-left.
- `title` is shown in the title bar.

## Steps

1. **Create `apps/yourapp.h`** declaring your class and a factory
   function:
   ```cpp
   #pragma once
   #include "gui/window.h"

   class YourApp : public App {
   public:
       void draw(int x, int y, int w, int h) override;
       const char* title() const override { return "Your App"; }
   };

   App* create_yourapp();
   ```

2. **Create `apps/yourapp.cpp`** with the implementation. Copy
   `apps/settings.cpp` as a minimal starting template, or
   `apps/terminal.cpp` for a more complete example with keyboard input
   and scrolling text.

3. **Register it in `gui/desktop.cpp`**:
   ```cpp
   #include "apps/yourapp.h"
   // ...
   taskbar::add_launcher("Your App", create_yourapp);
   ```
   That's the *only* existing file you need to touch. Everything else —
   window creation, dragging, closing, focus, drawing order — is handled
   automatically by the compositor.

4. **Add the new source file to the build** — nothing to do! The
   Makefile globs every `.cpp` under `apps/` automatically.

## Tips

- Keep per-instance state as member variables (see `Terminal`'s
  `history`/`inputLine`) — each window owns its own `App` instance, so
  opening the same app twice gives two independent instances for free.
- The font (`gui/font8x8.h`) currently covers uppercase letters, digits,
  space, and common punctuation; lowercase input is automatically
  upper-cased by `fb::draw_char`. If you need more glyphs (e.g. accented
  characters), add entries to the `font8x8` table following the existing
  8-row-of-8-bits format.
- For anything that needs real persistent storage (saving files, etc.),
  see `docs/ROADMAP.md`'s filesystem section — that's the missing piece,
  not the app framework.
