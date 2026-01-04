#pragma once

#include "ParaNdis-Util.h"
#include "Parandis_DesignPatterns.h"

enum class SMNotifications {
    Started,
    Stopped,
    SupriseRemoved,
    NeedsReset,
    PoweredOn,
    PoweringOff
};

class CFlowStateMachine : public CPlacementAllocatable
{
public:
	CFlowStateMachine()
	{
		m_Counter.SetMask(StoppedMask);
		m_State = FlowState::Stopped;
		m_StopReason = NDIS_STATUS_PAUSED;
		m_SurpriseRemoved = false;
	}

    virtual void Start()
    {
        if (m_State != FlowState::Stopped)
        {
            return;
        }
        m_Counter.AddRef();
        m_State = FlowState::Running;
        m_Counter.ClearMask(StoppedMask);
    }

    virtual void Stop(NDIS_STATUS Reason = NDIS_STATUS_PAUSED)
    {
        if (m_State != FlowState::Running)
        {
            return;
        }
        m_State = FlowState::Stopping;
        m_StopReason = Reason;
        m_NoOutstandingItems.Clear();
        m_Counter.SetMask(StoppedMask);
        UnregisterOutstandingItem();
        m_NoOutstandingItems.Wait();
    }

    virtual void SupriseRemove()
    {
        m_SurpriseRemoved = true;
    }

    virtual bool RegisterOutstandingItems(ULONG NumItems,
        NDIS_STATUS *FailureReason = nullptr)
    {
        auto value = m_Counter.AddRef(NumItems);
        if (value & StoppedMask)
        {
            value = m_Counter.Release(NumItems);
            if (value == StoppedMask)
            {
                CompleteStopping();
            }
            if (FailureReason != nullptr)
            {
                *FailureReason = m_StopReason;
            }
            return false;
        }
        return true;
    }

    virtual void UnregisterOutstandingItems(ULONG NumItems)
    {
        NETKVM_ASSERT(m_State != FlowState::Stopped);
        LONG value = m_Counter.Release(NumItems);
        CheckCompletion(value);
    }

    virtual void CheckCompletion(LONG Value)
    {
        if (Value == StoppedMask)
        {
            CompleteStopping();
        }
        else if (Value)
        {
            // common case, data transfer (StoppedMask not set)
            // pausing or completing not last packet during pausing (StoppedMask set)
        }
        else
        {
            // illegal case
            NETKVM_ASSERT(Value != 0);
        }
    }

    virtual bool RegisterOutstandingItem()
    {
        return RegisterOutstandingItems(1);
    }

    virtual void UnregisterOutstandingItem()
    {
        UnregisterOutstandingItems(1);
    }

	~CFlowStateMachine() {};
    CFlowStateMachine(const CFlowStateMachine&);
    CFlowStateMachine& operator= (const CFlowStateMachine&);

protected:
    virtual void CompleteStopping()
    {
#ifndef _WIN32_WCE
		TPassiveSpinLocker lock(m_CompleteStoppingLock);
#endif

        if (m_State == FlowState::Stopping)
        {
            m_State = FlowState::Stopped;
            m_NoOutstandingItems.Notify();
        }
    }

    enum { StoppedMask = 0x40000000 };

    enum class FlowState
    {
        Running,
        Stopping,
        Stopped
    };

    CNdisRefCounter m_Counter;
    FlowState m_State;
    CNdisSpinLock m_CompleteStoppingLock;
    CNdisEvent m_NoOutstandingItems;
    NDIS_STATUS m_StopReason;
    bool m_SurpriseRemoved;
};

class CDataFlowStateMachine : public CFlowStateMachine
{
public:


    CDataFlowStateMachine() { }
	~CDataFlowStateMachine() { };
    CDataFlowStateMachine(const CDataFlowStateMachine&);
    CDataFlowStateMachine& operator= (const CDataFlowStateMachine&);

private:
	DECLARE_CNDISLIST_ENTRY(CDataFlowStateMachine);
};

