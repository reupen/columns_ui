#include "pch.h"
#include "dark_mode_trackbar.h"

#include "dark_mode.h"

namespace cui::dark {

uih::TrackbarCustomColours get_dark_trackbar_colours()
{
    return {get_dark_colour(ColourID::TrackbarChannel), get_dark_colour(ColourID::TrackbarThumb),
        get_dark_colour(ColourID::TrackbarHotThumb), get_dark_colour(ColourID::TrackbarDisabledThumb)};
}

} // namespace cui::dark
