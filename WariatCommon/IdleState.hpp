#pragma once

#include <cstdint>

#include "CommunicationProtocol.hpp"
#include "IState.hpp"
#include "MindStateId.hpp"
#include "RobotStateContext.hpp"

/**
 * @brief The robot is idle and waiting for a command.
 */
class IdleState : public IState<IdleState>
{
public:
    void enter(RobotStateContext& context)
    {
        m_entryTimeMs = context.currentTimeMs;
        context.publishCommand(WariatCommon::CommandType::CMD_STOP);
    }

    void update(RobotStateContext& context)
    {
        const uint64_t elapsedMs = context.currentTimeMs - m_entryTimeMs;
        if (elapsedMs > 2000)
        {
            context.requestTransition(MindStateId::MoveForward);
        }
    }

    void exit(RobotStateContext& context)
    {
        (void)context;
    }

private:
    uint64_t m_entryTimeMs = 0;
};
