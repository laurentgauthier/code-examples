#ifndef _VIDEO_SOURCE_H_
#define _VIDEO_SOURCE_H_

enum io_method {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

struct buffer {
	void   *start;
	size_t  length;
};

struct video_source {
	char *dev_name;
	enum io_method io;
	int fd;
	struct buffer *buffers;
	unsigned int n_buffers;
	int force_format;
	int frame_count;
	int frame_number;
        void (*process_image)(struct video_source *source, const void *p, int size);
};

void video_source_loop(struct video_source *source);

void video_source_stop(struct video_source *source);

void video_source_start(struct video_source *source);

void video_source_open(struct video_source *source);

void video_source_close(struct video_source *source);

#endif
