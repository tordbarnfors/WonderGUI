# Widget Bench

A wonderapp-based tool for trying out widgets, skins and layouts in
isolation, without wading through one huge main.cpp. Pick a test from the
dropdown at the top of the window; its widget tree is built into the pane
below.

## Adding a test

1. Copy `tests/text_test.cpp` to a new file under `tests/`.
2. Write a `build(TestContext& ctx)` function that fills
   `ctx.pContent->slot` with whatever widget(s) you want to look at.
   `ctx.pAPI` gives you the wonderapp platform API (loading resources,
   dialogs, etc.) if you need it.
3. Call `REGISTER_TEST("Name Shown In Picker", build)` at file scope.
4. Add the new file to `TEST_SOURCES` in `CMakeLists.txt`.

That's the whole workflow - `widgetbench.cpp` and `test.h`/`test.cpp` never
need to change. Tests register themselves at static-init time and show up
in the picker automatically, sorted by name.

See `tests/buttons_test.cpp` for an example that also wires up a message
route (`Base::msgRouter()->addRoute(...)`), which is the pattern for
anything interactive.
