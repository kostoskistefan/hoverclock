# Hoverclock

Hoverclock is a simple, yet customizable floating clock for Linux. It is a very lightweight alternative to [Hoverclock-QT](https://github.com/kostoskistefan/hoverclock-qt) with around 1500% better memory efficiency. It is built around the XCB library and uses the FontConfig and Freetype libraries for font rendering.

## Configuration

The configuration file for hoverclock is located at `$HOME/.config/hoverclock/hoverclock.vault`.

### Settings description

| Setting                           | Description                                                                                           |
| --------------------------------- | ----------------------------------------------------------------------------------------------------- |
| window_location_name              | The location of the clock - See [Window location name setting](#window-location-name-setting)         |
| window_vertical_margin            | The vertical space from the top or bottom edge of the screen                                          |
| window_horizontal_margin          | The horizontal space from the top or bottom edge of the screen                                        |
| window_background_color           | The background color of the window - See [Color settings](#color-settings)                            |
| time_label_visible                | Whether to show or hide the time label                                                                |
| time_label_format                 | The time format used by the time label - See [Time and date format](#time-and-date-format-settings)   |
| time_label_font_name              | The name of the font used by the time label - See [Font name](#font-name-settings)                    |
| time_label_text_color             | The color of the time label text - See [Color settings](#color-settings)                              |
| time_label_shadow_color           | The color of the time label text shadow - See [Color settings](#color-settings)                       |
| date_label_visible                | Whether to show or hide the date label                                                                |
| date_label_format                 | The date format used by the date label - See [Time and date format](#time-and-date-format-settings)   |
| date_label_font_name              | The name of the font used by the date label - See [Font name](#font-name-settings)                    |
| date_label_text_color             | The color of the date label text - See [Color settings](#color-settings)                              |
| date_label_shadow_color           | The color of the date label text shadow - See [Color settings](#color-settings)                       |
| vertical_spacing_between_labels   | The vertical spacing to add between the time and date labels                                          | 

#### Window location name setting

This setting consists of 2 strings separated by a whitespace character. The first one is the vertical position, the second one is the horizontal position on the screen. The vertical position can have a value of `top` or `bottom`, while the horizontal position can have a value of `left` or `right`. All valid values for this setting are:

 * `top left`
 * `top right`
 * `bottom left`
 * `bottom right`

#### Time and date format settings

The time and date labels require a format string which can be understood by the strftime function in C. All valid values for the format string can be found on the [following link](https://en.cppreference.com/w/c/chrono/strftime#Format_string). 

#### Color settings

All of the settings that represent a color are defined in an ARGB format. Each color needs to start with a `0x` prefix (to represent a hex value). 

| Prefix | Alpha component range | Red component range | Green component range | Blue component range |
| ------ | --------------------- | ------------------- | --------------------- | -------------------- |
| 0x     | 00 - ff               | 00 - ff             | 00 - ff               | 00 - ff              |

Examples: 

 * Fully opaque red color: `0xffff0000`
 * Fully opaque green color: `0xff00ff00`
 * Fully opaque blue color: `0xff0000ff`
 * Semi transparent red color: `0x88ff0000`
 * Semi transparent green color: `0x8800ff00`
 * Semi transparent blue color: `0x880000ff`

#### Font name settings

The font name settings do not only set the name of the font, but the font attributes as well. All wanted font attributes can be appended to the font name using a colon as a separator.

Examples:

 * Regular Roboto font: `Roboto` 
 * Bold Roboto font: `Roboto:bold` 
 * Italic Roboto font: `Roboto:italic` 
 * Bold Italic Roboto font: `Roboto:bold:italic` 

## Dependencies

### Runtime dependencies 

 * An X Server
 * libxcb
 * xcb-shape
 * fontconfig
 * freetype2
 * [Vault](https://github.com/kostoskistefan/vault)
 * [Insight](https://github.com/kostoskistefan/insight)
 * [Inthreadval](https://github.com/kostoskistefan/inthreadval)

The last three dependencies are libraries that I've built in the past and are packed inside this project. You do **NOT** need to download and install them separatelly.

### Build dependencies

 * GCC
 * Meson

## Build from source

 1. Clone this repository: `git clone https://github.com/kostoskistefan/hoverclock.git`
 2. Go into the cloned directory: `cd hoverclock`
 3. Let Meson setup its build directory: `meson setup build`
 4. Take a look at Meson's output and check if any dependencies are missing. If so, install them using the package manager on your Linux distro of choice.
 5. Use meson to compile hoverclock: `meson compile -C build`
 6. Hoverclock is ready and can be run with the following command (assuming you are still in the root of cloned directory): `./build/hoverclock`

## Contributing

Any contributions to this project are most welcome!

For any suggestions, feature requests, bug reports and questions, feel free to open an issue in this repository's [issue tracker](https://github.com/kostoskistefan/hoverclock/issues).

Pull requests are also encouraged! Do not hesitate to fork this repository, make changes to the code and submit a pull request. Make sure to follow the coding style as much as possible, to make this project as consistent as possible.
