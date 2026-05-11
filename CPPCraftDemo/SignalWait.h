#pragma once

#include <mutex>

class SignalWait
{
public:
    // Constructor
    SignalWait();

    // Set and reset the condition
    void Set(bool in_state);

    // Check condition without waiting
    bool IsTrue(void);

    // Wait until condition becomes true
    // If the condition has already been set the function
    // will immediately return.
    void WaitUntilTrue(void);

private:
    bool mSignal;

    // Access mutex for reading/writing mSignal
    std::mutex mMutex;

    // Condition variable for triggering a waiting thread
    std::condition_variable mConditionVar;
};