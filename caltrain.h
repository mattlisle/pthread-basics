
#ifndef CALTRAIN_H
#define CALTRAIN_H

#include "pintos_thread.h"

/**
 * @brief representation of a robotic train station
 */
struct station {
	int seats_left; /**< total number of seats on the train */
	int passengers_waiting; /**< total number of passengers on the train */
	int passengers_standing; /**< total number of passengers waiting to be seated */
	struct condition *free_seat; /**< if there is a free seat on board the train */
	struct condition *all_aboard;
	struct lock *station_lock; /**< lock for synchronization */
};

/**
 * @brief set station parameters to their initial values
 * 
 * @param station whose parameters are being set
 */
void station_init(struct station *station);

/**
 * @brief when a train arrives in the station and has opened its doors, it invokes this
 * 
 * @param station the station that the train is at
 * @param count indicates how many seats are available on the train
 * @return when the train is satisfactorily loaded (all passengers are in their seats,
 *         and either the train is full or all waiting passengers have boarded)
 */
void station_load_train(struct station *station, int count);

/**
 * @brief when a passenger robot arrives in a station, it first invokes this function
 * 
 * @param station the station that the passenger is at
 * @return when a train is in the station (when `station_load_train` is in progress)
 *         and there are enough free seats on the train for this passenger to sit down
 */
void station_wait_for_train(struct station *station);

/**
 * @brief once the passenger is seated, it will call this function
 * 
 * @param station the station that the passenger and train are at
 * @return once the train knows that the passenger is on board
 */
void station_on_board(struct station *station);

#endif
