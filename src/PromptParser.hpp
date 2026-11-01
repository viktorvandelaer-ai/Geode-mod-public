#pragma once
#include "Themes.hpp"
#include <string>

// Reads a free-text prompt and modifies theme/density accordingly.
// Returns a short human readable summary of what it understood (empty if nothing).
std::string applyPrompt(std::string const& prompt, Theme& theme, float& density);
