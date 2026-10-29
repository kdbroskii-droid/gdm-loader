#include "autoplay_planner.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace judsz::autoplay;

int main() {
    const PlayerState player{{0.0f, 0.0f, 10.0f, 10.0f},
                             10.0f, 0.0f, 20.0f, -10.0f, true};

    const Decision empty = choose_action(player, nullptr, 0, 1.0f, 0.05f);
    assert(empty.prediction_valid);
    assert(empty.action == Action::None);

    const Obstacle obstacle{{20.0f, 0.0f, 10.0f, 10.0f}, 42, 0.0f, 0.0f};
    const Decision jump = choose_action(player, &obstacle, 1, 2.0f, 0.05f);
    assert(jump.prediction_valid);
    assert(jump.action == Action::Jump);
    assert(jump.obstacle_id == 42);

    const PlayerState airborne{{0.0f, 0.0f, 10.0f, 10.0f},
                               10.0f, 0.0f, 20.0f, -10.0f, false};
    const Decision cannot_jump = choose_action(airborne, &obstacle, 1, 2.0f, 0.05f);
    assert(cannot_jump.prediction_valid);
    assert(cannot_jump.action == Action::NoSafeAction);

    const Obstacle invalid{{20.0f, 0.0f, -1.0f, 10.0f}, 7, 0.0f, 0.0f};
    const Decision rejected = choose_action(player, &invalid, 1, 2.0f, 0.05f);
    assert(!rejected.prediction_valid);
    assert(rejected.action == Action::NoSafeAction);

    const Decision bad_parameters = choose_action(player, &obstacle, 1, 2.0f, 0.0f);
    assert(!bad_parameters.prediction_valid);
    assert(bad_parameters.action == Action::NoSafeAction);

    std::cout << "Autoplay planner tests passed.\n";
    return 0;
}
