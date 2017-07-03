#ifndef MOCK_TIME_MANAGER_H
#define MOCK_TIME_MANAGER_H

#include "time_manager_interface.h"

class MockTimeManager : public TimeManagerInterface
{
public:
    /**
     * Constructor.
     */
    MockTimeManager();
    
    void addTime(double time);
    ros::Time simulationTimeNow();

private:
    unsigned int index;
    std::vector<double> time_list; 

};

#endif /* MOCK_TIME_MANAGER_H */
