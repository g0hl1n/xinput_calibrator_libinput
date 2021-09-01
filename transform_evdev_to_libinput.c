#include <stdio.h>
#include <stdlib.h>

// scaleAxis() from xinput_calibrator
// Copyright (c) 2010 Tias Guns <tias@ulyssis.org> and others
float scaleAxis(float Cx, int to_max, int to_min, int from_max, int from_min)
{
	float X;
	int64_t to_width   = to_max - to_min;
	int64_t from_width = from_max - from_min;

	if (from_width) {
		X = (((to_width * (Cx - from_min)) / from_width) + to_min);
	} else {
		X = 0;
		printf("Divide by Zero in scaleAxis\n");
		exit(1);
	}

	return X;
}

int main(int argc, char **argv)
{
	if (argc < 9) {
		printf("usage: %s min_x, max_x, min_y, max_y, screen_px_x, screen_px_y, touch_px_x, "
			   "touch_px_y\n",
			   argv[0]);
		return EXIT_FAILURE;
	}

	int min_x		= atoi(argv[1]);
	int max_x		= atoi(argv[2]);
	int min_y		= atoi(argv[3]);
	int max_y		= atoi(argv[4]);
	int screen_px_x = atoi(argv[5]);
	int screen_px_y = atoi(argv[6]);
	int touch_px_x	= atoi(argv[7]);
	int touch_px_y	= atoi(argv[8]);

	int li_min_x = scaleAxis(min_x, screen_px_x, 0, touch_px_x, 0);
	int li_max_x = scaleAxis(max_x, screen_px_x, 0, touch_px_x, 0);
	// printf("libinput min_x/max_x: %d/%d\n", li_min_x, li_max_x);

	int li_min_y = scaleAxis(min_y, screen_px_y, 0, touch_px_y, 0);
	int li_max_y = scaleAxis(max_y, screen_px_y, 0, touch_px_y, 0);
	// printf("libinput min_y/max_y: %d/%d\n", li_min_y, li_max_y);

	int li_tot_x = abs(li_max_x - li_min_x);
	int li_tot_y = abs(li_max_y - li_min_y);
	// printf("libinput tot_x/tot_y: %d/%d\n", li_tot_x, li_tot_y);

	float a = -(float)screen_px_y / (float)li_tot_y;
	float b = (float)li_min_y / (float)li_tot_y;
	float c = -(float)screen_px_x / (float)li_tot_x;
	float d = (float)li_min_x / (float)li_tot_x;
	printf("xinput set-prop 'TOUCH_DEVICE' 'libinput Calibration Matrix' 0, %f, %f, %f, 0, %f, 0, "
		   "0, 1;\n",
		   a, b, c, d);
	printf("ENV{LIBINPUT_CALIBRATION_MATRIX}=\"0 %f %f %f 0 %f 0 0 1\"\n", a, b, c, d);

	return EXIT_SUCCESS;
}
