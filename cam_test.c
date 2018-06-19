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
    
    //Open file descriptor to the device.
    int fd;
    if((fd = open("/dev/video0", O_RDWR)) < 0)
    {
        perror("open");
        exit(1);
    }

    //Retrieve the device’s capabilities.
    //Every v4l2-compatible device is expected to handle this request
    struct v4l2_capability cap;
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("VIDIOC_QUERYCAP");
        exit(1);
    }

    //Image format.
    //Format must be made available by the device.
    //use the command 'v4l2-ctl -d /dev/video0 --list-formats-ext' to check all available formats
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //tell V4L2 we are doing a video capture
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 800;
    format.fmt.pix.height = 600;
 
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("VIDIOC_S_FMT");
        exit(1);
    }

    //Inform the device about your buffers: how are you going to allocate them? How many are there?
    //Information is sent using the VIDIOC_REQBUFS call and a v4l2_requestbuffers structure
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //tell V4L2 we are doing a video capture
    bufrequest.memory = V4L2_MEMORY_MMAP; //use memory mapping to allocate the buffers
    bufrequest.count = 1; //buffer count
 
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        exit(1);
    }

    //Allocate buffer
    //Ask the device the amount of memory it needs, and allocate it.
    //Information is retrieved using the VIDIOC_QUERYBUF call, and its v4l2_buffer structure.
    struct v4l2_buffer bufferinfo;

    //Clearing the structure’s memory space before using it.
    memset(&bufferinfo, 0, sizeof(bufferinfo));
 
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0; //index of the buffer
 
    if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
        perror("VIDIOC_QUERYBUF");
        exit(1);
    }

    //Memory mapping
    void* buffer_start = mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bufferinfo.m.offset);
    
    if(buffer_start == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    
    memset(buffer_start, 0, bufferinfo.length);


    //Get a frame.
    //
    //
    // Activate streaming
    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        exit(1);
    }

    // Put the buffer in the incoming queue.
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    // The buffer's waiting in the outgoing queue.
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    // Deactivate streaming.
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    //Save frame as a JPEG file
    int jpgfile;
    if((jpgfile = open("/tmp/img.jpeg", O_WRONLY | O_CREAT, 0660)) < 0){ //change the path as you wish.
        perror("open");
        return 1;
    }
 
    write(jpgfile, buffer_start, bufferinfo.length);
    close(jpgfile);
    
    close(fd);
    return 0;
}