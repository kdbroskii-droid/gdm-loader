# Judsz hitbox-based autoplay planner

The planner is a deterministic native component that predicts player/obstacle AABB overlap over a short horizon and selects a jump only when the sampled jump trajectory is clear.

## Input contract

A **verified Geometry Dash 2.2081 binding** must supply, every game update:
- the player's current collision hitbox in a consistent world coordinate system;
- grounded state, vertical velocity, gravity and jump impulse;
- obstacle collision hitboxes for the currently loaded level, including the current user-created level;
- relative horizontal motion (obstacle motion relative to the player, or equivalent coordinates sampled across time).

The `choose_action()` function is deliberately pure: it consumes supplied hitboxes and never guesses native addresses or reads arbitrary process memory. It samples both the current path and jump path; if neither is clear, it returns `NoSafeAction`.

## Current integration status

This planner is compiled into the Judsz native library, but it is **not yet connected to Geometry Dash**. Judsz still has no verified 2.2081 binding that enumerates live level objects or applies a jump input. Therefore this code cannot yet autoplay a real Geometry Dash level. The native runtime must stay fail-closed until the binding is reverse-engineered and validated against the exact APK/ABI.

## Validation needed before enabling live play

1. Verify player and obstacle hitbox coordinates against visible collision behavior.
2. Test flat ground, single spikes, stacked obstacles, gaps, moving hazards and custom/current user levels.
3. Compare predicted collisions against observed deaths over deterministic replay frames.
4. Enable the gameplay command only after the binding and planner pass those tests.
