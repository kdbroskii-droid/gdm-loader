#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

// Runtime state shared by the GD M menu and the PlayLayer hook.
// Auto Play starts OFF; the user must enable it from the in-game menu.
static bool g_autoPlayEnabled = false;

class GDMOverlay final : public CCLayerColor {
public:
    static GDMOverlay* create() {
        auto* ret = new GDMOverlay();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() override {
        if (!CCLayerColor::initWithColor(ccc4(0, 7, 20, 190))) return false;

        const auto size = CCDirector::sharedDirector()->getWinSize();
        const float panelWidth = std::min(size.width * 0.78f, 440.f);
        const float panelHeight = std::min(size.height * 0.76f, 390.f);

        auto* panel = CCLayerColor::create(
            ccc4(3, 17, 42, 250), panelWidth, panelHeight
        );
        panel->setPosition(
            (size.width - panelWidth) / 2.f,
            (size.height - panelHeight) / 2.f
        );
        this->addChild(panel, 1);

        auto* title = CCLabelBMFont::create("GD M  |  MOD MENU", "bigFont.fnt");
        if (title) {
            title->setScale(0.62f);
            title->setPosition(panelWidth / 2.f, panelHeight - 32.f);
            panel->addChild(title);
        }

        auto* subtitle = CCLabelBMFont::create(
            "Blue interface - Geode-powered prototype", "goldFont.fnt"
        );
        if (subtitle) {
            subtitle->setScale(0.34f);
            subtitle->setPosition(panelWidth / 2.f, panelHeight - 58.f);
            panel->addChild(subtitle);
        }

        auto* autoLabel = CCLabelBMFont::create(
            g_autoPlayEnabled ? "Auto Play: ON" : "Auto Play: OFF",
            "bigFont.fnt"
        );
        auto* resourcesLabel = CCLabelBMFont::create(
            "Unlimited Resources: not implemented", "bigFont.fnt"
        );
        auto* unlocksLabel = CCLabelBMFont::create(
            "Complete Everything: not implemented", "bigFont.fnt"
        );
        auto* closeLabel = CCLabelBMFont::create("CLOSE", "bigFont.fnt");

        auto* menu = CCMenu::create();
        if (!autoLabel || !resourcesLabel || !unlocksLabel || !closeLabel || !menu) {
            return false;
        }

        autoLabel->setScale(0.48f);
        resourcesLabel->setScale(0.34f);
        unlocksLabel->setScale(0.34f);
        closeLabel->setScale(0.5f);

        auto* autoItem = CCMenuItemLabel::create(
            autoLabel, this, menu_selector(GDMOverlay::onToggleAutoPlay)
        );
        auto* resourcesItem = CCMenuItemLabel::create(
            resourcesLabel, this, menu_selector(GDMOverlay::onNotImplemented)
        );
        auto* unlocksItem = CCMenuItemLabel::create(
            unlocksLabel, this, menu_selector(GDMOverlay::onNotImplemented)
        );
        auto* closeItem = CCMenuItemLabel::create(
            closeLabel, this, menu_selector(GDMOverlay::onClose)
        );
        if (!autoItem || !resourcesItem || !unlocksItem || !closeItem) return false;

        menu->addChild(autoItem);
        menu->addChild(resourcesItem);
        menu->addChild(unlocksItem);
        menu->addChild(closeItem);
        menu->alignItemsVerticallyWithPadding(27.f);
        menu->setPosition(panelWidth / 2.f, panelHeight / 2.f - 10.f);
        panel->addChild(menu);
        return true;
    }

    void onToggleAutoPlay(CCObject*) {
        g_autoPlayEnabled = !g_autoPlayEnabled;
        FLAlertLayer::create(
            "GD M",
            g_autoPlayEnabled
                ? "Auto Play enabled. This is an early prototype and is not verified for every level or game mode."
                : "Auto Play disabled.",
            "OK"
        )->show();
        this->removeFromParentAndCleanup(true);
    }

    void onNotImplemented(CCObject*) {
        FLAlertLayer::create(
            "GD M",
            "This feature is shown in the interface but is not implemented yet. It will not change resources or unlocks.",
            "OK"
        )->show();
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};

// Add GD M's own entry point to the Geometry Dash main menu.
class $modify(GDMMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto* menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("GD M: bottom-menu node was not found; button skipped.");
            return true;
        }

        auto* mark = CCLabelBMFont::create("M", "bigFont.fnt");
        if (!mark) return true;
        mark->setScale(0.8f);

        auto* button = CCMenuItemLabel::create(
            mark, this, menu_selector(GDMMenuLayer::onOpenGDM)
        );
        if (!button) return true;

        button->setID("gdm-loader-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onOpenGDM(CCObject*) {
        auto* scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        if (scene->getChildByID("gdm-overlay"_spr)) return;

        auto* overlay = GDMOverlay::create();
        if (!overlay) {
            FLAlertLayer::create("GD M", "Could not create the GD M menu.", "OK")->show();
            return;
        }
        overlay->setID("gdm-overlay"_spr);
        scene->addChild(overlay, 9999);
    }
};

// Early Auto Play prototype. It evaluates native hitboxes in the currently loaded level;
// it does not use screenshots, textures, colors, OCR, or level names. Its behaviour
// is not yet validated for every mode, so it remains opt-in.
class $modify(GDMAutoPlayLayer, PlayLayer) {
    struct Fields {
        bool jumpHeld = false;
        float jumpReleaseTimer = 0.f;
        float triggerCooldown = 0.f;
        int lastObstacleID = -1;
    };

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!g_autoPlayEnabled) return;

        auto* player = this->m_player1;
        if (!player || this->m_isPaused || this->m_hasCompletedLevel) return;

        m_fields->triggerCooldown = std::max(0.f, m_fields->triggerCooldown - dt);
        m_fields->jumpReleaseTimer = std::max(0.f, m_fields->jumpReleaseTimer - dt);

        if (m_fields->jumpHeld && m_fields->jumpReleaseTimer <= 0.f) {
            player->releaseButton(PlayerButton::Jump);
            m_fields->jumpHeld = false;
        }

        const auto playerBox = player->getObjectRect(1.f, true);
        if (playerBox.size.width <= 0.f || playerBox.size.height <= 0.f) return;

        float nearestDistance = 100000.f;
        GameObject* nearest = nullptr;
        for (auto* object : CCArrayExt<GameObject*>(m_objects)) {
            if (!object || object == player) continue;
            const auto box = object->getObjectRect(1.f, true);
            if (box.size.width <= 0.f || box.size.height <= 0.f) continue;

            const float distance = box.getMinX() - playerBox.getMaxX();
            if (distance < 2.f || distance > 115.f) continue;

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

        if (player->m_isOnGround2) {
            const float speed = static_cast<float>(
                std::max(1.0, std::abs(player->getCurrentXVelocity()))
            );
            const float timeToContact = nearestDistance / speed;
            if (timeToContact <= 0.28f && player->pushButton(PlayerButton::Jump)) {
                m_fields->jumpHeld = true;
                m_fields->jumpReleaseTimer = 0.045f;
                m_fields->triggerCooldown = 0.16f;
                m_fields->lastObstacleID = nearest->m_objectID;
            }
        }
    }
};
