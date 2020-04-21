#include <stdbool.h>
#include "caltrain.h"

#define bool _Bool

void station_init(struct station *station) {
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

void station_load_train(struct station *station, int count) {
	printf("seats: %d, waiting: %d", count, station->passengers_waiting);
	lock_acquire(station->station_lock);
	station->seats_left = count;
	if (count && station->passengers_waiting) {
		cond_broadcast(station->free_seat, station->station_lock);
		cond_wait(station->all_aboard, station->station_lock);
	}
	lock_release(station->station_lock);
}

void station_wait_for_train(struct station *station) {
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

void station_on_board(struct station *station) {
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
