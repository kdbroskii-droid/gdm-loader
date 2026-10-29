#pragma once

#include <cstddef>
#include <cstdint>

namespace judsz::autoplay {

struct Hitbox {
    float x;
    float y;
    float width;
    float height;
};

struct Obstacle {
    Hitbox hitbox;
    uint32_t id;
    float velocity_x;  // World units per second; must be relative to player coordinates.
    float velocity_y;
};

struct PlayerState {
    Hitbox hitbox;
    float horizontal_speed;
    float vertical_velocity;
    float gravity;
    float jump_velocity;
    bool grounded;
};

enum class Action : uint8_t {
    None = 0,
    Jump = 1,
    NoSafeAction = 2
};

struct Decision {
    Action action;
    uint32_t obstacle_id;
    float seconds_until_contact;
    bool prediction_valid;
};

// Pure planner: it consumes hitboxes supplied by a verified game binding.
// It does not guess memory addresses or read another process by itself.
Decision choose_action(const PlayerState& player,
                       const Obstacle* obstacles,
                       std::size_t obstacle_count,
                       float lookahead_seconds,
                       float sample_step_seconds);

}  // namespace judsz::autoplay
