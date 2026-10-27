// The DDA raycaster: cast a ray through the grid and report the first wall hit.
#ifndef RAYCASTER_RAYCASTER_H
#define RAYCASTER_RAYCASTER_H

#include "Map.h"

struct RayHit {
    double perpDist;  // perpendicular distance to the wall (see Milestone 6)
    int mapX, mapY;   // grid cell that was hit
    int side;         // 0 = crossed a vertical grid line (E/W-facing wall)
                      // 1 = crossed a horizontal grid line (N/S-facing wall)
    int wallValue;    // map cell value at the hit (selects texture/color)
};

// Cast a ray from (posX, posY) in direction (rayDirX, rayDirY) through the map
// using DDA. rayDir need not be normalized; perpDist is returned in the same
// scale, i.e. the hit point is pos + perpDist * rayDir.
RayHit castRay(const Map& map, double posX, double posY, double rayDirX,
               double rayDirY);

#endif  // RAYCASTER_RAYCASTER_H
