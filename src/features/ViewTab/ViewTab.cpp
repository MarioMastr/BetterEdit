#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/DrawGridLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/utils/cocos.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

#include "../VisibilityToggle/VisibilityToggle.hpp"
#include "../RotateSaws/RotateSaws.hpp"

using namespace geode::prelude;
using namespace keybinds;

constexpr const int VISIBILITYTABUI_PRIO = 3456;

class $modify(GameObjectExtra, GameObject) {
    bool m_objectHidden = false;
    bool m_actualVisible = true;
    bool m_callFromExtra = false;

    void updatePresence() {
        if (m_highDetail) {
            m_fields->m_objectHidden = Mod::get()->getSettingValue<bool>("hide-ldm");
        }
        
        // update visibility
        m_fields->m_callFromExtra = true;
        setVisible(m_bVisible);
        m_fields->m_callFromExtra = false;
    }

    void setVisible(bool visible) {
        if (!m_fields->m_callFromExtra) {
            m_fields->m_actualVisible = visible;
        }

        if (m_fields->m_objectHidden) {
            visible = false;
        }

        else visible = m_fields->m_actualVisible;

        GameObject::setVisible(visible);
    }
};

static void updateVisibilityTab() {
    auto bbar = static_cast<EditButtonBar*>(EditorUI::get()->getChildByID("view-tab"_spr));

    if (!bbar) return;

    bbar->reloadItems(
        GameManager::sharedState()->getIntGameVariable("0049"),
        GameManager::sharedState()->getIntGameVariable("0050")
    );

    CCObject* btn;
    CCARRAY_FOREACH(bbar->m_buttonArray, btn) {
        typeinfo_cast<VisibilityToggle*>(btn)->updateState();
    }
}

struct $modify(VisibilityTabUI, EditorUI) {
    CCNode* viewModeBtn;

    void updateModeSprite(CCNode* node, int tag, const char* spr) {
        // resetUI resets the button sprites so we may have to change them back 
        // to squarish ones again
        if (node) {
            auto bg = m_selectedMode == tag ? "GJ_button_02.png" : "GJ_button_01.png";
            auto btn = static_cast<CCMenuItemSpriteExtra*>(node);
            if (auto bspr = typeinfo_cast<ButtonSprite*>(btn->getNormalImage())) {
                // remove GD's texture
                bspr->setTexture(nullptr);
                bspr->setTextureRect({ 0, 0, 0, 0 });
                bspr->updateBGImage(bg);
            }
            else {
                btn->setNormalImage(ButtonSprite::create(
                    CCSprite::createWithSpriteFrameName(spr),
                    0x32, true, 0x32, bg, .6f
                ));
            }
        }
    }

