#include "dobby.h"
#include <dlfcn.h>
#include "cocos2dcpp.h"

void* get_sym_addr(const char* sym_name)
{
    auto handle = dlopen("libgame.so", RTLD_NOW);
    return dlsym(handle, sym_name);
}

class GameManager
{
public:
    static void* sharedState();
};


void (*GameObject_addGlow)(void*);
void GameObject_addGlowH(GameObject* self)
{
    void* gm = GameManager::sharedState();
    void* gm_vft = *reinterpret_cast<void**>(gm);
    bool res = reinterpret_cast<bool(*)(void*)>(*reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(gm_vft) + 424))(gm);

    if (res)
        return;
    
    int block_id = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(self) + 0x31c);

    if (block_id != 83)
        return GameObject_addGlow(self);

    *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(self) + 0x27d) = true;

    cocos2d::CCSprite* glow = cocos2d::CCSprite::createWithSpriteFrameName("square_08_glow_001.png");

    *reinterpret_cast<cocos2d::CCSprite**>(reinterpret_cast<uintptr_t>(self) + 0x26c) = glow;
    
    glow->retain();

    glow->setPosition(CCPoint(0,0));

    self->setOpacity(255);
};

[[gnu::constructor]]
void constructor()
{
    DobbyHook(get_sym_addr("_ZN10GameObject7addGlowEv"), (void*)GameObject_addGlowH, (void**)&GameObject_addGlow);
};
