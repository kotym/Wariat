#pragma once
#include "ComMath.hpp"
#include "ComMap.hpp"
#include "ComProtocol.hpp"

template<typename ComInterfaceClass>
class ComNavi
{
    enum class EState
    {
        None,
        Start,
        WaitAfterStart,
        SearchForWall,
        DriveToWall,
        RotatingToWall,
        MovingToWall,
        DriveAlongWall,
        DriveAround,
    };

public:
    ComNavi(const ComMap& comMap, ComInterfaceClass& _comInterface)
        : map(comMap)
        , comInterface(_comInterface)
    {}

    ~ComNavi() = default;

    EState state = EState::Start;
    bool bMoving = false;
public:
    void MoveFinished()
    {
        bMoving = false;
    }
    void Update(const Transform& transform)
    {
        switch (state)
        {
            case EState::None:
                break;
            case EState::Start:
                Start();
                state = EState::SearchForWall;
                break;
            case EState::WaitAfterStart:
                if (!bMoving) state = EState::SearchForWall;
                break;
            case EState::SearchForWall:
                SearchForWall(transform);
                break;
            case EState::DriveToWall:
                DriveToWall(transform);
                break;
            case EState::RotatingToWall:
                if (!bMoving) state = EState::MovingToWall;
                bMoving = true;
                comInterface.SendData(WariatCommon::Payload::MoveForward((destination - transform.position).Length()));
                break;
            case EState::MovingToWall:
                if (!bMoving) state = EState::DriveAlongWall;
                break;
            case EState::DriveAlongWall:
                DriveAlongWall(transform);
                break;
            case EState::DriveAround:
                DriveAround(transform);
                break;
            default:
                break;
        }
    }

    void Reset()
    {
        state = EState::Start;
    }

    void Start()
    {
        bMoving = true;
        comInterface.SendData(WariatCommon::Payload::Rotate(6.29f)); // 360 deg
    }

    void SearchForWall(const Transform& transform)
    {
        const int32_t mapCellSizeInCm = map.GetCellSizeInCm();

        Vector2<int32_t> nearestWall;
        float nearestDistSq = FLT_MAX;
        float nearestAcceptTreshold = 50 / mapCellSizeInCm;
        nearestAcceptTreshold *= nearestAcceptTreshold;

        Vector2<int32_t> wariatPosOnMap(transform.position / mapCellSizeInCm);
        Vector2<int32_t> checkedCell;
        const int32_t searchRange = 400 / mapCellSizeInCm;
        for (checkedCell.y = -searchRange; checkedCell.y < searchRange; ++checkedCell.y)
        {
            for (checkedCell.x = -searchRange; checkedCell.x < searchRange; ++checkedCell.x)
            {
                EMapCellState cell = map.GetCellState(wariatPosOnMap + checkedCell);

                if (cell == EMapCellState::Wall)
                {
                    const float lenSq = checkedCell.LengthSq();
                    if (lenSq < nearestDistSq)
                    {
                        nearestDistSq = lenSq;
                        nearestWall = checkedCell;
                        if(nearestDistSq < nearestAcceptTreshold)
                        {
                            goto wallFound;
                        }
                    }
                }                
            }
        }

        if (nearestDistSq > map.GetMapWidthInCells())
        {
            destination = wariatPosOnMap * mapCellSizeInCm;
            destination += nearestAcceptTreshold;
            // wall not found
            // drive wherever
            state = EState::DriveAround;
            return;
        }

    wallFound:
        state = EState::DriveToWall;
        destination = nearestWall;
    }

    void DriveToWall(const Transform& transform)
    {
        Vector2<float> path = destination - transform.position;
        if (path.LengthSq() < 30 * 30)
        {
            const float angle = acosf(path.x / path.Length()) - transform.rotation;
            comInterface.SendData(WariatCommon::Payload::Rotate(NormalizeAngle(angle)));
            state = EState::RotatingToWall;
            bMoving = true;
        }
    }

    void DriveAround(const Transform& transform)
    {
        Vector2<float> path = destination - transform.position;
        if (path.LengthSq() < 10 * 10)
        {
            state = EState::SearchForWall;
            return;
        }
    }

    void DriveAlongWall(const Transform& transform)
    {
        EMapCellState cell = map.GetCellState(transform.position);

        // AABB

        // Detection forward
        const float sinA = sin(transform.rotation);
        const float cosA = cos(transform.rotation);

        const float wariatWidth = 1.5; //* 5 cm
        const float wDivCosA = wariatWidth / cosA;

        for (int32_t y = 0; y < 10; ++y)
        {
            for (int32_t x = 0; x < 10; ++x)
            {
                // is ahead of wariat
                if (y > -x * cosA / sinA)
                    continue;
                // is 
            }
        }

        // Detection right
        const float wDivSinA = wariatWidth / sinA;
    }

protected:
    const ComMap& map;
    ComInterfaceClass& comInterface;


    Vector2<float> destination;
};