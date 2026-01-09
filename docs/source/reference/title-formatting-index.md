# Title formatting index

This page contains an index of all title formatting functions and fields
available in Columns UI and not available in the Default User Interface.

## Functions

| Name                        | Reference link                                      | Supported contexts                                 |
| --------------------------- | --------------------------------------------------- | -------------------------------------------------- |
| `$blend()`                  | [Text styling](/other/text-styling)                 | Built-in panels, status bar and status pane        |
| `$calculate_blend_target()` | [Style script](/playlist-view/style-script)         | Playlist view (style scripts)                      |
| `$hsl()`                    | [Text styling](/other/text-styling)                 | Built-in panels, status bar and status pane        |
| `$offset_colour()`          | [Style script](/playlist-view/style-script)         | Playlist view (style scripts)                      |
| `$get_global()`             | [Global variables](/playlist-view/global-variables) | Playlist view (display, style and sorting scripts) |
| `$set_font()`               | [Text styling](/other/text-styling)                 | Item details                                       |
| `$set_format()`             | [Text styling](/other/text-styling)                 | Built-in panels, status bar and status pane        |
| `$set_global()`             | [Global variables](/playlist-view/global-variables) | Playlist view (global variable script)             |
| `$set_style()`              | [Style script](/playlist-view/style-script)         | Playlist view (style scripts)                      |
| `$reset_font()`             | [Text styling](/other/text-styling)                 | Item details                                       |
| `$reset_format()`           | [Text styling](/other/text-styling)                 | Built-in panels, status bar and status pane        |
| `$rgb()`                    | [Text styling](/other/text-styling)                 | Built-in panels, status bar and status pane        |
| `$transition()`             | [Text styling](/other/text-styling)                 | Built-in panels, status bar and status pane        |

## Fields

### Item details

| Field                 | Reference link                      |
| --------------------- | ----------------------------------- |
| `%default_font_face%` | [Text styling](/other/text-styling) |
| `%default_font_size%` | [Text styling](/other/text-styling) |

### Playlist switcher

| Field                 | Reference link                                          |
| --------------------- | ------------------------------------------------------- |
| `%default_font_size%` | [Text styling](/other/text-styling)                     |
| `%is_active%`         | [Title formatting](/playlist-switcher/title-formatting) |
| `%is_locked%`         | [Title formatting](/playlist-switcher/title-formatting) |
| `%is_playing%`        | [Title formatting](/playlist-switcher/title-formatting) |
| `%filesize%`          | [Title formatting](/playlist-switcher/title-formatting) |
| `%filesize_raw%`      | [Title formatting](/playlist-switcher/title-formatting) |
| `%lock_name%`         | [Title formatting](/playlist-switcher/title-formatting) |
| `%size%`              | [Title formatting](/playlist-switcher/title-formatting) |
| `%title%`             | [Title formatting](/playlist-switcher/title-formatting) |
| `%length%`            | [Title formatting](/playlist-switcher/title-formatting) |

### Playlist view

| Name                        | Reference link                                      | Supported contexts                          |
| --------------------------- | --------------------------------------------------- | ------------------------------------------- |
| `%_back%`                   | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_display_index%`          | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%default_font_size%`       | [Text styling](/other/text-styling)                 | Display and global variables scripts        |
| `%_is_group%`               | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_is_themed%`              | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%playlist_name%`           | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |
| `%_text%`                   | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_selected_back%`          | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_selected_back_no_focus%` | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_selected_text%`          | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_selected_text_no_focus%` | [Style script](/playlist-view/style-script)         | Style scripts                               |
| `%_system_day%`             | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |
| `%_system_day_of_week%`     | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |
| `%_system_hour%`            | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |
| `%_system_julian_day%`      | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |
| `%_system_month%`           | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |
| `%_system_year%`            | [Title formatting](/playlist-view/title-formatting) | Display, style and global variables scripts |

### Status bar

| Field                 | Reference link                      |
| --------------------- | ----------------------------------- |
| `%default_font_size%` | [Text styling](/other/text-styling) |

### Status pane

| Field                 | Reference link                      |
| --------------------- | ----------------------------------- |
| `%default_font_size%` | [Text styling](/other/text-styling) |
