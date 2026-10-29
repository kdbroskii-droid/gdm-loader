#include "autoplay_planner.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace judsz::autoplay {
namespace {

bool valid_box(const Hitbox& box) {
    return std::isfinite(box.x) && std::isfinite(box.y) &&
           std::isfinite(box.width) && std::isfinite(box.height) &&
           box.width > 0.0f && box.height > 0.0f;
}

Hitbox predict_obstacle_box(const Obstacle& obstacle, float t) {
    return {obstacle.hitbox.x + obstacle.velocity_x * t,
            obstacle.hitbox.y + obstacle.velocity_y * t,
            obstacle.hitbox.width, obstacle.hitbox.height};
}

bool overlaps(const Hitbox& a, const Hitbox& b) {
    return a.x < b.x + b.width && a.x + a.width > b.x &&
           a.y < b.y + b.height && a.y + a.height > b.y;
}

Hitbox predict_box(const PlayerState& p, float t, bool jump) {
    const float vy0 = jump ? p.jump_velocity : p.vertical_velocity;
    const float y = p.hitbox.y + vy0 * t + 0.5f * p.gravity * t * t;
    return {p.hitbox.x + p.horizontal_speed * t, y,
            p.hitbox.width, p.hitbox.height};
}

bool path_is_clear(const PlayerState& p, const Obstacle* obstacles,
                   std::size_t count, float horizon, float step, bool jump,
                   uint32_t* first_id, float* first_time) {
    for (float t = 0.0f; t <= horizon + step * 0.5f; t += step) {
        const Hitbox player_box = predict_box(p, t, jump);
        for (std::size_t i = 0; i < count; ++i) {
            if (overlaps(player_box, predict_obstacle_box(obstacles[i], t))) {
                if (first_id) *first_id = obstacles[i].id;
                if (first_time) *first_time = t;
                return false;
            }
        }
    }
    return true;
}
}  // namespace

Decision choose_action(const PlayerState& player, const Obstacle* obstacles,
                       std::size_t count, float horizon, float step) {
    const Decision invalid{Action::NoSafeAction, 0,
                           std::numeric_limits<float>::infinity(), false};
    if (!valid_box(player.hitbox) || !std::isfinite(player.horizontal_speed) ||
        !std::isfinite(player.vertical_velocity) || !std::isfinite(player.gravity) ||
        !std::isfinite(player.jump_velocity) || !std::isfinite(horizon) ||
        !std::isfinite(step) || horizon <= 0.0f || step <= 0.0f ||
        (count > 0 && obstacles == nullptr)) {
        return invalid;
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (!valid_box(obstacles[i].hitbox) ||
            !std::isfinite(obstacles[i].velocity_x) ||
            !std::isfinite(obstacles[i].velocity_y)) return invalid;
    }
    if (count == 0) return {Action::None, 0, horizon, true};

    uint32_t normal_id = 0;
    float normal_time = std::numeric_limits<float>::infinity();
    if (path_is_clear(player, obstacles, count, horizon, step, false,
                      &normal_id, &normal_time)) {
        return {Action::None, 0, horizon, true};
    }

    if (player.grounded) {
        uint32_t jump_id = 0;
        float jump_time = std::numeric_limits<float>::infinity();
        if (path_is_clear(player, obstacles, count, horizon, step, true,
                          &jump_id, &jump_time)) {
            return {Action::Jump, normal_id, normal_time, true};
        }
    }
    return {Action::NoSafeAction, normal_id, normal_time, true};
}

}  // namespace judsz::autoplay
