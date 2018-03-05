
#define CHANNEL_UP 1234
#define CHANNEL_DOWN -1234

struct INPUT {
	double lx;
	double ly;
	double rx;
	double ry;

	char lpad;
	char rpad;

	char sel;
	char start;
	char ps;

	char up;
	char down;
	char left;
	char right;

	char r1;
	char r2;

	char l1;
	char l2;

	char sqr;
	char crc;
	char crs;
	char tri;
};
