// ComboMoveData.cpp
#include "ComboMoveData.h"
#include "Engine/Engine.h"

void UComboMoveData::SyncNextMovesID()
{
    NextMovesID.Empty();
    NextMovesID.Reserve(NextMoves.Num());
    
    for (const TWeakObjectPtr<const UComboMoveData>& NextMovePtr : NextMoves)
    {
        if (NextMovePtr.IsValid())
        {
            NextMovesID.Add(NextMovePtr->MoveID);
        }
        else
        {
            // 如果弱指针无效，添加一个特殊标记
            NextMovesID.Add(FName("INVALID_MOVE"));
            UE_LOG(LogTemp, Warning, TEXT("Found invalid NextMove pointer in %s"), *MoveID.ToString());
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("Synced NextMovesID for %s: %d entries"), 
        *MoveID.ToString(), NextMovesID.Num());
}

void UComboMoveData::AddNextMove(const UComboMoveData* NextMove)
{
    if (!NextMove)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to add null NextMove to %s"), *MoveID.ToString());
        return;
    }
    
    // 添加到 NextMoves 数组
    NextMoves.Add(TWeakObjectPtr<const UComboMoveData>(NextMove));
    
    // 添加到 NextMovesID 数组
    NextMovesID.Add(NextMove->MoveID);
    
    UE_LOG(LogTemp, Verbose, TEXT("Added NextMove %s to %s"), 
        *NextMove->MoveID.ToString(), *MoveID.ToString());
}

void UComboMoveData::ClearNextMoves()
{
    NextMoves.Empty();
    NextMovesID.Empty();
    
    UE_LOG(LogTemp, Verbose, TEXT("Cleared NextMoves for %s"), *MoveID.ToString());
}
