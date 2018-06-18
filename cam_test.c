#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int main()
{
    
    //Open device.
    int fd;
    if((fd = open("/dev/video0", O_RDWR)) < 0)
    {
        perror("open");
        exit(1);
    }

    //Retrieve the device’s capabilities.
    struct v4l2_capability cap;
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("VIDIOC_QUERYCAP");
        exit(1);
    }

    //image format.
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 800;
    format.fmt.pix.height = 600;
 
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("VIDIOC_S_FMT");
        exit(1);
    }

    //request buffers.
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;
 
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        exit(1);
    }

    //Query buffer
    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
 
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
 
    if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
        perror("VIDIOC_QUERYBUF");
        exit(1);
    }

    void* buffer_start = mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bufferinfo.m.offset);
 
    if(buffer_start == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
 
    memset(buffer_start, 0, bufferinfo.length);

    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        exit(1);
    }

    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }


    int jpgfile;
    if((jpgfile = open("/tmp/img.jpeg", O_WRONLY | O_CREAT, 0660)) < 0){
        perror("open");
        return 1;
    }
 
    write(jpgfile, buffer_start, bufferinfo.length);
    close(jpgfile);
    close(fd);
    return 0;
}