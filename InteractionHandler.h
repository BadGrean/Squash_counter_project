#ifndef INTERACTION_HANDLER_H
#define INTERACTION_HANDLER_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Enum for game outcomes
typedef enum {
    No_action = -1,
    Player1_point = 0,
    Player2_point = 1,
    Game_restarted = 2,
    Match_ended = 3
} OutcomeState;

// Function declarations
OutcomeState processInteraction(bool sensor1BelowThreshold, bool sensor2BelowThreshold);
const char* outcomeStateToString(OutcomeState outcome); // Converts enum to string

#ifdef __cplusplus
}
#endif

#endif
