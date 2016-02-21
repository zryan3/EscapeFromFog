#ifndef KILLDASHNINE_HPP
#define KILLDASHNINE_HPP

#include <glm/glm.hpp>

#include "engine/SdlManager.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/DataLogger.hpp"

#include "IApplication.hpp"

// temp testing
#include "engine/Camera.hpp"
#include "engine/graphics/Entity.hpp"
#include "engine/graphics/Skybox.hpp"
#include "engine/graphics/Light.hpp"
#include "engine/graphics/PostProcessorImpl.hpp"
#include "engine/graphics/Sprite.hpp"
#include "engine/audio/SdlMixer.hpp"

#include "LevelGenerator.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
#include "ImGuiHelper.hpp"

class KillDashNine final : public IApplication
{
public:
    KillDashNine();
    virtual void start() override;

protected:
    virtual void loop() override;
    virtual void handleEvents() override;
    virtual void update(float dt, double timeSinceInit) override;
    virtual void render() override;
    virtual void finish() override;

private:
    static const float sTimePerFrame;
    static const glm::uvec2 sWindowDimens;
    static const std::string sTitle;

    SdlManager mSdlManager;
    ResourceManager mResources;
    DataLogger mLogger;

    bool mAppIsRunning;
    unsigned int mFrameCounter;
    float mTimeSinceLastUpdate;
    float mAccumulator;

    Camera mCamera;
    Entity mCube;
    LevelGenerator mLevelGen;
    ImGuiHelper mImGui;
    Player mPlayer;
    Skybox mSkybox;
    PostProcessorImpl mPostProcessor;
    Light mLight;
    Sprite mTestSprite;
    std::vector<Enemy::Ptr> mEnemies;
    std::vector<Sprite::Ptr> mPowerUps;

    // exits
    // power ups

    SdlMixer mSdlMixer;

private:
    void init();
    void printFramesToConsole(const float dt);
    void sdlEvents(SDL_Event& event, float& mouseWheelDy);
};

#endif // KILLDASHNINE_HPP
