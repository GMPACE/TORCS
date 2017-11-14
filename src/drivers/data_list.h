enum actuating_data_name{ACCEL, BRAKE, LEFT_STEER, RIGHT_STEER, STEER_MODE, NUM_DUMMY};

enum flag_data_name{CC,LKAS};

enum sensing_data_name{CC_TRIGGER, 	LKAS_TRIGGER, 		SPEED, 		TARGET_SPEED, 		ACCEL_VALUE, 
			STEER_ANGLE, 	LOOKAHEAD,		TO_LEFT, 	TO_RIGHT, 		TO_MIDDLE,
			WIDTH, 		FUEL,			ENGINE_RPM, 	GEAR_RATIO, 		GEAR, 		
			PITCH, 		ROLL, 			YAW_RATE, 	DISTANCE, 		POS_X, 		
			POS_Y, 		SPEED_ERROR,		LATERAL, 	TARGET_WHEEL_SPEED, 	WHEEL_SPEED, 	
			IS_RUNNING, 	PASSED_TIME};
/******** DATA ********/

/* Name / Type / Number


--------Control-------
CC_TRIGGER / short / 0
LKAS_TRIGGER / short / 1
SPEED / float / 2
TARGET_SPEED / float / 3
ACCEL / float / 4
TRACK / float / 5
YAW / float / 6
TO_MIDDLE / float / 7
WIDTH / float / 8
TO_LEFT / float / 9
TO_RIGHT / float / 10 */
