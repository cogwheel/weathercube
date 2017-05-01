#include "weather_types.hpp"
#include <pgmspace.h>

char const * weatherName(Weather weather) {
    switch (weather) {
        case Weather::Cloud: return PSTR("Cloud");
        case Weather::Rain: return PSTR("Rain");
        case Weather::Snow: return PSTR("Snow");
        case Weather::Storm: return PSTR("Storm");
        case Weather::Clear: return PSTR("Clear");
        case Weather::PartCloud: return PSTR("PartCloud");
        case Weather::Lightning: return PSTR("Lightning");
        default: return "Unknown";
    }
}

Weather getWeather(char const * icon) {
    struct IconMapping {
        char const * icon;
        Weather weather;
    };

    static IconMapping const iconMap[] = {
        {PSTR("chanceflurries"), Weather::Snow},
        {PSTR("chancerain"), Weather::Rain},
        {PSTR("chancesleet"), Weather::Snow},
        {PSTR("chancesnow"), Weather::Snow},
        {PSTR("chancetstorms"), Weather::Lightning},
        {PSTR("clear"), Weather::Clear},
        {PSTR("cloudy"), Weather::Cloud},
        {PSTR("flurries"), Weather::Snow},
        {PSTR("fog"), Weather::Cloud},
        {PSTR("hazy"), Weather::Cloud},
        {PSTR("mostlycloudy"), Weather::Cloud},
        {PSTR("mostlysunny"), Weather::PartCloud},
        {PSTR("partlycloudy"), Weather::PartCloud},
        {PSTR("partlysunny"), Weather::PartCloud},
        {PSTR("sleet"), Weather::Rain},
        {PSTR("rain"), Weather::Rain},
        {PSTR("snow"), Weather::Snow},
        {PSTR("sunny"), Weather::Clear},
    };

    for (auto & mapping : iconMap) {
        if (strcmp_P(icon, mapping.icon) == 0) {
            return mapping.weather;
        }
    }
    return Weather::Unknown;
}
