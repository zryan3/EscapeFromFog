#include "Player.hpp"

#include <algorithm>

#include "engine/SdlManager.hpp"
#include "engine/Camera.hpp"

#include "LevelGenerator.hpp"

const float Player::scMovementScalar = 20.0f;
const float Player::scMouseSensitivity = 0.65f;
const glm::vec2 Player::scPlayerSize = glm::vec2(0.2f);

/**
 * @brief Player::Player
 * @param camera
 * @param level
 */
Player::Player(Camera& camera, LevelGenerator& level)
: mFirstPersonCamera(camera)
, mLevel(level)
, mStartPosition(camera.getPosition())
, mMovementDir(glm::vec3(0))
, mMouseLocked(false)
{

}

/**
 * @brief Player::getPosition
 * @return
 */
glm::vec3 Player::getPosition() const
{
    return mFirstPersonCamera.getPosition();
}

/**
 * @brief Player::setPosition
 * @param position
 */
void Player::setPosition(const glm::vec3& position)
{
    mFirstPersonCamera.setPosition(position);
}

/**
 * @brief Player::move
 * @param vel
 * @param dt
 */
void Player::move(const glm::vec3& vel, float dt)
{
    mFirstPersonCamera.move(vel, dt);
}

/**
 * @brief Player::input
 * @param sdlManager
 * @param mouseWheelDelta
 */
