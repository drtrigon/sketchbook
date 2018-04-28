#ifndef _gamepad_h__
#define _gamepad_h__

typedef struct {
	// size of reports built by buildReport
	int report_size;

	void (*init)(void);
	void (*update)(void);
	char (*changed)(void);
	void (*buildReport)(unsigned char *buf);

	/* Check for the controller */
	char (*probe)(void); /* return true if found */
} Gamepad;

#endif // _gamepad_h__


