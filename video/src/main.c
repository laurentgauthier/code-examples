#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>	     /* getopt_long() */
#include <errno.h>
#include <video/source.h>

static void usage(FILE *fp, int argc, char **argv, struct video_source *source)
{
	fprintf(fp,
		 "Usage: %s [options]\n\n"
		 "Version 1.3\n"
		 "Options:\n"
		 "-d | --device name   Video device name [%s]\n"
		 "-h | --help          Print this message\n"
		 "-m | --mmap          Use memory mapped buffers [default]\n"
		 "-r | --read          Use read() calls\n"
		 "-u | --userp         Use application allocated buffers\n"
		 "-f | --format        Force format to 640x480 YUYV\n"
		 "-c | --count         Number of frames to grab [%i]\n"
		 "",
		 argv[0], source->dev_name, source->frame_count);
}

static const char short_options[] = "d:hmrufc:";

static const struct option
long_options[] = {
	{ "device", required_argument, NULL, 'd' },
	{ "help",   no_argument,       NULL, 'h' },
	{ "mmap",   no_argument,       NULL, 'm' },
	{ "read",   no_argument,       NULL, 'r' },
	{ "userp",  no_argument,       NULL, 'u' },
	{ "format", no_argument,       NULL, 'f' },
	{ "count",  required_argument, NULL, 'c' },
	{ 0, 0, 0, 0 }
};

static void process_image(struct video_source *source, const void *p, int size)
{
        source->frame_number++;
	fprintf(stderr, "frame %d size %d\n", source->frame_number, size);
}

int main(int argc, char **argv)
{
	struct video_source *source;

	/* Initialize the video source */
	source = (struct video_source *)calloc(1, sizeof(struct video_source));
	source->dev_name = "/dev/video0";
	source->io = IO_METHOD_MMAP;
	source->fd = -1;
	source->frame_count = 200;
	source->process_image = process_image;

	for (;;) {
		int idx;
		int c;

		c = getopt_long(argc, argv,
				short_options, long_options, &idx);

		if (-1 == c)
			break;

		switch (c) {
		case 0: /* getopt_long() flag */
			break;

		case 'd':
			source->dev_name = optarg;
			break;

		case 'h':
			usage(stdout, argc, argv, source);
			exit(EXIT_SUCCESS);

		case 'm':
			source->io = IO_METHOD_MMAP;
			break;

		case 'r':
			source->io = IO_METHOD_READ;
			break;

		case 'u':
			source->io = IO_METHOD_USERPTR;
			break;

		case 'f':
			source->force_format++;
			break;

		case 'c':
			errno = 0;
			source->frame_count = strtol(optarg, NULL, 0);
			if (errno) {
				fprintf(stderr, "%s error %d, %s\n", optarg, errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			break;

		default:
			usage(stderr, argc, argv, source);
			exit(EXIT_FAILURE);
		}
	}

	video_source_open(source);
	video_source_start(source);
	video_source_loop(source);
	video_source_stop(source);
	video_source_close(source);
	free(source);
	fprintf(stderr, "\n");
	return 0;
}
