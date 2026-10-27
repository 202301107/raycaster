#include "Raycaster.h"

#include <cmath>

RayHit castRay(const Map& map, double posX, double posY, double rayDirX,
               double rayDirY) {
    // Which cell we start in.
    int mapX = static_cast<int>(posX);
    int mapY = static_cast<int>(posY);

    // Ray length needed to cross one full cell in x / in y. A zero direction
    // component means the ray never crosses that family of grid lines, so we
    // make its cost effectively infinite and that axis is never chosen.
    const double deltaDistX =
        (rayDirX == 0.0) ? 1e30 : std::fabs(1.0 / rayDirX);
    const double deltaDistY =
        (rayDirY == 0.0) ? 1e30 : std::fabs(1.0 / rayDirY);

    // Step direction through cells, and distance from the start to the first
    // grid line of each kind (depends on the fractional position in the cell).
    int stepX, stepY;
    double sideDistX, sideDistY;

    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
    }

    // Step to the nearer grid line each iteration until we land in a wall cell.
    int side = 0;
    while (!map.isWall(mapX, mapY)) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }
    }

    // sideDist* overshot to the *next* line of that kind; back off one step to
    // get the distance to the line we actually hit.
    const double perpDist =
        (side == 0) ? sideDistX - deltaDistX : sideDistY - deltaDistY;

    return RayHit{perpDist, mapX, mapY, side, map.at(mapX, mapY)};
}
