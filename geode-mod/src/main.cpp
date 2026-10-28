#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

// This first native milestone deliberately uses Geode's normal mod lifecycle.
// It does not patch save data or download/execute arbitrary code.
class $modify(GDMMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("GD M: bottom-menu node was not found; button skipped.");
            return true;
        }

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (!sprite) {
            log::warn("GD M: could not create button sprite; button skipped.");
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(GDMMenuLayer::onOpenGDM)
        );
        if (!button) {
            log::warn("GD M: could not create menu button.");
            return true;
        }

        button->setID("gdm-loader-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        log::info("GD M Geode integration initialized.");
        return true;
    }

    void onOpenGDM(CCObject*) {
        FLAlertLayer::create(
            "GD M",
            "Android Geode integration is active.\n\n"
            "Geode remains responsible for loading native .geode packages. "
            "The Judsz bridge and in-game catalog are not connected yet.\n\n"
            "Do not treat demo Judsz commands as live game actions.",
            "OK"
        )->show();
    }
};