class CConfigFlowStateMachine : public CFlowStateMachine
{
public:

    void Stop(NDIS_STATUS Reason = NDIS_STATUS_PAUSED) override
    {
        bool started = m_State != FlowState::Stopped;
        m_State = FlowState::Stopping;
        m_StopReason = Reason;
        m_NoOutstandingItems.Clear();
        m_Counter.SetMask(StoppedMask);
        if (started)
        {
            UnregisterOutstandingItem();
        }
        else
        {
            CheckCompletion(m_Counter);
        }
        m_NoOutstandingItems.Wait();
    }

    CConfigFlowStateMachine() { }
	~CConfigFlowStateMachine() { };
    CConfigFlowStateMachine(const CConfigFlowStateMachine&);
    CConfigFlowStateMachine& operator= (const CConfigFlowStateMachine&);

private:
	DECLARE_CNDISLIST_ENTRY(CConfigFlowStateMachine);
};

class CFlowStateMachineWithPointer : public CFlowStateMachine
{
public:
    CFlowStateMachineWithPointer() : m_Value(NULL) { }
    void Clear()
    {
        Stop();
        m_Value = NULL;
    }
    void Set(PVOID Value)
    {
        m_Value = Value;
        Start();
    }
    PVOID Reference()
    {
        if (!RegisterOutstandingItem())
            return NULL;
        if (!m_Value)
        {
            UnregisterOutstandingItem();
            return NULL;
        }
        return m_Value;
    }
    void Dereference()
    {
        UnregisterOutstandingItem();
    }
private:
    PVOID m_Value;
};

class CBindingToSriov : public CFlowStateMachineWithPointer, public CObserver<SMNotifications>
{
public:
	CBindingToSriov()
	{ 
		m_Context = NULL;
	}
	
	void SetAdapterContext(PVOID Context)
    {
        m_Context = Context;
    }
private:
    void Notify(SMNotifications message) override;
    PVOID m_Context;
};

class CMiniportStateMachine : public CPlacementAllocatable, public CObservee<SMNotifications>
{
public:
		CMiniportStateMachine()
		{ m_State = MiniportState::Halted; }

        void RegisterFlow(CDataFlowStateMachine &Flow)
        { m_DataFlows.PushBack(&Flow); }

        void UnregisterFlow(CDataFlowStateMachine &Flow)
        { m_DataFlows.Remove(&Flow); }

        void RegisterFlow(CConfigFlowStateMachine &Flow)
        { m_ConfigFlows.PushBack(&Flow); }

        void UnregisterFlow(CConfigFlowStateMachine &Flow)
        { m_ConfigFlows.Remove(&Flow); }

        void NotifyInitialized(PVOID AdapterContext)
        {
            StartConfigFlows();
            ChangeState(MiniportState::Paused, MiniportState::Halted);
            m_BindingToSriov.SetAdapterContext(AdapterContext);
            Add(&m_BindingToSriov);
        }

        void NotifyShutdown()
        { ChangeState(MiniportState::Shutdown,
                      MiniportState::Paused); }

        void NotifyRestarted()
        {
            StartFlows();
            ChangeState(MiniportState::Running, MiniportState::Paused);
        }

        void NotifyPaused()
        {
            StopFlows(NDIS_STATUS_PAUSED);
            ChangeState(MiniportState::Paused,
                MiniportState::Running);
        }

        void NotifyPowerOn()
        {
            UpdateFlowsOnPowerOn();
        }

        void NotifyResumed()
        {
            if (IsInState(MiniportState::FastSuspend))
            {
                StartFlows();
                ChangeState(MiniportState::Running, MiniportState::FastSuspend);
            }
            else
            {
                ChangeState(MiniportState::Paused, MiniportState::Suspended);
            }

        }

        void NotifySupriseRemoved()
        {
            UpdateFlowsOnSurpriseRemove();
            ChangeState(MiniportState::SurpriseRemoved,
            MiniportState::Running);
        }