    void updateModeSprites() {
        this->updateModeSprite(m_buildModeBtn, 2, "tab-create.png"_spr);
        this->updateModeSprite(m_deleteModeBtn, 1, "tab-delete.png"_spr);
        this->updateModeSprite(m_editModeBtn, 3, "tab-edit.png"_spr);
        this->updateModeSprite(m_fields->viewModeBtn, 4, "tab-view.png"_spr);
    }

    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel))
            return false;
        
        auto winSize = CCDirector::get()->getWinSize();
        
        // make a bit space for new style menu since the old one is a tiny bit cramped
        if (auto side = getChildOfType<CCSprite>(this, 1)) {
            side->setPositionX(96.f);
        }
        for (auto& child : CCArrayExt<CCNode>(m_pChildren)) {
            if (auto bar = typeinfo_cast<EditButtonBar*>(child)) {
                bar->setPositionX(bar->getPositionX() + 5.f);
            }
        }

        if (auto menu = this->getChildByID("toolbar-categories-menu")) {
            m_fields->viewModeBtn = CCMenuItemSpriteExtra::create(
                CCNode::create(), this, menu_selector(EditorUI::toggleMode)
            );
            m_fields->viewModeBtn->setID("view-button"_spr);
            m_fields->viewModeBtn->setTag(4);
            menu->addChild(m_fields->viewModeBtn);

            this->updateModeSprites();

            menu->setContentSize({ 90.f, 90.f });
            menu->setPositionX(47.f);
            menu->setLayout(RowLayout::create()
                ->setCrossAxisOverflow(false)
                ->setGrowCrossAxis(true)
            );
        }

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) {
                this->toggleMode(m_fields->viewModeBtn);
            }
            return ListenerResult::Propagate;
        }, "view-mode"_spr);

        /*
            Create buttons
        */
        auto btns = CCArray::create();

        #define GET_SETTING(set) Mod::get()->getSettingValue<bool>(set)
        #define SET_SETTING(set, val) Mod::get()->setSettingValue<bool>(set, val)
        #define ADD_VIEW_TAB_BTN(sprite, _getter, _setter) \
            btns->addObject(VisibilityToggle::create(GEODE_CONCAT(sprite, _spr), []() -> bool _getter, [&](bool b, auto p) -> void _setter))

        ADD_VIEW_TAB_BTN("v_rotate.png",
            {
                return shouldRotateSaw();
            }, {
                onRotateSaws(p);
            }
        );
        ADD_VIEW_TAB_BTN("v_ldm.png",
            {
                return GET_SETTING("hide-ldm");
            }, {
                static_cast<VisibilityTabUI*>(EditorUI::get())->toggleLDM(b);
            }
        );
        ADD_VIEW_TAB_BTN("v_pulse.png",
            {
                return GET_SETTING("editor-pulse");
            }, {
                SET_SETTING("editor-pulse", b);
            }
        );
        ADD_VIEW_TAB_BTN("v_prevmode.png",
            {
                return GameManager::sharedState()->getGameVariable("0036");
            }, {
                GameManager::sharedState()->setGameVariable("0036", b);
                LevelEditorLayer::get()->resetMovingObjects();
                LevelEditorLayer::get()->m_editorUI->onStopPlaytest(p);
                LevelEditorLayer::get()->updateEditorMode();
            }
        );
        ADD_VIEW_TAB_BTN("v_bpm_line.png",
            {
                return GameManager::sharedState()->m_showSongMarkers;
            }, {
                GameManager::sharedState()->m_showSongMarkers = b;
            }
        );
        ADD_VIEW_TAB_BTN("v_pos_line.png",
            {
                return GET_SETTING("pos-line");
            }, {
                SET_SETTING("pos-line", b);
            }
        );
        ADD_VIEW_TAB_BTN("v_dur_line.png",
            {
                return GameManager::sharedState()->getGameVariable("0058");
            }, {
                GameManager::sharedState()->setGameVariable("0058", b);
                LevelEditorLayer::get()->updateOptions();
            }
        );
        ADD_VIEW_TAB_BTN("v_eff_line.png",
            {
                return GameManager::sharedState()->getGameVariable("0043");
            }, {
                GameManager::sharedState()->setGameVariable("0043", b);
                LevelEditorLayer::get()->updateOptions();
            }
        );
        ADD_VIEW_TAB_BTN("v_grnd.png",
            {
                return GameManager::sharedState()->getGameVariable("0037");
            }, {
                GameManager::sharedState()->setGameVariable("0037", b);
                LevelEditorLayer::get()->updateOptions();
                LevelEditorLayer::get()->m_groundLayer->setVisible(b);
            }
        );
        ADD_VIEW_TAB_BTN("v_grid.png",
            {
                return GameManager::sharedState()->getGameVariable("0038");
            }, {
                GameManager::sharedState()->setGameVariable("0038", b);
                LevelEditorLayer::get()->updateOptions();
            }
        );
        ADD_VIEW_TAB_BTN("v_portal_borders.png",
            {
                return GET_SETTING("portal-lines");
            }, {
                SET_SETTING("portal-lines", b);
            }
        );
        ADD_VIEW_TAB_BTN("v_highlight.png",
            {
                return GET_SETTING("highlight-triggers");
            }, {
                SET_SETTING("highlight-triggers", b);
            }
        );
        ADD_VIEW_TAB_BTN("v_dash.png",
            {
                return GET_SETTING("dash-lines");
            }, {
                SET_SETTING("dash-lines", b);
            }
        );

        auto buttonBar = EditButtonBar::create(
            btns,
            { CCDirector::sharedDirector()->getWinSize().width / 2, 86.0f },
            m_tabsArray->count(), false,
            GameManager::sharedState()->getIntGameVariable("0049"),
            GameManager::sharedState()->getIntGameVariable("0050")
        );
        buttonBar->setID("view-tab"_spr);
        // Need to set a tag too so GD doesn't accidentally grab this tab 
        // when doing getChildByTag for its tabs
        buttonBar->setTag(-1);
        buttonBar->setVisible(m_selectedMode == 4);

        this->addChild(buttonBar, 10);

        // other
        this->updateLDM();

        return true;
    }

    void showUI(bool show) {
        EditorUI::showUI(show);
        m_fields->viewModeBtn->setVisible(show);
        this->showVisibilityTab(show);
    }

    void toggleMode(CCObject* sender) {
        EditorUI::toggleMode(sender);
        this->resetUI();
        this->updateModeSprites();

        if (auto viewBtnBar = getChildByID("view-tab"_spr)) {
            viewBtnBar->setVisible(m_selectedMode == 4);
        }
    }

    void showVisibilityTab(bool show) {
        typeinfo_cast<EditButtonBar*>(getChildByID("view-tab"_spr))
            ->setVisible(show && m_selectedMode == 4);

        m_buildModeBtn->getParent()->getChildByTag(4)
            ->setVisible(show);
    }

    void toggleLDM(bool toggled) {
        Mod::get()->setSettingValue<bool>("hide-ldm", toggled);
        this->updateLDM();
    }

    void updateLDM() {
        // update every object in the level (oh god)
        for (auto& obj : CCArrayExt<GameObjectExtra*>(m_editorLayer->getAllObjects())) {
            obj->updatePresence();
        }
    }

    void selectObject(GameObject* obj, bool filter) {
        if (!static_cast<GameObjectExtra*>(obj)->m_fields->m_objectHidden) {
            EditorUI::selectObject(obj, filter);
        }
    }

    void selectObjects(CCArray* objs, bool ignoreFilters) {
        // filter out LDM objects
        if (objs) {
            size_t objCount = objs->count();
            for (size_t i = 0; i < objCount; i++) {
                auto obj = static_cast<GameObjectExtra*>(objs->objectAtIndex(i));
                if (obj->m_fields->m_objectHidden) {
                    objs->removeObjectAtIndex(i, false);
                }
                i -= 1;
                objCount -= 1;
            }
        }
        EditorUI::selectObjects(objs, ignoreFilters);
    }

    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorUI::selectObject", VISIBILITYTABUI_PRIO);
        (void)self.setHookPriority("EditorUI::selectObjects", VISIBILITYTABUI_PRIO);
    }
};

class $modify(DrawGridLayer) {
    void draw() {
        bool origBool = m_editor->m_previewMode;
        m_editor->m_previewMode = Mod::get()->getSettingValue<bool>("pos-line");
        
        DrawGridLayer::draw();

        m_editor->m_previewMode = origBool;
    }
};

class $modify(EditorPauseLayer) {
    void onResume(CCObject* pSender) {
        EditorPauseLayer::onResume(pSender);

        updateVisibilityTab();
    }
};

$execute {
    BindManager::get()->registerBindable(BindableAction(
        "view-mode"_spr,
        "View Mode",
        "Toggle the View Tab",
        { Keybind::create(KEY_Four) },
        Category::EDITOR_UI
    ), "robtop.geometry-dash/delete-mode");
}