void Player::input(const SdlManager& sdlManager, const float mouseWheelDelta)
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
    int coordX;
    int coordY;
    const Uint32 currentMouseStates = SDL_GetMouseState(&coordX, &coordY);
    glm::vec2 coords = glm::vec2(coordX, coordY);

    glm::vec2 winCenter = glm::vec2(
        static_cast<float>(sdlManager.getDimensions().x) * 0.5f,
        static_cast<float>(sdlManager.getDimensions().y) * 0.5f);

    SDL_PumpEvents();

    // handle input events
    if (mMouseLocked && currentKeyStates[SDL_SCANCODE_TAB])
    {
        SDL_ShowCursor(SDL_ENABLE);
        mMouseLocked = false;

        if (APP_DEBUG)
            SDL_Log("MOUSE UNLOCKED\n");
    }
    else if (!mMouseLocked && (currentMouseStates & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {
        SDL_ShowCursor(SDL_DISABLE);
        mMouseLocked = true;

        if (APP_DEBUG)
            SDL_Log("MOUSE LOCKED\n");
    }

    // reset movement direction every iteration
    mMovementDir = glm::vec3(0);

    // keyboard movements
    if (currentKeyStates[SDL_SCANCODE_W])
        mMovementDir += mFirstPersonCamera.getTarget();
    if (currentKeyStates[SDL_SCANCODE_S])
        mMovementDir -= mFirstPersonCamera.getTarget();
    if (currentKeyStates[SDL_SCANCODE_A])
        mMovementDir -= mFirstPersonCamera.getRight();
    if (currentKeyStates[SDL_SCANCODE_D])
        mMovementDir += mFirstPersonCamera.getRight();

    // mouse wheel events
    if (mouseWheelDelta != 0)
        mFirstPersonCamera.updateFieldOfView(mouseWheelDelta);

    // rotations (mouse movements)
    if (mMouseLocked)
    {
        float xOffset = coords.x - winCenter.x;
        float yOffset = winCenter.y - coords.y;

//        SDL_Log("winCenter.x = %f, winCenter.y = %f", winCenter.x, winCenter.y);
//        SDL_Log("coords.x = %f, coords.y = %f", coords.x, coords.y);
//        SDL_Log("xOffset = %f, yOffset = %f", xOffset, yOffset);

        if (xOffset || yOffset)
        {
            mFirstPersonCamera.rotate(xOffset * scMouseSensitivity, yOffset * scMouseSensitivity, false, false);
            SDL_WarpMouseInWindow(sdlManager.getSdlWindow(), winCenter.x, winCenter.y);
        }
    }
}

/**
 * @brief Player::update
 * @param dt
 * @param timeSinceInit
 */
void Player::update(const float dt, const double timeSinceInit)
{
    if (glm::length(mMovementDir) > 0)
    {
        mMovementDir = glm::normalize(mMovementDir);

        glm::vec3 origin (getPosition());
        // R(t) = P + Vt
        glm::vec3 direction (origin + (mMovementDir * scMovementScalar * dt));
        glm::vec3 collision (iterateThruSpace(mLevel.getEmptySpace(), mLevel.getTileScalar(), origin, direction));
//        mMovementDir *= collision;
//        // SDL_Log("collision (%f, %f, %f)\n", collision.x, collision.y, collision.z);
//        mMovementDir.y = 0.0f;

        mFirstPersonCamera.move(mMovementDir, scMovementScalar * dt);

        if (isOnExitPoint(getPosition()))
        {

        }
    }
    // else no movement
}

/**
 * In first person, the player's hands are rendered.
 * In third person, the entire player is rendered.
 * @brief Player::render
 */
void Player::render() const
{

}

/**
 * @brief Player::getCamera
 * @return
 */
Camera& Player::getCamera() const
{
    return mFirstPersonCamera;
}

/**
 * @brief Player::iterateThruSpace
 * @param emptySpaces
 * @param spaceScalar
 * @param origin
 * @param dir
 * @return
 */
glm::vec3 Player::iterateThruSpace(const std::vector<glm::vec3>& emptySpaces,
    const glm::vec3& spaceScalar,
    const glm::vec3& origin,
    const glm::vec3& dir) const
{
    glm::vec3 collisionVec (1);
    for (auto& emptiness : emptySpaces)
    {
        collisionVec *= rectangularCollision(origin, dir,
            glm::vec3(scPlayerSize.x, 0, scPlayerSize.y), emptiness, spaceScalar);
    }

    return collisionVec;
}

/**
 * Returns zero vector if no movement whatsoever,
 * else it returns 1 along the axis of movement.
 * @brief Player::rectangularCollision
 * @param origin
 * @param dir
 * @param objSize
 * @param rectangle
 * @param scalar
 * @return
 */
glm::vec3 Player::rectangularCollision(const glm::vec3& origin,
    const glm::vec3& dir, const glm::vec3& objSize,
    const glm::vec3& rectangle,
    const glm::vec3& scalar) const
{
    glm::vec3 result (0.0f, 1.0f, 0.0f);

    if (dir.x + objSize.x < rectangle.x * scalar.x ||
       dir.x - objSize.x > (rectangle.x + 1.0f) * scalar.x  ||
       origin.z + objSize.z < rectangle.z * scalar.z ||
       origin.z - objSize.z > (rectangle.z + 1.0f) * scalar.z)
    {
        result.x = 1.0f;
    }

    if (origin.x + objSize.x < rectangle.x * scalar.x  ||
       origin.x - objSize.x > (rectangle.x + 1.0f) * scalar.x  ||
       dir.z + objSize.z < rectangle.z * scalar.z ||
       dir.z - objSize.z > (rectangle.z + 1.0f) * scalar.z)
    {
        result.z = 1.0f;
    }

    return result;
}

/**
 * Check if the length of the distance from the player
 * to the point is equal to half the size of the sprite.
 * @brief Player::isOnExitPoint
 * @param origin
 * @return
 */
bool Player::isOnExitPoint(const glm::vec3& origin) const
{
    const auto& exitPoints = mLevel.getExitPoints();

    auto exited = std::find_if(exitPoints.begin(), exitPoints.end(),
        [&] (const glm::vec3& point)->bool {
            return glm::length(point - origin) < mLevel.getSpriteHalfWidth();
    });

    return (exited != exitPoints.end());
}

/**
 * @brief Player::isOnSpeedPowerUp
 * @param origin
 * @return
 */
bool Player::isOnSpeedPowerUp(const glm::vec3& origin) const
{
    return false;
}

/**
 * @brief Player::isOnRechargePowerUp
 * @param origin
 * @return
 */
bool Player::isOnRechargePowerUp(const glm::vec3& origin) const
{
    return false;
}

/**
 * @brief Player::isOnInvinciblePowerUp
 * @param origin
 * @return
 */
bool Player::isOnInvinciblePowerUp(const glm::vec3& origin) const
{
    return false;
}
