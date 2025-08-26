// Fill   out your copyright notice in the Description page of Project Settings.


#include "WarriorGamePlayTags.h"

namespace WarriorGamePlayTags
{
	/** Input Tags **/
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "Input.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Mouse_Lock, "Input.Mouse.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "Input.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Walk, "Input.Walk");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Sprint, "Input.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Jump, "Input.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_OrientRotation, "Input.OrientRotation");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_GamePad_Lock, "Input.GamePad.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Roll, "Input.Roll");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SwitchTarget, "Input.SwitchTarget");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SpecialWeaponAbility_Light, "Input.SpecialWeaponAbility.Light");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SpecialWeaponAbility_Heavy, "Input.SpecialWeaponAbility.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_PickUp_Stones, "Input.PickUp.Stones");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipAxe, "Input.EquipAxe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UnequipAxe, "Input.UnequipAxe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_LightAttack_Axe, "Input.LightAttack.Axe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_HeavyAttack_Axe, "Input.HeavyAttack.Axe");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld, "Input.MustBeHeld");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld_Block, "Input.MustBeHeld.Block");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable, "Input.Toggleable");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable_TargetLock, "Input.Toggleable.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable_Rage, "Input.Toggleable.Rage");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Finisher, "Input.Finisher");

	/** Warrior Player Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip_Axe, "Player.Ability.Equip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Unequip_Axe, "Player.Ability.Unequip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_Light_Axe, "Player.Ability.Attack.Light.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_Heavy_Axe, "Player.Ability.Attack.Heavy.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_HitPause, "Player.Ability.HitPause");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Roll, "Player.Ability.Roll");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Block, "Player.Ability.Block");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_TargetLock, "Player.Ability.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Rage, "Player.Ability.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Finisher, "Player.Ability.Finisher");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_SpecialWeaponAbility_Light, "Player.Ability.SpecialWeaponAbility.Light");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_SpecialWeaponAbility_Heavy, "Player.Ability.SpecialWeaponAbility.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_PickUp_Stones, "Player.Ability.PickUp.Stones");
	
	UE_DEFINE_GAMEPLAY_TAG(Player_Colldown_SpecialWeaponAbility_Light, "Player.Cooldown.SpecialWeaponAbility.Light");
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_SpecialWeaponAbility_Heavy, "Player.Cooldown.SpecialWeaponAbility.Heavy");

	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Axe, "Player.Weapon.Axe");

	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Axe, "Player.Event.Equip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Unequip_Axe, "Player.Event.Unequip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_HitPause, "Player.Event.HitPause");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SuccessfulBlock, "Player.Event.SuccessfulBlock");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SwitchTarget_Left, "Player.Event.SwitchTarget.Left");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SwitchTarget_Right, "Player.Event.SwitchTarget.Right");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_ActivateRage, "Player.Event.ActivateRage");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Finisher, "Player.Event.Finisher");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_MovementReady, "Player.Event.MovementReady");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_ConsumeStones, "Player.Event.ConsumeStones");

	UE_DEFINE_GAMEPLAY_TAG(Player_Status_JumpToFinisher, "Player.Status.JumpToFinisher");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rolling, "Player.Status.Rolling");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Blocking, "Player.Status.Blocking");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Invincible, "Player.Status.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_TargetLock, "Player.Status.TargetLock");
	
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_Activating, "Player.Status.Rage.Activating");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_Active, "Player.Status.Rage.Active");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_Full, "Player.Status.Rage.Full");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_None, "Player.Status.Rage.None");

	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Light, "Player.SetByCaller.AttackType.Light");
	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Heavy, "Player.SetByCaller.AttackType.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Finisher, "Player.SetByCaller.AttackType.Finisher");
	/** Warrior Player Tags **/

	/** Samurai Player Tags **/
	UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipKatana, "Input.EquipKatana");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UnequipKatana, "Input.UnequipKatana");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_LightAttack_Katana, "Input.LightAttack.Katana");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_HeavyAttack_Katana, "Input.HeavyAttack.Katana");

	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Katana, "Player.Weapon.Katana");
	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Katana_Sheath, "Player.Weapon.Katana.Sheath");

	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Katana, "Player.Event.Equip.Katana");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Unequip_Katana, "Player.Event.Unequip.Katana");

	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip_Katana, "Player.Ability.Equip.Katana");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Unequip_Katana, "Player.Ability.Unequip.Katana");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_Light_Katana, "Player.Ability.Attack.Light.Katana");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_Heavy_Katana, "Player.Ability.Attack.Heavy.Katana");

	/** Samurai Player Tags **/

	/** Enemy Player Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Melee, "Enemy.Ability.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Ranged, "Enemy.Ability.Ranged");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_StrongRanged, "Enemy.Ability.StrongRanged");
	
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Weapon, "Enemy.Weapon");
	
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_SummonEnemies, "Enemy.Ability.SummonEnemies");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_SpawnStone, "Enemy.Ability.SpawnStone");
	
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_SummonEnemies, "Enemy.Event.SummonEnemies");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_Strafing, "Enemy.Status.Strafing");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_UnderAttack, "Enemy.Status.UnderAttack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_Unblockable, "Enemy.Status.Unblockable");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_Finishable, "Enemy.Status.Finishable");
	/** Enemy Player Tags **/

	/** Shared Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_HitReact, "Shared.Ability.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_Death, "Shared.Ability.Death");

	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_HitReact, "Shared.Event.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_SpawnProjectile, "Shared.Event.SpawnProjectile");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_MeleeHit, "Shared.Event.MeleeHit");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_FootstepSound, "Shared.Event.FootstepSound");

	UE_DEFINE_GAMEPLAY_TAG(Shared_SetByCaller_BaseDamage, "Shared.SetByCaller.BaseDamage");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Dead, "Shared.Status.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Front, "Shared.Status.HitReact.Front");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Left, "Shared.Status.HitReact.Left"); 
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Right, "Shared.Status.HitReact.Right");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Back, "Shared.Status.HitReact.Back");
	
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Invincible, "Shared.Status.Invincible");
	
	/* Foley Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Handplant, "Foley.Event.Handplant");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Jump, "Foley.Event.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Land, "Foley.Event.Land");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Run, "Foley.Event.Run");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_RunBackwds, "Foley.Event.RunBackwds");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_RunStrafe, "Foley.Event.RunStrafe");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Scuff, "Foley.Event.Scuff");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_ScuffPivot, "Foley.Event.ScuffPivot");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_ScuffWall, "Foley.Event.ScuffWall");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Tumble, "Foley.Event.Tumble");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Walk, "Foley.Event.Walk");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_WalkBackwds, "Foley.Event.WalkBackwds");
	/* Foley Event Tags */

	/* Game Data tags */
	UE_DEFINE_GAMEPLAY_TAG(GameData_Level_SurvivalGameModeMap, "GameData.Level.SurvivalGameModeMap");
	UE_DEFINE_GAMEPLAY_TAG(GameData_Level_MainMenuMap, "GameData.Level.MainMenuMap");
	
	UE_DEFINE_GAMEPLAY_TAG(GameData_SaveGame_Slot_1, "GameData.SaveGame.Slot.1");
	/* Game Data tags */
}
