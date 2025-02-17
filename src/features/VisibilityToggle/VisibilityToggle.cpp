#include "VisibilityToggle.hpp"

void VisibilityToggle::onToggle(CCObject*) {
    if (m_pSetter)
        m_pSetter(!this->isToggled(), this);
}

void VisibilityToggle::updateState() {
    if (m_pGetter)
        this->toggle(m_pGetter());
}

VisibilityToggle* VisibilityToggle::invokeSetter() {
    if (m_pSetter)
        m_pSetter(m_pGetter(), this);

    return this;
}

VisibilityToggle* VisibilityToggle::create(
    const char* sprite,
    VisibilityToggle::Getter getter,
    VisibilityToggle::Setter setter
) {
    auto ret = new (std::nothrow) VisibilityToggle;

    if (ret && ret->init(
        createEditBtnSprite(sprite, false),
        createEditBtnSprite(sprite, true),
        ret,
        menu_selector(VisibilityToggle::onToggle)
    )) {
        ret->m_pSetter = setter;
        ret->m_pGetter = getter;
        ret->updateState();
        ret->retain();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

VisibilityToggle* VisibilityToggle::create(
    const char* sprite
) {
    auto ret = new VisibilityToggle;

    if (ret && ret->init(
        createEditBtnSprite(sprite, false),
        createEditBtnSprite(sprite, true),
        ret,
        nullptr
    )) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

VisibilityToggle::~VisibilityToggle() {
    this->release();
    CCMenuItemToggler::~CCMenuItemToggler();
}