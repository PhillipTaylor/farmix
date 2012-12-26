#include <system.h>

#define SCR_HEIGHT 25
#define SCR_WIDTH 80
#define SCR_SIZE (SCR_HEIGHT * SCR_WIDTH)

// Available lines of scrollback including what you can actually
// see. (min value=SCR_HEIGHT)
#define BUFFER_LINES 60
#define BUFFER_SIZE (BUFFER_LINES * SCR_WIDTH)

#define TAB_SPACES 4

#define KEY_BACKSPACE 0x08
#define KEY_TAB 0x09

void redraw(void);
void move_csr(void);
void clear_screen(void);
inline int is_eob(void); // eob = end of buffer
inline size_t eol_distance(void); // eol = end of line.
inline void push_scrollback(void);

// This is the memory that's shared with the VGA. Anything written into
// here becomes visible on screen. This console driver essentially works
// by copying everything into buffer, and then redraw() copies the viewport
// (view_offset) portion into this address area.
unsigned short *textmemptr;

unsigned short buffer[BUFFER_SIZE][SCR_WIDTH];

int attrib = 0x0F; //controls color etc.
unsigned blank; // what a blank space given the colour attributes.

unsigned short *buf_csr; //buffer cursor location.
unsigned short *view_offset; //which subset of the buffer we're viewing.
int scroll_lock; //move viewport on print

void init_video(void) {
	textmemptr = (unsigned short *)0xB8000;
	blank = 0x20 | (attrib << 8);
	buf_csr = &buffer;
	view_offset = &buffer;
	cls();
	scroll_lock = 0;
	redraw();
}

void scr_scroll_down(void) {
	if (view_offset + SCR_WIDTH < &buffer + BUFFER_SIZE) {
		view_offset += SCR_WIDTH;
		move_csr();
		redraw();
	}
}

void scr_scroll_up(void) {
	if (view_offset > &buffer) {
		view_offset -= SCR_WIDTH;
		move_csr();
		redraw();
	}
}

void redraw(void) {
	//copy a screens worth of data from the buffer into
	//the actual video memory.
	unsigned short *text_tmp, *view_tmp;
	unsigned short *eob, *eos;

	text_tmp = textmemptr;
	view_tmp = view_offset;

	//one whole screen from viewport location (eos=end of screen)
	eos = view_offset + SCR_SIZE;
	eob = &buffer + BUFFER_SIZE; //eob = end of buffer

	while (view_tmp < eob && view_tmp < eos) {
		*text_tmp = *view_tmp;
		text_tmp++;
		view_tmp++;
	}

	// blank the screen if you scrolled past the end of the buffer.
	while (view_tmp < eos) {
		*text_tmp = blank;
		text_tmp++;
		view_tmp++;
	}
}

/* Updates the hardware cursor: the little blinking line
*  on the screen under the last character pressed! */
void move_csr(void) {
	unsigned temp;

	if (view_offset < buf_csr && view_offset + SCR_SIZE > buf_csr)
		temp = buf_csr - view_offset;
	else
		temp = 0;

	//more info at http://www.brackeen.com/home/vga
	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

// clear everything including scrollback
void cls(void) {
	memsetw(&buffer, blank, BUFFER_SIZE);
	clear_screen();
	buf_csr = &buffer;
	view_offset = &buffer;
	move_csr();
}

void clear_screen(void) {
	memsetw(textmemptr, blank, SCR_SIZE);
}

void putch(char ch) {
	size_t i;
	int j;
	unsigned att = attrib << 8;
	unsigned short *eob;
	unsigned short *eolwrite;

	if (is_eob())
		push_scrollback();

	// do the backspace key later
	// when we actually write a shell.
	if (ch == KEY_BACKSPACE)
		return;

	if (ch == KEY_TAB) {
		for (j = 0; j < TAB_SPACES; j++)
			putch(' ');
	} else if (ch == '\n') {

			i = eol_distance();

			//easiest implementation? just fill to end of line with spaces?
			while (i > 0) {
				putch(' '); //replace with memset later?
				--i;
			}

	} else if (ch >= ' ') {
		*buf_csr = ch | att;
		buf_csr++;
		move_csr();
		redraw();
	}
}

// Tells me if the buffer cursor is in
// the final line of the buffer.
inline int is_eob(void) {
	return (buf_csr + 1 == (&buffer + BUFFER_SIZE));
}

// return the number of chars to the end of the current line
// used for blanking out the spaces when a newline is pressed.
inline size_t eol_distance(void) {
	return SCR_WIDTH - ((buf_csr - &buffer[0][0]) % SCR_WIDTH);
}

// Rely our specific implementation of memcpy
// which when moving the scrollback up(?) in
// memory isn't a problem even though the pointers
// overlap.
void push_scrollback(void) {

	unsigned short *buf_iter, *buf_trail;
	unsigned short *eob;

	eob = &buffer + BUFFER_SIZE;

	buf_trail = &buffer;
	buf_iter = buf_trail + SCR_WIDTH;

	// I could have used memcpy(buf_iter, buf_trail, SCR_WIDTH)
	// if I had wanted to.
	while (buf_iter < eob) {
		*buf_trail = *buf_iter;
		buf_trail++;
		buf_iter++;
	}

	//blank the last line so it isn't duplicated
	//and it's fresh and ready to write on.
	while (buf_trail < eob) {
		*buf_trail = blank;
		buf_trail++;
	}

	//rewind the text insertion pointer.
	buf_csr -= SCR_WIDTH;
}

/* put string */
void puts(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++)
		putch(str[i]);
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(unsigned char forecolor, unsigned char backcolor) {
	// Top 4 bytes is background, bottom 4 bytes is foreground color
	attrib = (backcolor << 4) | (forecolor & 0x0F);
}
