# facet-theme-halloween

A Halloween theme for [Facet](https://github.com/siakinnik/facet-core) (0.8 or
later): a night sky with a moon and drifting clouds, bats across it, a ghost, a
dead tree and tombstones on the horizon, cobwebs in the corners, jack-o'-lanterns
with flickering candles, and tiles and lists with a neon orange edge.

The module is **only data**: a manifest and one theme file (`halloween.json`),
written at build time with Facet's theme SDK (`facet/theme.h`). No program of
it runs on the device, so it costs nothing while it is not the chosen theme,
and it works on every architecture.

## Install

```bash
curl -fsSL https://raw.githubusercontent.com/siakinnik/facet-core/main/scripts/get.sh | sudo bash -s -- --plugin siakinnik/facet-theme-halloween
```

Then pick **Settings > Theme > Halloween**. The moving parts (bats, clouds,
the ghost, the candles) follow **Settings > Theme animation**: *Automatic*
moves them only when the graphics card draws Facet's interface (the processor
can do it too, but it takes about half a core of a small CPU at 1080p), *On*
and *Off* are what they say.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build        # build/theme-halloween.plugin/ is the module
```

The SDK comes from `../facet-core` if present, else from GitHub. To try it
without installing: `FACET_PLUGIN_PATH=build facet`.

## Make your own

`src/main.cpp` is a complete example of what themes can do: palette, corner
radius, drawings under and over every screen, on every tile and on every list
card, with gradients, glows and numbers that move with time. See
`sdk/include/facet/theme.h` and `docs/ARCHITECTURE.md` section 5.1 in
facet-core.

## License

GPL-3.0 (`LICENSE`).
