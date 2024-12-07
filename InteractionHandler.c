#include "InteractionHandler.h"

// Constants for timing thresholds
const unsigned long POINT_TRIGGER_TIME = 500;   // Time for individual sensor point
const unsigned long RESET_TIME = 1000;         // Time for resetting score
const unsigned long END_MATCH_TIME = 3000;     // Time for ending the match

// Internal state variables
static unsigned long sensor1StartTime = ULONG_MAX;
static unsigned long sensor2StartTime = ULONG_MAX;
static unsigned long bothSensorsStartTime = ULONG_MAX;

// Main game logic function
OutcomeState processInteraction(bool sensor1BelowThreshold, bool sensor2BelowThreshold) {
    unsigned long currentTime = millis();

    // Check sensor 1
    if (sensor1BelowThreshold) {
        if (sensor1StartTime == ULONG_MAX) {
            sensor1StartTime = currentTime;
        }
    } else {
        if (!sensor2BelowThreshold) {
            if ((currentTime - sensor1StartTime) >= POINT_TRIGGER_TIME && sensor1StartTime != ULONG_MAX) {
                sensor1StartTime = ULONG_MAX;
                return Player1_point; // Player 1 scores
            }
        }
        sensor1StartTime = ULONG_MAX; // Reset timer
    }

    // Check sensor 2
    if (sensor2BelowThreshold) {
        if (sensor2StartTime == ULONG_MAX) {
            sensor2StartTime = currentTime;
        }
    } else {
        if (!sensor1BelowThreshold) {
            if ((currentTime - sensor2StartTime) >= POINT_TRIGGER_TIME && sensor2StartTime != ULONG_MAX) {
                sensor2StartTime = ULONG_MAX;
                return Player2_point; // Player 2 scores
            }
        }
        sensor2StartTime = ULONG_MAX; // Reset timer
    }

    // Both sensors below threshold
    if (sensor1BelowThreshold && sensor2BelowThreshold) {
        if (bothSensorsStartTime == ULONG_MAX) {
            bothSensorsStartTime = currentTime;
        }
        sensor1StartTime = ULONG_MAX;
        sensor2StartTime = ULONG_MAX;
    } else {
        unsigned long elapsed = currentTime - bothSensorsStartTime;
        if (elapsed >= RESET_TIME && elapsed < END_MATCH_TIME && bothSensorsStartTime != ULONG_MAX) {
            bothSensorsStartTime = ULONG_MAX;
            return Game_restarted; // Game restarted
        }
        if (elapsed >= END_MATCH_TIME && bothSensorsStartTime != ULONG_MAX) {
            bothSensorsStartTime = ULONG_MAX;
            sensor1StartTime = ULONG_MAX;
            sensor2StartTime = ULONG_MAX;
            return Match_ended; // Match ended
        }
        bothSensorsStartTime = ULONG_MAX; // Reset
    }

    return No_action; // No action
}

// Converts OutcomeState enum values to human-readable strings
const char* outcomeStateToString(OutcomeState outcome) {
    switch (outcome) {
        case Player1_point: return "Player 1 Point";
        case Player2_point: return "Player 2 Point";
        case Game_restarted: return "Game Restarted";
        case Match_ended: return "Match Ended";
        default: return "No Action";
    }
}