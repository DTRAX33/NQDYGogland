/*
Author: PapaReap

ToDo:
Make trigger run loop option
Maybe make a player distance check to task?
Get global array updated. Check?
*/

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class PR_SpawnTaskTriggerClass : PR_CoreTriggerClass
{
	// prefab properties here
}

class PR_SpawnTaskTrigger : PR_CoreTrigger
{
	//! PR Task Spawner: Tasks - Individual Tasks - Individual tasks to assign, with optional move feature.
	[Attribute(desc: "Individual tasks to assign, with optional move feature.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	protected ref array<ref PR_TaskDetails> m_aIndividualTasks;

	//! PR Task Spawner: Tasks - Global Task Pool
	//! Use tasks from the global task pool set by 'PR_TaskCollection'.
	//! NOTE: Overrides 'Individual Tasks' from above.
	static string hintUseAllAvailableTasksFromPool = (
	"Use tasks from the global task pool set by 'PR_TaskCollection'." + "<br />" +
	"----------------------------------------------------------------------------" + "<br />" +
	"NOTE: Overrides 'Individual Tasks' from above.  "
	);
	[Attribute("false", UIWidgets.CheckBox, hintUseAllAvailableTasksFromPool, category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected bool m_bUseTaskPool;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute("false", UIWidgets.CheckBox,"Use all tasks available from the global task pool, ignore filters below.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected bool m_bUseAllAvailableTasksFromPool;

	//! PR Task Spawner: Tasks - Global Task Pool
	[Attribute(desc: "Filter available tasks for the scenario. You can have more than 1 filter.  ", category: "PR Task Spawner: Tasks - Global Task Pool")]
	protected ref array<ref PR_TaskType> m_aTaskTypesFilter;

	//! PR Task Spawner: Tasks - Pick random tasks from all avaliable task list above.
	[Attribute("false", UIWidgets.CheckBox,"Pick random tasks from all avaliable tasks from list above.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseRandomTasks;

	//--- Amount of delay before spawning task
	[Attribute("1", desc: "Amount of random tasks to pick. 'Use Random Tasks' must be checked. -1 will use all avaliable tasks from list above.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iRandomTaskCount;

	//! PR Task Spawner: Tasks - Spawner - Use random delay timer: Uses min and max values below.
	[Attribute("false", UIWidgets.CheckBox,"Use random delay timer: Uses min and max values below.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseFirstTaskRandomDelayTimer;

	//--- Amount of delay before spawning task
	[Attribute("0", UIWidgets.EditBox, "Amount of delay before spawning first task. In seconds.  Minimum value if used with 'Use Random Delay Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerToSpawnFirstTaskMin;

	//! PR Task Spawner: Tasks - Spawner - Delay timer max value: Maximum amount of delay before spawning task if used with random timer. (seconds)
	[Attribute("0", UIWidgets.EditBox, "Maximum amount of delay before spawning task if used with 'Use Random Delay Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerToSpawnFirstTaskMax;

	//! PR Task Spawner: Tasks - Spawner - Use random delay between tasks timer: Uses min and max values below.
	[Attribute("false", UIWidgets.CheckBox,"Use random delay between tasks timer: Uses min and max values below.  ", category: "PR Task Spawner: Tasks - Spawner")]
	protected bool m_bUseRandomDelayBetweenTasksTimer;

	//! PR Task Spawner: Tasks - Spawner - Amount of delay between spawning each task. In seconds.  Minimum value if used with 'Use Random Delay Between Tasks Timer'. (seconds)
	[Attribute("0", UIWidgets.EditBox, "Amount of delay between spawning each task. In seconds.  Minimum value if used with 'Use Random Delay Between Tasks Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerBetweenEachTaskMin;

	//! PR Task Spawner: Tasks - Spawner - Delay timer max value: Maximum amount of delay before spawning task if used with 'Use Random Delay Between Tasks Timer'. (seconds)
	[Attribute("0", UIWidgets.EditBox, "Maximum amount of delay before spawning task if used with 'Use Random Delay Between Tasks Timer'. (seconds)  ", "0 inf 1", category: "PR Task Spawner: Tasks - Spawner")]
	protected int m_iDelayTimerBetweenEachTaskMax;

	protected ref array<PR_TASK_ESFTaskType> m_aESFTaskTypesAvailable = {};
	protected ref array<string> m_aTaskArrayFiltered = {};
	protected ref array<int> m_aTaskTypesAvailableArray = {};
	protected ref array<string> m_aTaskArrayGlobal = {}; //PR_TaskCollections.GetTaskArrayGlobal();
	protected ref array<string> m_aTaskCollectionsArray = {};

	//------------------------------------------------------------------------------------------------
	//! EOnInit
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (m_bIsTestingMode)
		{
			if (m_aIndividualTasks.Count() > 0)
			{
				foreach (PR_TaskDetails taskDetails : m_aIndividualTasks)
				{
					SetIndividualTasks(taskDetails, m_aIndividualTasks);
				}
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aIndividualTasksToSpawnOnActivation: %3", m_sLogMode, m_sTriggerName, m_aIndividualTasksToSpawnOnActivation), LogLevel.WARNING);
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_bMoveTaskDestinationArray: %3", m_sLogMode, m_sTriggerName, m_bMoveTaskDestinationArray), LogLevel.WARNING);
			}

			if (m_bUseTaskPool)
				GetTaskPool();
			else
				GetTaskIndividual();

			if (GetTaskArrayFiltered().Count() == 0)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : ScenarioFramework: No valid tasks to spawn! ", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
				return;
			}

			GetTasksFinal();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! OnActivate
	override protected event void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (!Replication.IsServer())
			return;

		if (m_bIsTriggerActivated)
			return;

		m_bIsTriggerActivated = true;

		//--- Delay to spawn first task
		int delay = m_iDelayTimerToSpawnFirstTaskMin * 1000;

		if (m_bUseFirstTaskRandomDelayTimer)
			delay = Math.RandomInt(m_iDelayTimerToSpawnFirstTaskMin * 1000, m_iDelayTimerToSpawnFirstTaskMax * 1000);

		//--- Individual task details
		if (m_aIndividualTasks.Count() > 0)
		{
			foreach (PR_TaskDetails taskDetails : m_aIndividualTasks)
			{
				SetIndividualTasks(taskDetails, m_aIndividualTasks);
			}
		}

		if (m_bUseTaskPool)
			GetTaskPool();
		else
			GetTaskIndividual();

		if (GetTaskArrayFiltered().Count() == 0)
		{
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : ScenarioFramework: No valid tasks to spawn! ", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
			return;
		}

		//--- End up with m_aTaskCollectionsArray
		GetTasksFinal();

		//--- Setup global tasks
		if (m_bUseTaskPool)
		{
			//--- Remove tasks from the global PR_TaskCollections array
			m_aTaskArrayGlobal = PR_TaskCollections.GetTaskArrayGlobal();

			foreach (int index, string task : m_aTaskCollectionsArray)
			{
				if (m_aTaskArrayGlobal.Find(task) > -1)
				{
					Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : Task has been removed from the PR_TaskCollections global array: %3", m_sLogMode, m_sTriggerName, task), LogLevel.ERROR);
					m_aTaskArrayGlobal.Remove(m_aTaskArrayGlobal.Find(task));
				}
			}

			PR_TaskCollections.SetTaskArrayGlobal(m_aTaskArrayGlobal);
		}

		//--- Get base info
		GetBaseInfo();

		foreach (string taskName : m_aTaskCollectionsArray)
		{
			int index = m_aIndividualTasksToSpawnOnActivation.Find(taskName);
			if (index == -1)
				continue;

			bool useMoveTaskDestination = m_bMoveTaskDestinationArray.Get(index);
			if (useMoveTaskDestination)
			{
				array<ref PR_MoveTask> taskMoveDetails = m_aTaskMoveDetailsArray.Get(index);
				if (taskMoveDetails.Count() == 0)
					continue;

				string taskSectionToMove;
				array<string> moveSectionTo = {};
				array<int> moveSectionToRandomBases = {};
				bool insertBaseNameInTaskInfos;

				foreach (PR_MoveTask details : taskMoveDetails)
				{
					taskSectionToMove = details.m_sTaskSectionToMove;
					moveSectionTo = details.m_sMoveSectionTo;
					moveSectionToRandomBases = details.m_sMoveSectionToRandomBases;
					insertBaseNameInTaskInfos = details.m_bInsertBaseNameInTaskInfos;

					if (!taskSectionToMove)
						continue;

					array<string> combinedArray = {};
					array<string> combinedBaseArray = {};
					string callSign;

					if (moveSectionTo.Count() > 0)
					{
						int moveSectionToCount = 0;
						moveSectionToCount = moveSectionTo.Count();
						IEntity holder;
						array<string> moveSectionToCountArray = {};
						int _i = 0;
						while (moveSectionToCount > _i)
						{
							holder = GetGame().GetWorld().FindEntityByName(moveSectionTo.Get(_i));
							if (holder)
								GetAllChildrenNames(holder, moveSectionToCountArray, m_bDebugLogs);

							_i++;
						}
						combinedArray = moveSectionToCountArray;
					}

					if (moveSectionToRandomBases.Count() > 0) //   GetBaseNames(), GetBaseVectors(), GetBaseIDs(), GetBaseCallsigns()
					{
						foreach (int baseType : moveSectionToRandomBases)
						{
							switch (baseType)
							{
								case 0: // "None"
									break;

								case 1: // "Main Base"
								{
									string hq = GetBaseNames().Get(0);
									if (hq)
										combinedBaseArray.Insert(hq);
									break;
								}
								case 2: // "Random Base CP - Friendly"
								{
									if (m_aFriendlyPoints.Count() > 0)
									{
										foreach (string base : m_aFriendlyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
								case 3: // "Random Base CP - Enemy"
								{
									if (m_aEnemyPoints.Count() > 0)
									{
										foreach (string base : m_aEnemyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
								case 4: // "Random Base CP"
								{
									if (m_aFriendlyPoints.Count() > 0)
									{
										foreach (string base : m_aFriendlyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									if (m_aEnemyPoints.Count() > 0)
									{
										foreach (string base : m_aEnemyPoints)
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
								case 5: // "Random All Base and Relays"
								{
									if (GetBaseNames().Count() > 0)
									{
										foreach (string base : GetBaseNames())
										{
											combinedBaseArray.Insert(base);
										}
									}
									break;
								}
							}
						}

						if (insertBaseNameInTaskInfos)
						{
							foreach (string baseName : combinedBaseArray)
							{
								Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : GetBaseNames().Find(baseName): %3", m_sLogMode, m_sTriggerName, GetBaseNames().Find(baseName)), LogLevel.WARNING);
								index = GetBaseNames().Find(baseName);
								if (!index == -1)
									continue;

								Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : taskName: %3", m_sLogMode, m_sTriggerName, taskName), LogLevel.WARNING);
								Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : GetBaseCallsigns(): %3", m_sLogMode, m_sTriggerName, GetBaseCallsigns()), LogLevel.WARNING);
								Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : GetBaseCallsigns().Get(index): %3", m_sLogMode, m_sTriggerName, GetBaseCallsigns().Get(index)), LogLevel.WARNING);

								UpdateTitles(taskName, index);
							}
						}
					}

					foreach (string baseName : combinedBaseArray)
					{
						combinedArray.Insert(baseName);
					}

					if (combinedArray.Count() == 0)
						return;

					int randomIndex = combinedArray.GetRandomIndex();
					string whereToMove = combinedArray.Get(randomIndex);

					TeleportObject(taskSectionToMove, whereToMove);
				}
			}
		}

		//--- Persistence cleanup
		PersistenceCleanup();

		GetGame().GetCallqueue().CallLater(SpawnObjects, delay, false, m_aTaskCollectionsArray, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION/*, delayBetween*/);

		Deactivate();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTitles(string taskName, int index)
	{
		IEntity layerTask = GetLayer(taskName);
		if (!layerTask)
			return;

		string hqCallsign = GetBaseCallsigns().Get(0);
		if (!hqCallsign)
			hqCallsign = "Unknown HQ";

		string callSign = GetBaseCallsigns().Get(index);
		if (!callSign)
			callSign = "Unknown Location";

		SCR_ScenarioFrameworkLayerBase layerComponent = SCR_ScenarioFrameworkLayerBase.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerBase));

		IEntity child = layerComponent.GetOwner().GetChildren();
		if (!child)
			return;

		while (child)
		{
			string title;
			string description;

			SCR_ScenarioFrameworkSlotPick slotPick = SCR_ScenarioFrameworkSlotPick.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotPick));
			if (slotPick)
			{
				title = slotPick.GetTaskTitle(0);
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : (UpdateTitles) slotPick title: %2", m_sLogMode, title), LogLevel.WARNING);
				title = string.Format(title, callSign, hqCallsign);
				description = slotPick.GetTaskTitle(1);
				description = string.Format(description, callSign, hqCallsign);
				string titleUpdate1 = slotPick.GetTaskTitle(2);
				titleUpdate1 = string.Format(titleUpdate1, callSign, hqCallsign);
				string descriptionUpdate1 = slotPick.GetTaskTitle(3);
				descriptionUpdate1 = string.Format(descriptionUpdate1, callSign, hqCallsign);
				string titleUpdate2 = slotPick.GetTaskTitle(4);
				titleUpdate2 = string.Format(titleUpdate2, callSign, hqCallsign);
				string descriptionUpdate2 = slotPick.GetTaskTitle(5);
				descriptionUpdate2 = string.Format(descriptionUpdate2, callSign, hqCallsign);

				slotPick.SetTitleAndDescriptions(title, description, titleUpdate1, descriptionUpdate1, titleUpdate2, descriptionUpdate2);
			} else
			{
				SCR_ScenarioFrameworkSlotTask slotBase = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
				if (slotBase)
				{
					title = slotBase.GetTaskTitle(0);
					Print(string.Format("[PR_SpawnTaskTrigger] %1 : (UpdateTitles) title: %2", m_sLogMode, title), LogLevel.WARNING);
					title = string.Format(title, callSign, hqCallsign);
					Print(string.Format("[PR_SpawnTaskTrigger] %1 : (UpdateTitles) title: %2", m_sLogMode, title), LogLevel.WARNING); // remove after testing
					description = slotBase.GetTaskDescription(0);
					description = string.Format(description, callSign, hqCallsign);

					SCR_ScenarioFrameworkSlotClearArea slotClearArea = SCR_ScenarioFrameworkSlotClearArea.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotClearArea));
					if (slotClearArea)
						slotClearArea.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotDefend slotDefend = SCR_ScenarioFrameworkSlotDefend.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotDefend));
					if (slotDefend)
						slotDefend.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotExtraction slotExtraction = SCR_ScenarioFrameworkSlotExtraction.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotExtraction));
					if (slotExtraction)
						slotExtraction.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotDestroy slotDestroy = SCR_ScenarioFrameworkSlotDestroy.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotDestroy));
					if (slotDestroy)
						slotDestroy.SetTitleAndDescription(title, description);

					SCR_ScenarioFrameworkSlotKill slotKill = SCR_ScenarioFrameworkSlotKill.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotKill));
					if (slotKill)
						slotKill.SetTitleAndDescription(title, description);
				}
			}

			child = child.GetSibling();
		}
	}

	protected ref array<string> m_aIndividualTasksToSpawnOnActivation = {};
	protected ref array<bool> m_bMoveTaskDestinationArray = {};
	protected ref array<array<ref PR_MoveTask>> m_aTaskMoveDetailsArray = {};
	protected ref array<string> m_aNameOfTasksToSpawnOnActivation = m_aIndividualTasksToSpawnOnActivation;

	//------------------------------------------------------------------------------------------------
	//!
	protected void SetIndividualTasks(PR_TaskDetails taskDetails, array<ref PR_TaskDetails> individualTasks)
	{
		string taskName = taskDetails.m_sTaskName;
		m_aIndividualTasksToSpawnOnActivation.Insert(taskName);

		bool useMoveTaskDestination = taskDetails.m_bUseMoveTaskDestination; // get bool later
		m_bMoveTaskDestinationArray.Insert(useMoveTaskDestination);

		array<ref PR_MoveTask> taskMoveDetails = taskDetails.m_aTaskMoveDetails;
		m_aTaskMoveDetailsArray.Insert(taskMoveDetails);
	}

	//------------------------------------------------------------------------------------------------
	//! Tasks from Individual Entries
	protected void GetTaskIndividual()
	{
		if (m_aNameOfTasksToSpawnOnActivation.Count() > 0)
			GetLayerTask(m_aNameOfTasksToSpawnOnActivation, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
	}

	//------------------------------------------------------------------------------------------------
	//! Tasks from global pool
	protected void GetTaskPool()
	{
		m_aTaskArrayGlobal = PR_TaskCollections.GetTaskArrayGlobal();

		//Fetching available Task Types for generation based on type
		foreach (PR_TaskType taskTypeClass : m_aTaskTypesFilter)
		{
			m_aESFTaskTypesAvailable.Insert(taskTypeClass.GetTaskType());
		}

		//--- Gather available task types
		if (m_bUseAllAvailableTasksFromPool)
		{
			m_aTaskTypesAvailableArray = {1, 4, 5, 9, 10, 11, 12};
		} else
		if (m_aESFTaskTypesAvailable.Find(0) == 0)
		{
			m_aTaskTypesAvailableArray = {};
		} else
		{
			m_aTaskTypesAvailableArray = m_aESFTaskTypesAvailable;
		}

		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : m_aTaskArrayGlobal)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aTaskArrayGlobal: [ %3 ] : %3", m_sLogMode, m_sTriggerName, index, x), LogLevel.NORMAL);
			}
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aESFTaskTypesAvailable: [ %3 ]", m_sLogMode, m_sTriggerName, m_aESFTaskTypesAvailable), LogLevel.NORMAL);
		}

		if (m_aTaskArrayGlobal.Count() > 0)
			GetLayerTask(m_aTaskArrayGlobal, m_aTaskTypesAvailableArray);

		SetNameOfTasksToSpawnOnActivation(GetTaskArrayFiltered());
		if (m_bDebugLogs)
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aNameOfTasksToSpawnOnActivation: %3", m_sLogMode, m_sTriggerName, GetTaskArrayFiltered()), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Packs tasks into a final array
	protected void GetTasksFinal()
	{
		if (m_bUseRandomTasks)
		{
			array<string> randomArray = m_aNameOfTasksToSpawnOnActivation;

			if (m_iRandomTaskCount > randomArray.Count())
				m_iRandomTaskCount = randomArray.Count();

			if (m_iRandomTaskCount == -1)
				m_iRandomTaskCount = randomArray.Count();

			array<string> tempArray = {};
			for (int i; i < m_iRandomTaskCount; i++)
			{
				int randomIndex = randomArray.GetRandomIndex();
				string taskName = randomArray.Get(randomIndex);
				tempArray.Insert(taskName);
				randomArray.Remove(randomIndex);
			}
			m_aTaskCollectionsArray = tempArray;
		} else
			m_aTaskCollectionsArray = m_aNameOfTasksToSpawnOnActivation;

		if (m_bDebugLogs)
		{
			foreach (int index, string x : m_aTaskCollectionsArray)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : m_aTaskCollectionsArray: [ %3 ] %4", m_sLogMode, m_sTriggerName, index, x), LogLevel.NORMAL);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseNames;
	protected ref array<string> m_aBaseNames = {};
	protected void SetBaseNames(string name)
	{
		if (!name)
			name = "";
		m_aBaseNames.Insert(name);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseNames;
	array<string> GetBaseNames()
	{
		return m_aBaseNames;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseVectors
	protected ref array<vector> m_aBaseVectors = {};
	protected void SetBaseVectors(vector location)
	{
		if (!location)
			location = "0 0 0";
		m_aBaseVectors.Insert(location);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseVectors;
	array<vector> GetBaseVectors()
	{
		return m_aBaseVectors;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseIDs
	protected ref array<int> m_aBaseIDs = {};
	protected void SetBaseIDs(int baseID)
	{
		if (!baseID)
			baseID = -1;
		m_aBaseIDs.Insert(baseID);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseIDs;
	array<int> GetBaseIDs()
	{
		return m_aBaseIDs;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aBaseCallsigns
	protected ref array<string> m_aBaseCallsigns = {};
	protected void SetBaseCallsigns(string callsign)
	{
		if (!callsign)
			callsign = "";
		m_aBaseCallsigns.Insert(callsign);
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aBaseCallsigns;
	array<string> GetBaseCallsigns()
	{
		return m_aBaseCallsigns;
	}

	protected ref array<string> m_aControlPoints = {};
	protected ref array<string> m_aFriendlyPoints = {};
	protected ref array<string> m_aEnemyPoints = {};

	//------------------------------------------------------------------------------------------------
	//! Get all base information
	protected void GetBaseInfo()
	{
		//--- Get base info
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			m_OwnerFaction = factionManager.GetFactionByKey(m_OwnerFactionKey);
			if (m_OwnerFaction)
			{
				SCR_CampaignFaction mainBase = SCR_CampaignFaction.Cast(m_OwnerFaction.(SCR_CampaignFaction.GetMainBase()));
				if (mainBase)
				{
					SCR_Faction faction;
					SCR_MilitaryBaseCallsign callsignInfo;
					string callsignNameOnly;
					SCR_CampaignMilitaryBaseComponent hq;
					hq = mainBase.GetMainBase();
					IEntity owner;
					string name;

					if (hq)
					{

						faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_OwnerFactionKey));
						owner = hq.GetOwner();
						name = owner.GetName();
						if (!name)
							name = GetRandomName("HQ_");

						SetBaseNames(name);
						SetBaseVectors(owner.GetOrigin());
						SetBaseIDs(hq.GetCallsign()); // index
						callsignInfo = faction.GetBaseCallsignByIndex(hq.GetCallsign());
						SetBaseCallsigns(callsignInfo.GetCallsignShort());

						SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
						if (!baseManager)
							return;

						array<SCR_MilitaryBaseComponent> bases = {};
						baseManager.GetBases(bases);

						foreach (SCR_MilitaryBaseComponent base : bases)
						{
							SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
							if (!campaignBase)
								continue;

							owner = base.GetOwner();
							name = owner.GetName();
							if (!name)
								name = GetRandomName("Base_");

							if (GetBaseNames().Find(name) == -1)
							{
								SetBaseNames(name);
								SetBaseVectors(owner.GetOrigin());
								SetBaseIDs(base.GetCallsign());
								callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign());
								SetBaseCallsigns(callsignInfo.GetCallsignShort());
							}

							//--- Gather all control points
							if (campaignBase.IsControlPoint())
							{
								m_aControlPoints.Insert(name);

								//--- Filter friendly vs enemy/open points
								Faction baseFaction = base.GetFaction();

								if (baseFaction == m_OwnerFaction)
									m_aFriendlyPoints.Insert(name);
								else
									m_aEnemyPoints.Insert(name);
							}
						}

						if (m_bDebugLogs)
						{
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : GetBaseNames(): %2", m_sLogMode, GetBaseNames()), LogLevel.NORMAL);
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : ControlPoints: %2", m_sLogMode, m_aControlPoints), LogLevel.NORMAL);
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : FriendlyPoints: %2", m_sLogMode, m_aFriendlyPoints), LogLevel.NORMAL);
							Print(string.Format("[PR_SpawnTaskTrigger] %1 : EnemyPoints: %2", m_sLogMode, m_aEnemyPoints), LogLevel.NORMAL);
						}
					} else
						Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No hq!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
				} else
					Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No mainBase!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
			} else
				Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No m_OwnerFaction!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
		} else
			Print(string.Format("[PR_SpawnTaskTrigger] %1 : Trigger: %2 : No factionManager!", m_sLogMode, m_sTriggerName), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aTaskArrayFiltered
	protected void SetTaskArrayFiltered(array<string> taskArrayFiltered)
	{
		m_aTaskArrayFiltered = taskArrayFiltered;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aTaskArrayFiltered
	protected array<string> GetTaskArrayFiltered()
	{
		return m_aTaskArrayFiltered;
	}

	//------------------------------------------------------------------------------------------------
	//! sets m_aNameOfTasksToSpawnOnActivation
	protected void SetNameOfTasksToSpawnOnActivation(array<string> nameOfTasksToSpawnOnActivation)
	{
		if (m_bDebugLogs)
			Print(string.Format("[PR_SpawnTaskTrigger] (SetNameOfTasksToSpawnOnActivation): Trigger: %1 : nameOfTasksToSpawnOnActivation: %2", m_sTriggerName, nameOfTasksToSpawnOnActivation), LogLevel.NORMAL);

		m_aNameOfTasksToSpawnOnActivation = nameOfTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	//! returns m_aNameOfTasksToSpawnOnActivation
	protected array<string> GetNameOfTasksToSpawnOnActivation()
	{
		return m_aNameOfTasksToSpawnOnActivation;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn tasks from array
	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType/*, int delayBetween*/)
	{
		int nameCount = aObjectsNames.Count(); // gets elements count of array "aObjectsNames"
		int sleep = 0;

		for (int i; i < nameCount; i++)
		{
			string sTaskName = aObjectsNames.Get(i);
			IEntity object = GetGame().GetWorld().FindEntityByName(sTaskName);
			if (m_bDebugLogs)
				Print(string.Format("[PR_SpawnTaskTrigger] (SpawnObjects): Trigger: %1 : Task Index: [ %2 ]. Task Name: %3", m_sTriggerName, i, sTaskName), LogLevel.NORMAL);

			if (!object)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (SpawnObjects): %1 : Trigger: %2 : ScenarioFramework: Can't spawn object set in slot (%3). Slot doesn't exist", m_sLogMode, m_sTriggerName, aObjectsNames.Get(i)), LogLevel.ERROR);
				continue;
			}

			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (SpawnObjects) %1 : Trigger: %2 : ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", m_sLogMode, m_sTriggerName), LogLevel.ERROR);
				continue;
			}

			//--- Complete, now spawn it in mission
			GetGame().GetCallqueue().CallLater(layer.Init, sleep, false, null, eActivationType);

			//--- Random delay between tasks
			int delayBetween = m_iDelayTimerBetweenEachTaskMin * 1000;

			if (m_bUseRandomDelayBetweenTasksTimer)
				delayBetween = Math.RandomInt(m_iDelayTimerBetweenEachTaskMin * 1000, m_iDelayTimerBetweenEachTaskMax * 1000);

			sleep = sleep + delayBetween;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get task layer
	protected IEntity GetLayer(string taskName)
	{
		IEntity layerTask = GetWorld().FindEntityByName(taskName);

		if (!layerTask)
		{
			Print(string.Format("[PR_SpawnTaskTrigger] (GetLayer): Trigger: %1 : No valid layer! Check 'LayerTask' taskName is correct. : %2", m_sTriggerName, taskName), LogLevel.ERROR);
			return null;
		}

		SCR_ScenarioFrameworkLayerBase selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(layerTask.FindComponent(SCR_ScenarioFrameworkLayerBase));

		if (selectedLayer)
		{
			SCR_ScenarioFrameworkArea area = selectedLayer.GetParentArea();
			if (!area)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (GetLayer): Trigger: %1 : No valid area! Check 'LayerTask' is inside a area. : %2", m_sTriggerName, taskName), LogLevel.ERROR);
				return null;
			}
		} else
		{
			Print(string.Format("[PR_SpawnTaskTrigger] (GetLayer): Trigger: %1 : No valid layer! Check 'LayerTask' is of proper base. : %2", m_sTriggerName, taskName), LogLevel.ERROR);
			return null;
		}

		return layerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieve tasks available for spawning, checks for valid layer against taskName
	protected void GetLayerTask(notnull array<string> taskArrayGlobal, array<int> taskTypesAvailable)
	{
		array<string> taskArrayFiltered = {};

		foreach (string taskName : taskArrayGlobal)
		{
			IEntity layerTask = GetLayer(taskName);

			if (!layerTask)
				continue;

			EntityPrefabData prefabData = layerTask.GetPrefabData();
			if (prefabData)
			{
				//--- GetAncestor because prefab drops off the hash when using ON_TRIGGER_ACTIVATION, which is what I want to use for this trigger
				//ResourceName prefabName = prefabData.GetPrefab().GetName();
				ResourceName prefabName = prefabData.GetPrefab().GetAncestor().GetName();

				if (m_bDebugLogs && m_bIsTestingMode)
					Print(string.Format("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: %1 : taskName: %2 : prefabName: %3", m_sTriggerName, taskName, prefabName), LogLevel.NORMAL);

				if (taskArrayFiltered.Find(taskName) == -1)
				{
					foreach (int type : taskTypesAvailable)
					{
						switch (type)
						{
							case 1: // DELIVER
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDeliver.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 2: // DELIVER_INTEL
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 3: // DELIVER_VEHICLE
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 4: // DESTROY
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDestroy.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 5: // DEFEND
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskDefend.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 6: // DEFEND_AREA
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 7: // DEFEND_AREA_AND_TARGET
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 8: // DEFEND_TARGET
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 9: // KILL & TaskKill
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskKill.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskKill.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 10: // CLEAR_AREA & TaskClearArea
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskClearArea.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 11: // MOVE
							{
								if (prefabName == "Prefabs/ScenarioFramework/Components/LayerTaskMove.et"
								|| prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskMove.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
							case 12: // EXFIL
							{
								if (prefabName == "Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskExfil.et")
									taskArrayFiltered.Insert(taskName);
								break;
							};
						}
					}
				}
			} else
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: %1 : No prefabData! Make sure layer Activation type is set to 'ON_TRIGGER_ACTIVATION' : [ %2 ]", m_sTriggerName, taskName), LogLevel.WARNING);
				continue;
			}
		}

		SetTaskArrayFiltered(taskArrayFiltered);
		if (m_bDebugLogs && m_bIsTestingMode)
		{
			foreach (int index, string x : taskArrayFiltered)
			{
				Print(string.Format("[PR_SpawnTaskTrigger] (GetLayerTask): Trigger: %1 : taskArrayFiltered: [ %2 ] : %3", m_sTriggerName, index, x), LogLevel.NORMAL);
			}
		}
	}
}

// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class PR_TaskType
{
	[Attribute("Type of task", UIWidgets.ComboBox,"Tasks of this type will be selected from the task pool.  ", enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskType))]
	protected PR_TASK_ESFTaskType m_TypeOfTask;

	//------------------------------------------------------------------------------------------------
	//! \return
	PR_TASK_ESFTaskType GetTaskType()
	{
		return m_TypeOfTask;
	}
}

enum PR_TASK_ESFTaskType
{
	"None" = 0, 					// 0
	"Deliver - All" = 1, 			// 1		{88821DCA414AF4C7}Prefabs/ScenarioFramework/Components/LayerTaskDeliver.et
	"Deliver - Intel" = 2, 			// 2		{31180485D450A1A1}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverIntel.et
	"Deliver - Vehicle" = 3, 		// 3		{BBB4E7BB4416F6B3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDeliverVehicles.et
	"Destroy" = 4, 					// 4		{5EDF39860639027D}Prefabs/ScenarioFramework/Components/LayerTaskDestroy.et || {265A8A1492CB6189}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDestroy.et
	"Defend - All" = 5, 			// 5		{775C493CE872C3A5}Prefabs/ScenarioFramework/Components/LayerTaskDefend.et
	"Defend - Area" = 6, 			// 6		{2B0E0A06A4475EA3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendArea.et
	"Defend - Area and Target" = 7,// 7		{18B9A717BAE9FF57}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendAreaAndTarget.et
	"Defend - Target" = 8, 			// 8		{A651662FD0667288}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskDefendTarget.et
	"Kill" = 9, 					// 9		{2008B4EE6C4D528E}Prefabs/ScenarioFramework/Components/LayerTaskKill.et || {B506343A3BF60DB3}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskKill.et
	"Clear Area" = 10, 				// 10	{CDC0845AD90BA073}Prefabs/ScenarioFramework/Components/LayerTaskClearArea.et || {C248387C4E5A9DE8}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskClearArea.et
	"Move" = 11, 					// 11	{246BEC080F393398}Prefabs/ScenarioFramework/Components/LayerTaskMove.et || {3512D2F2BF47D345}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskMove.et
	"Exfil" = 12, 					// 12	{172146470FF780EB}Prefabs/ScenarioFramework/Compositions/LayerTasks/TaskExfil.et
}

[BaseContainerProps()]
class PR_TaskDetails
{
	//! PR Task Spawner: Tasks - Individual Tasks - Name of task to spawn.
	[Attribute(desc: "Name of task to spawn.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	string m_sTaskName;

	//! PR Task Spawner: Tasks - Individual Tasks - Allow moving of Area, task layer, or slots to another destination.
	[Attribute("false", UIWidgets.CheckBox,"Allow moving of Area, task layer, or slots to another destination.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bUseMoveTaskDestination;

	//! PR Task Spawner: Tasks - Individual Tasks - If you desire the Area, task layer, or slots to be moved to another location, add details here.
	[Attribute(desc: "If you desire the Area, task layer, or slots to be moved to another location, add details here.  ", category: "PR Task Spawner: Tasks - Individual Tasks")]
	ref array<ref PR_MoveTask> m_aTaskMoveDetails;
}

[BaseContainerProps()]
class PR_MoveTask
{
	//! PR Task Spawner: Tasks - Individual Tasks
	//! Task section name. Area, task layer, or slot.
	//! i.e. Area_01, TaskMove_1, SlotPick2.
	//! Area: Moves area and all children.
	//! Task layer: Moves task layer and all children.
	//! Slot: Moves slot and Children.
	static string hintTaskSectionToMove = (
	"Task section name. Area, task layer, or slot.  " + "<br />" +
	"i.e. Area_01, TaskMove_1, SlotPick2.  " + "<br />" + "<br />" +
	"Area: Moves area and all children.  " + "<br />" +
	"Task layer: Moves task layer and all children.  " + "<br />" +
	"Slot: Moves slot and Children.  " + "<br />" +
	"----------------------------------------------------------------------" + "<br />" +
	"Requires: 'Move Section To' or 'Move Section To Random'"
	);
	[Attribute(desc: hintTaskSectionToMove)]
	string m_sTaskSectionToMove;

	//! PR Task Spawner: Tasks - Individual Tasks
	//! Object name to move task section to.
	//! More than one will pick a random location.
	//! This can combine with 'Move Section To Random'.
	static string hintMoveSectionTo = (
	"Object name to move task section to.  " + "<br />" +
	"-----------------------------------------------------------------------------------" + "<br />" +
	"More than one will pick a random location.  " + "<br />" +
	"Locations can grouped to make a collection, i.e. inside genericEnity.  " + "<br />" +
	"Be sure they have hierarchy added to them.  " + "<br />" + "<br />" +
	"This can combine with 'Move Section To Random Bases'.  "
	);
	[Attribute(desc: hintMoveSectionTo)]
	ref array<string> m_sMoveSectionTo;

	//! PR Task Spawner: Tasks - Individual Tasks
	//! Random locations to move task section to.
	//! This can combine with 'Move Section To'.
	static string hintMoveSectionToRandom = (
	"Random locations to move task section to.  " + "<br />" +
	"----------------------------------------------------" + "<br />" +
	"This can combine with 'Move Section To'.  "
	);
	[Attribute("", UIWidgets.ComboBox, desc: hintMoveSectionToRandom, enums: ParamEnumArray.FromEnum(PR_TASK_ESFTaskMove))]
	ref array<int> m_sMoveSectionToRandomBases;

	static string hintInsertBaseNameInTaskInfos = (
	"Inserts base callsign name into the task slot titles where '%1' is used.  " + "<br />" +
	"'%2' will always use main base callsign.  " + "<br />" +
	"i.e. 'Move to Point %2' will display 'Move to Point Alabama'.  " + "<br />" +
	"----------------------------------------------------------------------------------------" + "<br />" +
	"Only works in combination with 'Move Section To Random Bases' above.  "
	);
	//! PR Task Spawner: Tasks - Individual Tasks - Adds base callsign name in the task titles where '%1' is used.
	//! "'%2' will always use main base callsign.  "
	//! "I.E. 'Move to Point %2' will display 'Move to Point Alabama'.  "
	//! "Only works in combination with 'Move Section To Random Bases' above.  "
	[Attribute("false", UIWidgets.CheckBox, hintInsertBaseNameInTaskInfos, category: "PR Task Spawner: Tasks - Individual Tasks")]
	bool m_bInsertBaseNameInTaskInfos;
}

enum PR_TASK_ESFTaskMove
{
	"None" = 0,						// 0
	"Main Base" = 1,					// 1
	"Random Base CP - Friendly" = 2,	// 2
	"Random Base CP - Enemy" = 3,	// 3
	"Random Base CP" = 4,			// 4
	"Random All Base and Relays" = 5,// 5
}

modded class SCR_ScenarioFrameworkSlotPick
{
	void SetTitleAndDescriptions(string taskTitle, string taskDescription, string titleUpdate1, string descriptionUpdate1, string titleUpdate2, string descriptionUpdate2)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
		m_sTaskTitleUpdated1 = titleUpdate1;
		m_sTaskDescriptionUpdated1 = descriptionUpdate1;
		m_sTaskTitleUpdated2 = titleUpdate2;
		m_sTaskDescriptionUpdated2 = descriptionUpdate2;
	}

	override string GetTaskTitle(int iState = 0)
	{
		if (iState == 0)
			return super.GetTaskTitle();
		else if (iState == 1)
			return m_sTaskTitleUpdated1;
		else if (iState == 2)
			return m_sTaskTitleUpdated2;
		else if (iState == 4)
			return super.GetTaskTitle();
		else if (iState == 5)
			return m_sTaskTitleUpdated1;
		else if (iState == 6)
			return m_sTaskTitleUpdated2;

		return string.Empty;
	}
}

modded class SCR_ScenarioFrameworkSlotClearArea
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotDefend
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotExtraction
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotKill
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}

modded class SCR_ScenarioFrameworkSlotDestroy
{
	void SetTitleAndDescription(string taskTitle, string taskDescription)
	{
		m_sTaskTitle = taskTitle;
		m_sTaskDescription = taskDescription;
	}
}