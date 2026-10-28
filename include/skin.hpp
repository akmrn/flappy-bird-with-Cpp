#pragma once

// Every image used by the game is listed here.
// To change a look, change the file name only.
namespace skin {
    inline constexpr const char* bird1 = "Bird-1.png";
    inline constexpr const char* bird2 = "Bird-2.png";

    // Optional images: if a file is missing, a colored rectangle is drawn instead.
    inline constexpr const char* pipe       = "Pipe.png";
    inline constexpr const char* background = "Background.png";
    inline constexpr const char* ground     = "Ground.png";

    // Height (in source pixels) of the pipe cap at the top of the pipe image.
    // The rest of the image is treated as a body that can stretch vertically.
    inline constexpr float pipeCapSourceHeight = 40.0f;
}
