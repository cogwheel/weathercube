#pragma once

enum class Weather {
    Cloud,
    Rain,
    Snow,
    Storm,
    Clear,
    PartCloud,
    Lightning,
    Unknown
};

// Gets the string name of the weather enum value
char const * weatherName(Weather weather);

// Gets the weather enum value for a WeatherUnderground icon
Weather getWeather(char const * icon);
