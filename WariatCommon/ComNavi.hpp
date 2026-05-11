#pragma once
#include "ComMath.hpp"
#include "ComMap.hpp"
#include "ComProtocol.hpp"
#include "WariatDrive.hpp"

namespace WariatCommon
{

enum class EState
{
    None,
    Start,
    SearchForWall,
    DriveToWall,
    DriveAlongWall,
    DriveAround,
};
// TODO why ComNavi needs ComInterface only to pass it to WariatDrive. Fix it
template<typename ComInterfaceClass>
class ComNavi
{
public:

    ComNavi(ComMap& comMap, WariatDrive<ComInterfaceClass>& WariatDrive)
        : map(comMap), wariatDrive(WariatDrive)
    {
        activeInstance = this;
    }

    ~ComNavi() = default;

    void Update(const Transform& transform)
    {
        if (bMoving) return;

        switch (state)
        {
            case EState::None:
                break;
            case EState::Start:
                Start();
                break;
            case EState::SearchForWall:
                SearchForWall(transform);
                break;
            case EState::DriveToWall:
                DriveToWall(transform);
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
        wariatDrive.Rotate(6.29f, &ComNavi::OnMoveFinished); // 360 deg
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
        {
            float mapWidthSq = map.GetMapWidthInCells();
            mapWidthSq *= mapWidthSq;

            if (nearestDistSq > mapWidthSq)
            {
                destination = wariatPosOnMap * mapCellSizeInCm;
                destination += nearestAcceptTreshold;
                // wall not found
                // drive wherever
                state = EState::DriveAround;
                return;
            }
        }

    wallFound:
        state = EState::DriveToWall;

        Vector2<float> relativeDestination = nearestWall;
        relativeDestination.Normalize();
        relativeDestination *= nearestWall.Length() - toWallDistanceCm;

        destination = (wariatPosOnMap + relativeDestination) * mapCellSizeInCm;
    }

    void DriveToWall(const Transform& transform)
    {
        bMoving = true;
        wariatDrive.DriveTo(destination, &ComNavi::OnMoveFinished);
    }

    void DriveAround(const Transform& transform)
    {
        bMoving = true;
        wariatDrive.DriveTo(destination, &ComNavi::OnMoveFinished);
    }

    static void OnMoveFinished()
    {
        if (activeInstance)
            activeInstance->MoveFinished();
    }

    void MoveFinished()
    {
        bMoving = false;

        switch (state)
        {
            case WariatCommon::EState::None:
                break;
            case WariatCommon::EState::Start:
                state = EState::SearchForWall;
                break;
            case WariatCommon::EState::SearchForWall:
                break;
            case WariatCommon::EState::DriveToWall:
                state = EState::DriveAlongWall;
                break;
            case WariatCommon::EState::DriveAlongWall:
                break;
            case WariatCommon::EState::DriveAround:
                state = EState::SearchForWall;
                break;
            default:
                break;
        }
    }

    //void FindWallAngle

    void DriveAlongWall(const Transform& transform)
    {
        // AABB ?

        float aheadDistSq = FLT_MAX, rightDistSq = FLT_MAX;
        Vector2<int32> aheadPos, rightPos;

        // Detection forward
        const float sinA = sinf(transform.rotation);
        const float cosA = cosf(transform.rotation);
        const float oneDivCosA = 1 / cosA;
        const float tanA = sinA * oneDivCosA;

        const float wariatWidth = 1.5; //* 5 cm

        const bool bTopHalf = transform.rotation > 0;
        const bool bRightHalf = transform.rotation < M_PI_2 && transform.rotation > -M_PI_2;

        int32_t mapCellSizeInCm = map.GetCellSizeInCm();
        Vector2<int32_t> wariatPosOnMap(transform.position / mapCellSizeInCm);
        Vector2<int32_t> checkedCell;
        const int32_t searchRange = 400 / mapCellSizeInCm;

        map.lastScannedAhead.clear();
        map.lastScannedRight.clear();

        for (checkedCell.y = -searchRange; checkedCell.y < searchRange; ++checkedCell.y)
        {
            for (checkedCell.x = -searchRange; checkedCell.x < searchRange; ++checkedCell.x)
            {
                EMapCellState cell = map.GetCellState(wariatPosOnMap + checkedCell);
                //if (cell != EMapCellState::Wall) // TODO uncomment after debug
                //{
                //    continue;
                //}

                //
                //  TODO Idk why byt most of this equations had to be reversed to the equations from desmos (> instead of <) in order to work
                //

                // is ahead of wariat
                if (bTopHalf ? (checkedCell.y > -checkedCell.x * cosA / sinA) : (checkedCell.y < -checkedCell.x * cosA / sinA))
                {
                    // is the width of the wariat ahead
                    if (/*bRightHalf ? */(cosA * checkedCell.y > sinA * checkedCell.x - wariatWidth) && (cosA * checkedCell.y < sinA * checkedCell.x + wariatWidth))
                                   //: (cosA * checkedCell.y < sinA * checkedCell.x - wariatWidth) && (cosA * checkedCell.y > sinA * checkedCell.x + wariatWidth))
                    {
                        map.lastScannedAhead.push_back(checkedCell + wariatPosOnMap); // TODO remove debug
                        if (cell != EMapCellState::Wall) // TODO remove debug
                        {
                            continue;
                        }
                        // the cell is ahead in the width of the wariat
                        const float cellDistSq = checkedCell.LengthSq();
                        if (cellDistSq < aheadDistSq)
                        {
                            aheadDistSq = cellDistSq;
                            aheadPos = checkedCell;
                        }
                    }
                }
                // in on the right
                else if (bRightHalf ? (checkedCell.y > tanA * checkedCell.x) : (checkedCell.y < tanA * checkedCell.x))
                {
                    // is in the width ot the wariat on the side
                    if (/*bTopHalf ? */(checkedCell.y * sinA < -cosA * checkedCell.x - wariatWidth) && (checkedCell.y * sinA > -cosA * checkedCell.x - 2 * wariatWidth))
                               //  : (checkedCell.y * sinA > -cosA * checkedCell.x - wariatWidth) && (checkedCell.y * sinA < -cosA * checkedCell.x - 2 * wariatWidth))
                    {
                        /////////////////////////////////////////////////////// Cos jest zle z tym wykrywaniem
                        map.lastScannedRight.push_back(checkedCell + wariatPosOnMap); // TODO remove debug
                        if (cell != EMapCellState::Wall)// TODO remove debug
                        {
                            continue;
                        }
                        // the cell is one the right in the width of wariat
                        const float cellDistSq = checkedCell.LengthSq();
                        if (cellDistSq < rightDistSq)
                        {
                            rightDistSq = cellDistSq;
                            rightPos = checkedCell;
                        }
                    }
                }
            }
        }
        float aheadDist = sqrt(aheadDistSq);
        float rightDist = sqrt(rightDistSq);

        float rightDistRel = fabsf(rightDist - toWallDistanceCm);

        // TODO Idk why but it turns in the wrong direction (angle < 0 should be right, but is left)

        if (aheadDist < toWallDistanceCm)
        {
            // turn left
            wariatDrive.Rotate(-M_PI/9, &ComNavi::OnMoveFinished);
            //wariatDrive.RotateAndDrive(M_PI / 9, 5, &ComNavi::OnMoveFinished);
        }
        else if (rightDist > 142133)
        {
            wariatDrive.RotateAndDrive(M_PI / 9, 15, &ComNavi::OnMoveFinished);
        }
        else if (rightDistRel < 10 || rightDistRel > 5)
        {
            // turn right
            if (rightDist < toWallDistanceCm)
                wariatDrive.RotateAndDrive(-M_PI / 9, 15, &ComNavi::OnMoveFinished);
            else
                wariatDrive.RotateAndDrive(M_PI / 9, 15, &ComNavi::OnMoveFinished);

            //wariatDrive.Rotate(-M_PI/9, &ComNavi::OnMoveFinished);
        }
        else
        {
            // drive forward
            wariatDrive.MoveForward(15, &ComNavi::OnMoveFinished);
        }
        bMoving = true;
    }

//protected:
    ComMap& map;
    WariatDrive<ComInterfaceClass>& wariatDrive;

    Vector2<float> destination;

    EState state = EState::Start;
    bool bMoving = false;
    int32_t toWallDistanceCm = 35;

    inline static ComNavi<ComInterfaceClass>* activeInstance = nullptr;
};

}