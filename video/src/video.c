#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>	      /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#ifndef V4L2_PIX_FMT_H264
#define V4L2_PIX_FMT_H264     v4l2_fourcc('H', '2', '6', '4') /* H264 with start codes */
#endif

#include <video/source.h>

static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

static int read_frame(struct video_source *source)
{
	struct v4l2_buffer buf;
	unsigned int i;

	switch (source->io) {
	case IO_METHOD_READ:
		if (-1 == read(source->fd, source->buffers[0].start, source->buffers[0].length)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit("read");
			}
		}

		source->process_image(source, source->buffers[0].start,
				source->buffers[0].length);
		break;

	case IO_METHOD_MMAP:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(source->fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		assert(buf.index < source->n_buffers);

		source->process_image(source, source->buffers[buf.index].start, buf.bytesused);

		if (-1 == xioctl(source->fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		break;

	case IO_METHOD_USERPTR:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(source->fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < source->n_buffers; ++i)
			if (buf.m.userptr == (unsigned long)source->buffers[i].start
			    && buf.length == source->buffers[i].length)
				break;

		assert(i < source->n_buffers);

		source->process_image(source, (void *)buf.m.userptr, buf.bytesused);

		if (-1 == xioctl(source->fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		break;
	}

	return 1;
}

void video_source_loop(struct video_source *source)
{
	unsigned int count;

	count = source->frame_count;

	while (count-- > 0) {
		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(source->fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(source->fd + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (EINTR == errno)
					continue;
				errno_exit("select");
			}

			if (0 == r) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}

			if (read_frame(source))
				break;
			/* EAGAIN - continue select loop. */
		}
	}
}

void video_source_stop(struct video_source *source)
{
	enum v4l2_buf_type type;

	switch (source->io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(source->fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");
		break;
	}
}

void video_source_start(struct video_source *source)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (source->io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < source->n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(source->fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(source->fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < source->n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long)source->buffers[i].start;
			buf.length = source->buffers[i].length;

			if (-1 == xioctl(source->fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(source->fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");
		break;
	}
}

static void init_read(struct video_source *source, unsigned int buffer_size)
{
	source->buffers = calloc(1, sizeof(*source->buffers));

	if (!source->buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	source->buffers[0].length = buffer_size;
	source->buffers[0].start = malloc(buffer_size);

	if (!source->buffers[0].start) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

static void init_mmap(struct video_source *source)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(source->fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support "
				 "memory mapping\n", source->dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n",
			 source->dev_name);
		exit(EXIT_FAILURE);
	}

	source->buffers = calloc(req.count, sizeof(*source->buffers));

	if (!source->buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (source->n_buffers = 0; source->n_buffers < req.count; ++source->n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = source->n_buffers;

		if (-1 == xioctl(source->fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		source->buffers[source->n_buffers].length = buf.length;
		source->buffers[source->n_buffers].start =
			mmap(NULL /* start anywhere */,
			      buf.length,
			      PROT_READ | PROT_WRITE /* required */,
			      MAP_SHARED /* recommended */,
			      source->fd, buf.m.offset);

		if (MAP_FAILED == source->buffers[source->n_buffers].start)
			errno_exit("mmap");
	}
}

static void init_userp(struct video_source *source, unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count  = 4;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(source->fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support "
				 "user pointer i/o\n", source->dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	source->buffers = calloc(4, sizeof(*source->buffers));

	if (!source->buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (source->n_buffers = 0; source->n_buffers < 4; ++source->n_buffers) {
		source->buffers[source->n_buffers].length = buffer_size;
		source->buffers[source->n_buffers].start = malloc(buffer_size);

		if (!source->buffers[source->n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}

static void init_device(struct video_source *source)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(source->fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n",
				 source->dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n",
			 source->dev_name);
		exit(EXIT_FAILURE);
	}

	switch (source->io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read i/o\n",
				 source->dev_name);
			exit(EXIT_FAILURE);
		}
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming i/o\n",
				 source->dev_name);
			exit(EXIT_FAILURE);
		}
		break;
	}

	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(source->fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(source->fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
			}
		}
	} else {
		/* Errors ignored. */
	}

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (source->force_format) {
		fprintf(stderr, "Set GREY\r\n");
		fmt.fmt.pix.width       = 640; //replace
		fmt.fmt.pix.height      = 480; //replace
		//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264; //replace
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
		fmt.fmt.pix.field       = V4L2_FIELD_ANY;

		if (-1 == xioctl(source->fd, VIDIOC_S_FMT, &fmt))
			errno_exit("VIDIOC_S_FMT");

		/* Note VIDIOC_S_FMT may change width and height. */
	} else {
		/* Preserve original settings as set by v4l2-ctl for example */
		if (-1 == xioctl(source->fd, VIDIOC_G_FMT, &fmt))
			errno_exit("VIDIOC_G_FMT");
	}

#if 0
	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;
#else
	fmt.fmt.pix.sizeimage = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
#endif

	switch (source->io) {
	case IO_METHOD_READ:
		init_read(source, fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap(source);
		break;

	case IO_METHOD_USERPTR:
		init_userp(source, fmt.fmt.pix.sizeimage);
		break;
	}
}

void video_source_open(struct video_source *source)
{
	struct stat st;

	if (-1 == stat(source->dev_name, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
			 source->dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", source->dev_name);
		exit(EXIT_FAILURE);
	}

	source->fd = open(source->dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == source->fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
			 source->dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	init_device(source);
}

static void uninit_device(struct video_source *source)
{
	unsigned int i;

	switch (source->io) {
	case IO_METHOD_READ:
		free(source->buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < source->n_buffers; ++i)
			if (-1 == munmap(source->buffers[i].start, source->buffers[i].length))
				errno_exit("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < source->n_buffers; ++i)
			free(source->buffers[i].start);
		break;
	}

	free(source->buffers);
}

void video_source_close(struct video_source *source)
{
	uninit_device(source);

	if (-1 == close(source->fd))
		errno_exit("close");

	source->fd = -1;
}
