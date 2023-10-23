#include "dobby.h"
#include <dlfcn.h>
#include "cocos2dcpp.h"
#include <array>

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

void (*EditorUI_transformObject)(EditorUI* self, GameObject* obj, int editCommand);
void HEditorUI_transformObject(EditorUI* self, GameObject* obj, int editCommand)
{
    static std::array<int, 2> swapCmd = {9, 10};

    if (editCommand != 9 && editCommand != 10)
    {
        return EditorUI_transformObject(self, obj, editCommand);
    }

    auto goVft = reinterpret_cast<uintptr_t>(*reinterpret_cast<void**>(obj));
    auto getRotation = reinterpret_cast<float (*)(GameObject*)>(*reinterpret_cast<void**>(goVft + 48));

    float rot = fmodf(fabsf(getRotation(obj)), 360.0f);
    bool rotatedToSide = (rot == 90.0f || rot == 270.0f);

    if (!rotatedToSide)
    {
        return EditorUI_transformObject(self, obj, editCommand);
    }
    
    // this is so swag
    editCommand = swapCmd[editCommand != 10 && rotatedToSide];

    return EditorUI_transformObject(self, obj, editCommand);
}

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
    DobbyHook(
        get_sym_addr("_ZN8EditorUI15transformObjectEP10GameObject11EditCommand"), 
        (void*)HEditorUI_transformObject, (void**)&EditorUI_transformObject
    );
    DobbyHook(get_sym_addr("_ZN10GameObject7addGlowEv"), (void*)GameObject_addGlowH, (void**)&GameObject_addGlow);
};
