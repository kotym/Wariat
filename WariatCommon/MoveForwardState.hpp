#pragma once

#include "CommunicationProtocol.hpp"
#include "IState.hpp"
#include "MindStateId.hpp"
#include "RobotStateContext.hpp"

/**
 * @brief The robot moves forward.
 */
class MoveForwardState : public IState<MoveForwardState>
{
public:
    void enter(RobotStateContext& context)
    {
        context.publishCommand(WariatCommon::CommandType::CMD_MOVE_FORWARD);
    }

    void update(RobotStateContext& context)
    {
        if (context.isObstacleDetected)
        {
            context.requestTransition(MindStateId::Idle);
        }
    }

    void exit(RobotStateContext& context)
    {
        context.publishCommand(WariatCommon::CommandType::CMD_STOP);
    }
};
