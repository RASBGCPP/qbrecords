#include "stdafx.h"
#include "SignalWait.h"
SignalWait::SignalWait() :
    mSignal(false)
{
}

// Set and reset the condition
void SignalWait::Set(bool in_state)
{
    {
        std::lock_guard<std::mutex> guard(mMutex);
        mSignal = in_state;
    }
    if (in_state) mConditionVar.notify_one();
}

// Check condition without waiting
bool SignalWait::IsTrue(void)
{
    std::lock_guard<std::mutex> guard(mMutex);
    return mSignal;
}

// Wait until condition becomes true
// If the condition has already been set the function
// will immediately return.
void SignalWait::WaitUntilTrue(void)
{
    std::unique_lock<std::mutex> uLock(mMutex);
    mConditionVar.wait(uLock, [&] {return mSignal; });
    mSignal = false;//reset signal
}