        void NotifyDeviceNeedsReset()
        {
            UpdateFlowsOnNeedsReset();
            ChangeState(MiniportState::NeedsReset,
                MiniportState::Running);
        }

        void NotifySuspended()
        {
            if (IsInState(MiniportState::Running))
            {
                StopFlows(NDIS_STATUS_LOW_POWER_STATE);
                ChangeState(MiniportState::FastSuspend, MiniportState::Running);
            }
            else
            {
                ChangeState(MiniportState::Suspended, MiniportState::Paused);
            }
            UpdateFlowsOnEvent(SMNotifications::PoweringOff);
        }

        void NotifyHalted()
        {
            StopConfigFlows(NDIS_STATUS_PAUSED);
        }

        void NotifyBindSriov(PVOID Value)
        {
            m_BindingToSriov.Set(Value);
        }

        void NotifyUnbindSriov()
        {
            m_BindingToSriov.Clear();
        }

        PVOID ReferenceSriovBinding()
        {
            return m_BindingToSriov.Reference();
        }

        void DereferenceSriovBinding()
        {
            m_BindingToSriov.Dereference();
        }

		~CMiniportStateMachine() {};
        CMiniportStateMachine(const CMiniportStateMachine&);
        CMiniportStateMachine& operator= (const CMiniportStateMachine&);

private:
    enum class MiniportState
    {
        Halted,
        Running,
        Paused,
        Shutdown,
        Suspended,
        FastSuspend,
        SurpriseRemoved,
        NeedsReset
    };

    bool IsInState(MiniportState State) const
    { return m_State == State; }

    template <typename Args>
    bool IsInState(MiniportState State, Args MoreStates) const
    { return IsInState(State) || IsInState(MoreStates); }

    template <typename Args>
    void ChangeState(MiniportState NewState,
                     Args
#ifdef DBG
                     AllowedStates
#endif
                    )
    {
        NETKVM_ASSERT(IsInState(AllowedStates...));
        m_State = NewState;
    }

    void StartFlows()
    { m_DataFlows.ForEach([](CDataFlowStateMachine* Flow) { Flow->Start(); }); }

    void StopFlows(NDIS_STATUS Reason)
    { m_DataFlows.ForEach([Reason](CDataFlowStateMachine* Flow) { Flow->Stop(Reason); }); }

    void StartConfigFlows()
    { m_ConfigFlows.ForEach([](CConfigFlowStateMachine* Flow) { Flow->Start(); }); }

    void StopConfigFlows(NDIS_STATUS Reason)
    { m_ConfigFlows.ForEach([Reason](CConfigFlowStateMachine* Flow) { Flow->Stop(Reason); }); }

    void UpdateFlowsOnSurpriseRemove()
    {
        SMNotifications msg = SMNotifications::SupriseRemoved;
        m_DataFlows.ForEach([](CDataFlowStateMachine* Flow) { Flow->SupriseRemove(); });
        m_ConfigFlows.ForEach([](CConfigFlowStateMachine* Flow) { Flow->SupriseRemove(); });
        UpdateFlowsOnEvent(msg);
    }

    void UpdateFlowsOnNeedsReset()
    {
        SMNotifications msg = SMNotifications::NeedsReset;
        UpdateFlowsOnEvent(msg);
    }

    void UpdateFlowsOnPowerOn()
    {
        SMNotifications msg = SMNotifications::PoweredOn;
        UpdateFlowsOnEvent(msg);
    }

    void UpdateFlowsOnEvent(SMNotifications msg)
    {
        NotifyAll(msg);
    }

    MiniportState m_State;
    CNdisList<CDataFlowStateMachine, CRawAccess, CNonCountingObject> m_DataFlows;
    CNdisList<CConfigFlowStateMachine, CRawAccess, CNonCountingObject> m_ConfigFlows;
    CBindingToSriov m_BindingToSriov;
};
