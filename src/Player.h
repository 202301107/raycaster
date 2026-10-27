// The camera/player. Represented as position + direction vector + camera
// plane (perpendicular to direction). See main.cpp / the milestone notes for
// why this representation makes raycasting cheap.
#ifndef RAYCASTER_PLAYER_H
#define RAYCASTER_PLAYER_H

struct Player {
    double posX, posY;      // position in world space
    double dirX, dirY;      // facing direction (unit vector)
    double planeX, planeY;  // camera plane, perpendicular to dir; |plane| sets FOV

    // Rotate both dir and plane by angle `a` (radians, CCW). Applying the same
    // rotation matrix to both keeps the plane perpendicular to dir and keeps
    // its length, so FOV is preserved and the camera turns as a rigid unit.
    void rotate(double a);

    // Move along the facing direction by `dist` (negative = backward).
    void moveForward(double dist);

    // Strafe sideways along the camera plane by `dist` (negative = left).
    void strafe(double dist);
};

// Starting pose: near the east wall of the default map, facing west, 66° FOV
// (|plane| = 0.66 with a unit dir).
Player makeDefaultPlayer();

#endif  // RAYCASTER_PLAYER_H
