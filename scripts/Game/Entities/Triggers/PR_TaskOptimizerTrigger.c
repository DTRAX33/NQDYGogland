/*
Author: PapaReap

ToDo:
check custom xp is
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_TaskOptimizerTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_TaskOptimizerTrigger : PR_CoreTrigger
{
	//------------------------------------------------------------------------------------------------
	void PR_TaskOptimizerTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//! PR Task Optimizer: Name of task to optimize. Will also be used to check player distance to task for a 35% volunteer XP bonus.
	[Attribute(desc: "Name of task to optimize. Will also be used to check player distance to task for a 35% volunteer XP bonus.  ", category: "PR Task Optimizer")]
	string m_sTaskName;

	//! PR Task Optimizer: Set task type to be rewarded. XP listed in (parentheses).
	[Attribute("Type of task", UIWidgets.ComboBox,"Set task type to be rewarded. XP listed in (parentheses).  ", enums: ParamEnumArray.FromEnum(PR_EXPRewards), category: "PR Task Optimizer")]
	protected PR_EXPRewards m_PlayerXPTaskReward;

	//! PR Task Optimizer: XP multiplier to add or extract the task reward.
	[Attribute("1", desc: "XP multiplier to add or extract the task reward.  ", category: "PR Task Optimizer")]
	protected float m_fXPMultiplier;

	//! PR Task Optimizer: Custom XP amount to add to player. There will be no notification shown in game.
	[Attribute("0", desc: "Custom XP amount to add to player. There will be no notification shown in game.  ", category: "PR Task Optimizer")]
	protected int m_iCustomXP;

	//! PR Task Optimizer: Maximum distance a player can be from task before losing the 35% volunteer bonus.
	[Attribute("500", desc: "Maximum distance a player can be from task before losing the 35% volunteer bonus.  ", category: "PR Task Optimizer")]
	protected float m_fMaxDistanceForVolunteerBonus;

	//! PR Task Optimizer:
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "If 'Use Task Fail Object' is checked and object doesn't exist, player XP will be deducted instead of increased.  ", category: "PR Task Optimizer")]
	protected bool m_bUseTaskFailObject;

	//! PR Task Optimizer: Name of object to use for task failure. If object is deleted before trigger is activated, players will lose XP.
	[Attribute(desc: "Name of object to use for task failure. If object is deleted before trigger is activated, players will lose XP.  ", category: "PR Task Optimizer")]
	protected string m_sTaskFailObject;

	//! PR Task Optimizer: Individual Prefabs to remove
	[Attribute(desc: "Individual Prefabs to remove.  ", category: "PR Task Optimizer")]
	protected ref array<ref PR_RemovePrefabFilter> m_aRemovePrefabDetails;

	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (m_bIsTriggerActivated)
			return;

		m_bIsTriggerActivated = true;
		m_sLogMode = "(OnActivate)";

		int delay = m_iDelayTimerMin * 1000;

		if (m_bUseRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerMin * 1000, m_iDelayTimerMax * 1000);

		Print(string.Format("[PR_TaskOptimizerTrigger] %1 : Trigger: %2 : delay: %3", m_sLogMode, m_sTriggerName, delay), LogLevel.WARNING);

		IEntity persistentObject;

		if (m_bUsePersistence)
		{
			if (m_bIsTestingMode)
				persistentObject = GetWorld().FindEntityByName(m_sPersistentObject);
			else
				persistentObject = GetGame().GetWorld().FindEntityByName(m_sPersistentObject);

			if (!persistentObject)
			{
				Print(string.Format("[PR_TaskOptimizerTrigger] %1 : No persistentObject: %2. Exiting!", m_sLogMode, m_sPersistentObject), LogLevel.WARNING);
				return;
			}
		}

		if (m_bUseTaskFailObject)
		{
			IEntity taskFailObject = GetGame().GetWorld().FindEntityByName(m_sTaskFailObject);
			if (taskFailObject)
			{
				SetTaskFailObject(taskFailObject);
				SetSubtractXP(false);
			}
			else
			{
				SetTaskFailObject(null);
				SetSubtractXP(true);
			}
		}
		else
			SetSubtractXP(false);

		if (m_aRemovePrefabDetails.Count() > 0)
		{
			GetGame().GetCallqueue().CallLater(CleanupPrefabs, delay, false, m_aRemovePrefabDetails);
		}
		else
		{
			if (m_PlayerXPTaskReward > 0)
			{
				SCR_EXPRewards rewardID = GetXPReward();
				Print(string.Format("[PR_TaskOptimizerTrigger] %1 : m_PlayerXPTaskReward: %2", m_sLogMode, m_PlayerXPTaskReward), LogLevel.WARNING);

				GetGame().GetCallqueue().CallLater(RewardTask, delay, false, rewardID);
			}
		}
	}

	protected bool m_bSubtractXP;
	//------------------------------------------------------------------------------------------------
	//! sets m_bSubtractXP
	void SetSubtractXP(bool subtractXP)
	{
		m_bSubtractXP = subtractXP;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_bSubtractXP;
	bool GetSubtractXP()
	{
		return m_bSubtractXP;
	}

	protected IEntity m_TaskFailObject;
	//------------------------------------------------------------------------------------------------
	//! sets m_TaskFailObject
	void SetTaskFailObject(IEntity taskFailObject)
	{
		m_TaskFailObject = taskFailObject;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_TaskFailObject;
	IEntity GetTaskFailObject()
	{
		return m_TaskFailObject;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Custom XP Rewards
	protected SCR_EXPRewards GetXPReward()
	{
		switch (m_PlayerXPTaskReward)
		{
			case 1: // Clear Area (55)
			{
				return SCR_EXPRewards.CUSTOM_2;
			};
			case 2: // Defend Area (55)
			{
				return SCR_EXPRewards.CUSTOM_3;
			};
			case 3: // Defend Target (60)
			{
				return SCR_EXPRewards.CUSTOM_4;
			};
			case 4: // Defend Area and Target (65)
			{
				return SCR_EXPRewards.CUSTOM_5;
			};
			case 5: // Deliver Supplies (40)
			{
				return SCR_EXPRewards.CUSTOM_6;
			};
			case 6: // Deliver Vehicles (45)
			{
				return SCR_EXPRewards.CUSTOM_7;
			};
			case 7: // Deliver Intel (50)
			{
				return SCR_EXPRewards.CUSTOM_8;
			};
			case 8: // Destroy (50)
			{
				return SCR_EXPRewards.CUSTOM_9;
			};
			case 9: // Kill (55)
			{
				return SCR_EXPRewards.CUSTOM_10;
			};
			case 10: // Kill HVT (60)
			{
				return SCR_EXPRewards.CUSTOM_11;
			};
			case 11: // Extraction (50)
			{
				return SCR_EXPRewards.CUSTOM_12;
			};
			case 12: // Move (40)
			{
				return SCR_EXPRewards.CUSTOM_13;
			};
			case 13: // Last (40)
			{
				return SCR_EXPRewards.CUSTOM_14;
			};
		}
		return SCR_EXPRewards.CUSTOM_14;
	}

	//------------------------------------------------------------------------------------------------
	//! Reward Task
	void RewardTask(SCR_EXPRewards rewardID)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(m_OwnerFactionKey);

		Print(string.Format("[PR_TaskOptimizerTrigger] (RewardTask) %1 : rewardID: %2", m_sLogMode, rewardID), LogLevel.WARNING);
		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		if (comp && !GetTaskManager().IsProxy())
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();
			array<int> players = {};
			playerManager.GetPlayers(players);
			Faction playerFaction;
			IEntity playerEntity;
			bool volunteer = false;
			IEntity taskEntity = GetGame().GetWorld().FindEntityByName(m_sTaskName);

			foreach (int playerId : players)
			{
				playerEntity = playerManager.GetPlayerControlledEntity(playerId);
				if (!playerEntity)
					continue;

				playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(playerEntity);
				if (playerFaction != m_OwnerFaction)
					continue;

				if (taskEntity)
				{
					float actualDistance = vector.DistanceXZ(taskEntity.GetOrigin(), playerEntity.GetOrigin());
					Print(string.Format("[PR_TaskOptimizerTrigger] (RewardTask) %1 : Trigger: %2 : actualDistance: %3", m_sLogMode, m_sTriggerName, actualDistance), LogLevel.WARNING);
					if (actualDistance <= m_fMaxDistanceForVolunteerBonus)
						volunteer = true;
				}
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXPPlayer = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!compXPPlayer)
			return;	
		int playerXPWithPenalty = compXPPlayer.GetPlayerXP();
		Print(string.Format("[PR_TaskOptimizerTrigger] (RewardTask) %1 : Trigger: %2 : playerXPWithPenalty Pre: %3", m_sLogMode, m_sTriggerName, playerXPWithPenalty), LogLevel.WARNING);	
				if (GetSubtractXP())
				{
					int xpSubtract = comp.GetXPRewardAmount(SCR_EXPRewards.CUSTOM_20);
					
					int XP = comp.GetXPRewardAmount(rewardID);
					XP = -XP;
					int addCustomXP = 0;
					if (xpSubtract > XP) // -40 > -50
						addCustomXP =  XP - xpSubtract;
					//else
						//addCustomXP = xpSubtract - XP;

					if (!(addCustomXP == 0))
						comp.AwardXP(playerId, SCR_EXPRewards.CUSTOM_1, 1, false, addCustomXP);
					
					comp.AwardXP(playerId, SCR_EXPRewards.CUSTOM_20, 1, false, 0);
					Print(string.Format("[PR_TaskOptimizerTrigger] (RewardTask) %1 : Trigger: %2 : xpSubtract: %3", m_sLogMode, m_sTriggerName, xpSubtract), LogLevel.WARNING);
					Print(string.Format("[PR_TaskOptimizerTrigger] (RewardTask) %1 : Trigger: %2 : addCustomXP: %3", m_sLogMode, m_sTriggerName, addCustomXP), LogLevel.WARNING);
				}
				else
					comp.AwardXP(playerId, rewardID, m_fXPMultiplier, volunteer, m_iCustomXP);
				
				playerXPWithPenalty = compXPPlayer.GetPlayerXP();
				Print(string.Format("[PR_TaskOptimizerTrigger] (RewardTask) %1 : Trigger: %2 : playerXPWithPenalty Post: %3", m_sLogMode, m_sTriggerName, playerXPWithPenalty), LogLevel.WARNING);	
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup prefabs
	protected void CleanupPrefabs(array<ref PR_RemovePrefabFilter> removePrefabDetails)
	{
		ResourceName prefabType;
		int prefabCountToRemove;
		IEntity prefabEntity;
		int requiredPrefabCount = 0;
		int realPrefabCount = 0;

		foreach (PR_RemovePrefabFilter prefabDetails : m_aRemovePrefabDetails)
		{
			prefabType = prefabDetails.m_sSpecificPrefabName;
			prefabCountToRemove = prefabDetails.m_iPrefabCountToRemove;
			requiredPrefabCount = requiredPrefabCount + prefabCountToRemove;
			//int realPrefabCount = 0;

			if (!prefabType)
				continue;
			//m_bUseTaskFailObject
			//m_sTaskFailObject
			if (prefabCountToRemove > 0)
			{
				int _i = 0;
				while (prefabCountToRemove > _i)
				{
					prefabEntity = FindNearestPrefab(prefabType, m_Trigger.GetOrigin(), m_fRadius);
					if (prefabEntity)
					{
						SCR_EntityHelper.DeleteEntityAndChildren(prefabEntity);
						realPrefabCount++;
					}

					_i++;
				}

				//if (prefabCountToRemove > realPrefabCount && m_bUseTaskFailObject)
				//{
				//	if (GetTaskFailObject())
				//		SCR_EntityHelper.DeleteEntityAndChildren(GetTaskFailObject());

				//	SetSubtractXP(true);
				//}
				//else
				//	SetSubtractXP(false);

				//if (m_PlayerXPTaskReward > 0)
				//{
				//	SCR_EXPRewards rewardID = GetXPReward();
				//	Print(string.Format("[PR_TaskOptimizerTrigger] (CleanupPrefabs) %1 : m_PlayerXPTaskReward: %2", m_sLogMode, m_PlayerXPTaskReward), LogLevel.WARNING);

				//	GetGame().GetCallqueue().CallLater(RewardTask, delay, false, rewardID);
				//}
			}
		}
		
		Print(string.Format("[PR_TaskOptimizerTrigger] (CleanupPrefabs) %1 : requiredPrefabCount: %2 : realPrefabCount: %3", m_sLogMode, requiredPrefabCount, realPrefabCount), LogLevel.WARNING);
		if (requiredPrefabCount > realPrefabCount && m_bUseTaskFailObject)
		{
			if (GetTaskFailObject())
				SCR_EntityHelper.DeleteEntityAndChildren(GetTaskFailObject());

			SetSubtractXP(true);
		}
		else
			SetSubtractXP(false);
		
		if (m_PlayerXPTaskReward > 0)
		{
			SCR_EXPRewards rewardID = GetXPReward();
			Print(string.Format("[PR_TaskOptimizerTrigger] (CleanupPrefabs) %1 : m_PlayerXPTaskReward: %2", m_sLogMode, m_PlayerXPTaskReward), LogLevel.WARNING);

			GetGame().GetCallqueue().CallLater(RewardTask, 1000, false, rewardID);
		}
	}

	//--- From Arkensor's EPF scripts, thanks. I've tweaked it to work for my use.
	protected ResourceName m_sQueryPrefab;
	protected IEntity m_QueryResult;

	//------------------------------------------------------------------------------------------------
	protected IEntity FindNearestPrefab(ResourceName prefab, vector origin, float radius)
	{
		m_sQueryPrefab = prefab;
		m_QueryResult = null;
		GetGame().GetWorld().QueryEntitiesBySphere(origin, radius, FindNearestPrefab_FirstEntity, FindNearestPrefab_FilterEntities);
		IEntity result = m_QueryResult;
		m_QueryResult = null;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindNearestPrefab_FirstEntity(IEntity ent)
	{
		m_QueryResult = ent;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindNearestPrefab_FilterEntities(IEntity ent)
	{
		EntityPrefabData prefabData = ent.GetPrefabData();
		if (!prefabData)
			return false;

		BaseContainer baseContainer = ent.GetPrefabData().GetPrefab();
		if (!baseContainer)
			return false;

		string classname = baseContainer.GetClassName();
		if (!(classname == "GenericEntity"))
			return false;

		return SCR_BaseContainerTools.GetPrefabResourceName(baseContainer).Contains(m_sQueryPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the prefab the entity uses
	//! \param entity Instance of which to get the prefab name
	//! \return the resource name of the prefab or empty string if no prefab was used or entity is invalid
	protected ResourceName GetPrefabName(IEntity entity)
	{
		if (!entity)
			return string.Empty;

		return SCR_BaseContainerTools.GetPrefabResourceName(entity.GetPrefabData().GetPrefab());
	}
}

enum PR_EXPRewards
{
	"None (0)" = 0,
	"Clear Area (55)" = 1, 				// 1 CUSTOM_2
	"Defend Area (55)" = 2, 				// 2	 CUSTOM_3
	"Defend Target (60)" = 3, 			// 3 CUSTOM_4
	"Defend Area and Target (65)" = 4,	// 4	 CUSTOM_5
	"Deliver Supplies (40)" = 5, 		// 5	 CUSTOM_6
	"Deliver Vehicles (45)" = 6, 		// 6	 CUSTOM_7
	"Deliver Intel (50)" = 7, 			// 7	 CUSTOM_8
	"Destroy (50)" = 8, 					// 8	 CUSTOM_9
	"Kill (55)" = 9, 					// 9	 CUSTOM_10
	"Kill HVT (60)" = 10, 				// 10 CUSTOM_11
	"Extraction (50)" = 11, 				// 11 CUSTOM_12
	"Move (40)" = 12, 					// 12 CUSTOM_13
	"Last (40)" = 13, 					// 13 CUSTOM_14
}

[BaseContainerProps()]
class PR_RemovePrefabFilter
{
	//! PR Task Optimizer:
	[Attribute(params: "et", desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "PR Task Optimizer")]
	ResourceName m_sSpecificPrefabName;

	//! PR Task Optimizer:
	//[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "PR Task Optimizer")]
	//bool m_bIncludeChildren;

	//! PR Task Optimizer: Amount of prefabs to remove, should match slot prefabs required for task. Early removal of prefabs before trigger activates could result in XP loses.
	[Attribute("1", desc: "Amount of prefabs to remove, should match slot prefabs required for task. Early removal of prefabs before trigger activates could result in XP loses.  ", category: "PR Task Optimizer")]
	int m_iPrefabCountToRemove;

	//BaseContainer m_PrefabContainer;
}
