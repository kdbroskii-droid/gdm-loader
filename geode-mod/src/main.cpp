#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

// In-game HUD entry point. This is a Geode mod, so all game reads occur
// through the current PlayLayer and its live object list, not APK scraping.
class $modify(GDMMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("GD M: bottom-menu node was not found; button skipped.");
            return true;
        }

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (!sprite) return true;

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(GDMMenuLayer::onOpenGDM)
        );
        if (!button) return true;

        button->setID("gdm-loader-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onOpenGDM(CCObject*) {
        FLAlertLayer::create(
            "GD M AutoPlay",
            "AutoPlay uses live collision hitboxes from the currently loaded level. "
            "It does not use screenshots, colors, textures, or level names. "
            "This early build supports conservative ground-jump timing in normal "
            "auto-scroll mode; it is not yet validated for every game mode.",
            "OK"
        )->show();
    }
};

class $modify(GDMAutoPlayLayer, PlayLayer) {
    struct Fields {
        bool enabled = true;
        bool jumpHeld = false;
        float jumpReleaseTimer = 0.f;
        float triggerCooldown = 0.f;
        int lastObstacleID = -1;
    };

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_fields->enabled) return;

        auto* player = this->m_player1;
        if (!player || this->m_isPaused || this->m_hasCompletedLevel) return;

        m_fields->triggerCooldown = std::max(0.f, m_fields->triggerCooldown - dt);
        m_fields->jumpReleaseTimer = std::max(0.f, m_fields->jumpReleaseTimer - dt);

        // Release the input after one short pulse; never leave jump held.
        if (m_fields->jumpHeld && m_fields->jumpReleaseTimer <= 0.f) {
            player->releaseButton(PlayerButton::Jump);
            m_fields->jumpHeld = false;
        }

        // Only read native collision rectangles. No framebuffer, screenshot,
        // sprite color, texture, or OCR input is used by this decision.
        const auto playerBox = player->getObjectRect(1.f, true);
        if (playerBox.size.width <= 0.f || playerBox.size.height <= 0.f) return;

        // Conservative early version: predict the next grounded collision
        // from the real obstacle hitboxes ahead of the player. GameObject's
        // collision rect is the sole perception source.
        float nearestDistance = 100000.f;
        GameObject* nearest = nullptr;
        for (auto* object : CCArrayExt<GameObject*>(m_objects)) {
            if (!object || object == player) continue;
            const auto box = object->getObjectRect(1.f, true);
            if (box.size.width <= 0.f || box.size.height <= 0.f) continue;

            const float distance = box.getMinX() - playerBox.getMaxX();
            if (distance < 2.f || distance > 115.f) continue;

            // Require the obstacle hitbox to intersect the player's current
            // vertical band or sit just above the ground band. This excludes
            // most background/decorative rectangles without looking at art.
            const bool verticalRelevant =
                box.getMinY() <= playerBox.getMaxY() + 12.f &&
                box.getMaxY() >= playerBox.getMinY() - 8.f;
            if (!verticalRelevant) continue;

            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearest = object;
            }
        }

        if (!nearest || m_fields->triggerCooldown > 0.f) return;

        // Only perform a normal jump when grounded. The distance threshold
        // scales with the player's current horizontal speed.
        if (player->m_isOnGround2) {
            const float speed = static_cast<float>(
                std::max(1.0, std::abs(player->getCurrentXVelocity()))
            );
            const float timeToContact = nearestDistance / speed;
            if (timeToContact <= 0.28f) {
                if (player->pushButton(PlayerButton::Jump)) {
                    m_fields->jumpHeld = true;
                    m_fields->jumpReleaseTimer = 0.045f;
                    m_fields->triggerCooldown = 0.16f;
                    m_fields->lastObstacleID = nearest->m_objectID;
                }
            }
        }
    }
};
