#include "pintos_thread.h"
#include <stdbool.h>

#define bool _Bool

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
void
station_init(struct station *station)
{
	station->seats_left = 0;
	station->passengers_waiting = 0;
	station->passengers_standing = 0;
	station->free_seat = malloc(sizeof(struct condition));
	station->all_aboard = malloc(sizeof(struct condition));
	station->station_lock = malloc(sizeof(struct lock));

	cond_init(station->free_seat);
	cond_init(station->all_aboard);
	lock_init(station->station_lock);
}

/**
 * @brief when a train arrives in the station and has opened its doors, it invokes this
 * 
 * @param station the station that the train is at
 * @param count indicates how many seats are available on the train
 * @return when the train is satisfactorily loaded (all passengers are in their seats,
 *         and either the train is full or all waiting passengers have boarded)
 */
void
station_load_train(struct station *station, int count)
{
	printf("seats: %d, waiting: %d", count, station->passengers_waiting);
	lock_acquire(station->station_lock);
	station->seats_left = count;
	if (count && station->passengers_waiting) {
		cond_broadcast(station->free_seat, station->station_lock);
		cond_wait(station->all_aboard, station->station_lock);
	}
	lock_release(station->station_lock);
}

/**
 * @brief when a passenger robot arrives in a station, it first invokes this function
 * 
 * @param station the station that the passenger is at
 * @return when a train is in the station (when `station_load_train` is in progress)
 *         and there are enough free seats on the train for this passenger to sit down
 */
void
station_wait_for_train(struct station *station)
{
	lock_acquire(station->station_lock);
	++(station->passengers_waiting);

	while(true) {
		cond_wait(station->free_seat, station->station_lock);
		if ((station->seats_left) > 0) {
			--(station->seats_left);
			--(station->passengers_waiting);
			++(station->passengers_standing);
			lock_release(station->station_lock);
			return;
		}
	}	
}

/**
 * @brief once the passenger is seated, it will call this function
 * 
 * @param station the station that the passenger and train are at
 * @return once the train knows that the passenger is on board
 */
void
station_on_board(struct station *station)
{
	lock_acquire(station->station_lock);
	--(station->passengers_standing);
	bool passengers_ready = (station->passengers_standing) == 0;
	bool train_ready = (station->seats_left) == 0 || (station->passengers_waiting) == 0;

	if (!train_ready && passengers_ready) {
		cond_broadcast(station->free_seat, station->station_lock);
	} else if (train_ready && passengers_ready) {
		cond_signal(station->all_aboard, station->station_lock);
	}
	lock_release(station->station_lock);
}
