# epd

C++ ePaper display driver and graphics library for ESP32 and STM32

## Features
- Neatly separated device-specific transport layer, panel drivers and graphics engine
- Easy to port to new device targets by implementing a single interface
- Support for partial display refresh (depends on the specific panel)
- Graphics engine similiar to Adafruits GFX library, which supports:
    - Drawing primitives such as lines, rectangles and circles
    - Rotated virtual screen
    - Text rendering and measurement
    - Partial screen rendering based on partial panel refresh, if available

## Supported panels

### Dalian GoodDisplay

| Panel | Size | Resolution | Color | Partial Refresh | Controller | Driver implementation
| --- | --- | --- | --- | --- | --- | --- |
| GDEY037T03 | 3.7" | 240x416 | monochrome | yes | UC8253 | `epd_panel_GDEY037T03.hxx`

## How to use

### Adding the library into your firmware project
The repository contains various device-specific implementions of this library, pick the one that matches your microcontroller - e.g. for ESP32, pick `epd-esp32` and follow the device-specific instructions below.

#### ESP32 - `epd-esp32`
First, reference the `/epd-esp32` folder as an additional source for ESP-IDF components in your projects top-level `CMakeLists.txt` file:

```
set(EXTRA_COMPONENT_DIRS
    "path_to_repo/epd-esp32"
    ${extra_components_dirs_append})
```

Then, in every subcomponent of your project that wants to use `epd`, specify the `epd-esp32` component as a dependency in the components `CMakeLists.txt` file:

```
idf_component_register(
    SRCS
        ...
    INCLUDE_DIRS
        ...
    REQUIRES
        "epd-esp32"
)
```

### Using the library
In this case, the ESP32 device-specific implemention will be used for demonstration purposes.

In order to use `epd` to control an ePaper display, first check that your specific panel model is supported by the library.


#### Initializing the library
The first step is to instantiate the device-specific transport object matching the microcontroller you are using:

```
epd::esp32::pinmap epaperPins{
        .busy_pin = EPAPER_PIN_NUM_BUSY,
        .reset_pin = EPAPER_PIN_NUM_RST,
        .dc_pin = EPAPER_PIN_NUM_DC,
        .cs_pin = EPAPER_PIN_NUM_CS
    };
epd::esp32::transport transport{epaperPins, EPAPER_HOST};
```

With the transport created, you can setup the driver object for your specific panel. In this case we are using a monochrome `GDEY037T03` panel made by Dalian GoodDisplay:

```
epd::panels::GDEY037T03 panel{&transport};

status = panel.initialize();
ESP_ERROR_CHECK(status);
```

Finally, to make use of the graphics engine provided by `epd`, wrap the panel driver object in an instance of the `epd::graphics` class - optionally specifying a screen rotation:

```
epd::graphics graphics{&panel, epd::rotation::by_90deg};
```

#### Full display refresh

In order to render content to the display and perform a full display refresh, start with performing some drawing operations:

```
graphics.clear(epd::color::white);

graphics.draw_line(
    epd::position{0, 0}, 
    epd::position{25, 25}, 
    epd::color::black
);

graphics.draw_rect(
    epd::rectangle{ epd::position{25, 55}, epd::size{75, 45} }, 
    epd::color::black
);
```

Then, ask the graphics engine to perform an update:

```
result = graphics.display();
ESP_ERROR_CHECK(result);
```

#### Partial display refresh

To re-render only a part of the screen contents and to perform a partial display refresh, you need to first complete a full render and refresh as outlined in the previous section.

On the next up to six frames, you can then do a partial refresh to safe time and avoid flickering on the panel.

To do so, first clear the screen only in the area you want to update, and re-render the content of that area:

```
epd::rectangle updateArea{ ... };

graphics.clear_partial(updateArea, epd::color::white);

graphics.fill_rect(updateArea, epd::color::black);
```

Afterwards, ask the graphics engine to perform a partial update:

```
result = graphics.display_partial(updateArea);
ESP_ERROR_CHECK(result);
```

## AI usage disclaimer
No generative AI was used in writing this code. I'd rather write shitty code myself than let AI do all the work for me.