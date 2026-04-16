#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

#include "MindStateId.hpp"
#include "RobotStateContext.hpp"

/**
 * @brief A simple, efficient state machine manager.
 *
 * This class manages the current state and transitions between states.
 * It is designed to be lightweight and suitable for microcontrollers.
 */
class Mind
{
public:
    Mind() = default;
    ~Mind()
    {
        destroyActiveState();
    }

    Mind(const Mind&) = delete;
    Mind& operator=(const Mind&) = delete;
    Mind(Mind&&) = delete;
    Mind& operator=(Mind&&) = delete;

    /**
     * @brief Transitions the state machine to a new state.
     *
     * It calls exit() on the current state and enter() on the new state.
     *
     * @tparam NewState The type of the new state to transition to.
     * @param context The shared context data for the robot.
     */
    template <typename NewState, typename... TArgs>
    void transitionTo(RobotStateContext& context, TArgs&&... args)
    {
        static_assert(sizeof(NewState) <= kStateStorageSize,
                      "State is larger than Mind state storage.");
        static_assert(alignof(NewState) <= alignof(StateStorage),
                      "State alignment is larger than Mind state storage alignment.");

        exitAndDestroyActiveState(context);

        new (&m_stateStorage) NewState(std::forward<TArgs>(args)...);
        m_enterFn = &enterImpl<NewState>;
        m_updateFn = &updateImpl<NewState>;
        m_exitFn = &exitImpl<NewState>;
        m_destroyFn = &destroyImpl<NewState>;

        m_enterFn(&m_stateStorage, context);
    }

    /**
     * @brief Updates the current state. This should be called in the main loop.
     * @param context The shared context data for the robot.
     */
    void update(RobotStateContext& context)
    {
        if (m_updateFn != nullptr)
        {
            m_updateFn(&m_stateStorage, context);
        }
    }

    template <typename IdleStateT, typename MoveForwardStateT>
    void processRequestedTransition(RobotStateContext& context)
    {
        const MindStateId requested = context.requestedState;
        context.requestedState = MindStateId::None;

        switch (requested)
        {
            case MindStateId::Idle:
                transitionTo<IdleStateT>(context);
                break;
            case MindStateId::MoveForward:
                transitionTo<MoveForwardStateT>(context);
                break;
            case MindStateId::None:
            default:
                break;
        }
    }

private:
    static constexpr std::size_t kStateStorageSize = 64;
    using StateStorage = std::aligned_storage_t<kStateStorageSize, alignof(std::max_align_t)>;

    using StateEnterFn = void (*)(void* stateStorage, RobotStateContext& context);
    using StateUpdateFn = void (*)(void* stateStorage, RobotStateContext& context);
    using StateExitFn = void (*)(void* stateStorage, RobotStateContext& context);
    using StateDestroyFn = void (*)(void* stateStorage);

    template <typename TState>
    static void enterImpl(void* stateStorage, RobotStateContext& context)
    {
        static_cast<TState*>(stateStorage)->enter(context);
    }

    template <typename TState>
    static void updateImpl(void* stateStorage, RobotStateContext& context)
    {
        static_cast<TState*>(stateStorage)->update(context);
    }

    template <typename TState>
    static void exitImpl(void* stateStorage, RobotStateContext& context)
    {
        static_cast<TState*>(stateStorage)->exit(context);
    }

    template <typename TState>
    static void destroyImpl(void* stateStorage)
    {
        static_cast<TState*>(stateStorage)->~TState();
    }

    void exitAndDestroyActiveState(RobotStateContext& context)
    {
        if (m_exitFn != nullptr)
        {
            m_exitFn(&m_stateStorage, context);
        }
        destroyActiveState();
    }

    void destroyActiveState()
    {
        if (m_destroyFn != nullptr)
        {
            m_destroyFn(&m_stateStorage);
        }

        m_enterFn = nullptr;
        m_updateFn = nullptr;
        m_exitFn = nullptr;
        m_destroyFn = nullptr;
    }

    StateStorage m_stateStorage;
    StateEnterFn m_enterFn = nullptr;
    StateUpdateFn m_updateFn = nullptr;
    StateExitFn m_exitFn = nullptr;
    StateDestroyFn m_destroyFn = nullptr;
};
