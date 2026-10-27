#include "Player.h"

#include <cmath>

void Player::rotate(double a) {
    const double cosA = std::cos(a);
    const double sinA = std::sin(a);

    // 2D rotation matrix applied to dir...
    const double oldDirX = dirX;
    dirX = dirX * cosA - dirY * sinA;
    dirY = oldDirX * sinA + dirY * cosA;

    // ...and the SAME matrix applied to plane, keeping it perpendicular.
    const double oldPlaneX = planeX;
    planeX = planeX * cosA - planeY * sinA;
    planeY = oldPlaneX * sinA + planeY * cosA;
}

void Player::moveForward(double dist) {
    posX += dirX * dist;
    posY += dirY * dist;
}

void Player::strafe(double dist) {
    // The plane vector points to the player's right, so moving along it strafes
    // sideways. (plane is not unit-length, but |plane| is constant, so this is
    // a consistent strafe speed.)
    posX += planeX * dist;
    posY += planeY * dist;
}

Player makeDefaultPlayer() {
    Player p;
    p.posX = 22.0;
    p.posY = 12.0;
    p.dirX = -1.0;  // facing west
    p.dirY = 0.0;
    p.planeX = 0.0;  // plane perpendicular to dir, length 0.66 -> ~66 deg FOV
    p.planeY = 0.66;
    return p;
}
