#pragma once

struct RobotStateContext;

/**
 * @brief Base template for a state in the Hierarchical State Machine.
 *
 * This uses the Curiously Recurring Template Pattern (CRTP) to achieve
 * static polymorphism, avoiding the overhead of virtual functions (v-tables)
 * which is crucial for performance on microcontrollers like the ESP32.
 *
 * @tparam ConcreteState The class that inherits from IState.
 */
template <typename ConcreteState>
class IState
{
public:
    /**
     * @brief Called once when the state machine enters this state.
     * @param context The shared context data for the robot.
     */
    void enter(RobotStateContext& context)
    {
        static_cast<ConcreteState*>(this)->enter(context);
    }

    /**
     * @brief Called on every update tick of the state machine.
     * This is where the state's main logic resides.
     * @param context The shared context data for the robot.
     */
    void update(RobotStateContext& context)
    {
        static_cast<ConcreteState*>(this)->update(context);
    }

    /**
     * @brief Called once when the state machine exits this state.
     * @param context The shared context data for the robot.
     */
    void exit(RobotStateContext& context)
    {
        static_cast<ConcreteState*>(this)->exit(context);
    }

protected:
    // Ensure that this class can only be used as a base class.
    IState() = default;
    ~IState() = default;
};
