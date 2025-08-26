#pragma once

UENUM()
enum class EWarriorConfirmType : uint8
{
	EWC_Yes,
	EWC_No,
};

UENUM()
enum class EWarriorValidType : uint8
{
	EWT_Valid,
	EWT_Invalid,
};

UENUM()
enum class EWarriorSuccessType : uint8
{
	EWS_Successful,
	EWS_Failed,
};

UENUM()
enum class EWarriorCountDownActionInput : uint8
{
	Start,
	Cancel,
};

UENUM()
enum class EWarriorCountDownActionOutput : uint8
{
	Updated,
	Completed,
	Canceled
};

UENUM(BlueprintType)
enum class EWarriorGameDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	Hell
};

UENUM()
enum class EWarriorInputMode : uint8
{
	GameOnly,
	UIOnly
};